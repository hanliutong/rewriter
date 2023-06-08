#include "nlanes_check.hpp"
#include "clang/Lex/Lexer.h"
// #include "clang/ASTMatchers/ASTMatchFinder.h"

namespace clang::tidy {

NlanesCheck::NlanesCheck(StringRef name, ClangTidyContext *context)
    : ClangTidyCheck(name, context) {}

void NlanesCheck::registerMatchers(ast_matchers::MatchFinder *finder) {
  using namespace ast_matchers;
  auto broadcastCall = callExpr(isExpansionInMainFile(),
                                callee(functionDecl(hasName("v_broadcast_element")).bind("funcDecl")),
                                hasDescendant(declRefExpr(hasDeclaration(namedDecl(hasName("nlanes")))).bind("nlanesInTemplate")))
                           .bind("funcCall");
  auto extractCall = callExpr(isExpansionInMainFile(),
                              callee(functionDecl(hasName("v_extract_n")).bind("funcDecl")),
                              hasDescendant(declRefExpr(hasDeclaration(namedDecl(hasName("nlanes")))).bind("nlanesInTemplate")))
                         .bind("funcCall");
  auto arrSizeMatcher = declRefExpr(isExpansionInMainFile(),
                                    hasDeclaration(namedDecl(hasName("nlanes"))),
                                    hasAncestor(varDecl(hasType(arrayType()))))
                            .bind("constant");
  auto refForArrSizeMatcher = decl(isExpansionInMainFile(),
                                   hasDescendant(varDecl(hasInitializer(ignoringImpCasts(
                                                             declRefExpr(hasDeclaration(namedDecl(hasName("nlanes")))).bind("constant"))))
                                                     .bind("cVar")),
                                   hasDescendant(varDecl(hasType(arrayType()),
                                                         hasDescendant(declRefExpr(to(equalsBoundNode("cVar")))))));
  auto othersNlanesMatcher = declRefExpr(isExpansionInMainFile(),
                                         hasDeclaration(namedDecl(hasName("nlanes"))))
                                 .bind("x");

  finder->addMatcher(broadcastCall, this);
  finder->addMatcher(extractCall, this);
  finder->addMatcher(arrSizeMatcher, this);
  finder->addMatcher(refForArrSizeMatcher, this);
  finder->addMatcher(othersNlanesMatcher, this);
}

void NlanesCheck::check(const ast_matchers::MatchFinder::MatchResult &result) {
  const DeclRefExpr *matchedExpr = result.Nodes.getNodeAs<DeclRefExpr>("x");
  const DeclRefExpr *constantNlanes = result.Nodes.getNodeAs<DeclRefExpr>("constant");
  const DeclRefExpr *nlanesRefInTemplate = result.Nodes.getNodeAs<DeclRefExpr>("nlanesInTemplate");
  const FunctionDecl *funcDecl = result.Nodes.getNodeAs<FunctionDecl>("funcDecl");
  const CallExpr *callExpr = result.Nodes.getNodeAs<CallExpr>("funcCall");

  if (matchedExpr && matchedExpr->getNameInfo().getAsString().compare("nlanes") == 0 &&
      processedNlanes.find(matchedExpr) == processedNlanes.end()) {
    const auto *vecStructDecl = dyn_cast<CXXRecordDecl>(matchedExpr->getDecl()->getDeclContext()->getParent());
    if (vecStructDecl && vecStructDecl->getName().starts_with("v_")) {  // v_type::nlanes
      SourceManager &SM = result.Context->getSourceManager();
      std::string vecTypeName =  // get vector type from source code
          clang::Lexer::getSourceText(
              CharSourceRange::getTokenRange(
                  result.Context->getFullLoc(matchedExpr->getBeginLoc())),
              SM, result.Context->getLangOpts())
              .str();
      auto parent = result.Context->getParents(*matchedExpr)[0];
      diag(matchedExpr->getLocation(), "Found nlanes.")
          << FixItHint::CreateReplacement(parent.getSourceRange(),
                                          "VTraits<" + vecTypeName + ">::vlanes()");
      // } else if (matchedExpr) {  // vecStructDecl is not start with "v_"
      //   diag(matchedExpr->getLocation(),
      //        "Found others nlanes, maybe not a member of the Universal Intrinsic type",
      //        clang::DiagnosticIDs::Note);
    }
  }

  if (constantNlanes && constantNlanes->getNameInfo().getAsString().compare("nlanes") == 0) {
    processedNlanes.insert(constantNlanes);
    const auto *vecStructDecl = dyn_cast<CXXRecordDecl>(constantNlanes->getDecl()->getDeclContext()->getParent());
    if (vecStructDecl && vecStructDecl->getName().starts_with("v_")) {  // v_type::nlanes
      SourceManager &SM = result.Context->getSourceManager();
      constantNlanes->getSourceRange().dump(SM);
      std::string vecTypeName =  // get vector type from source code
          clang::Lexer::getSourceText(
              CharSourceRange::getTokenRange(
                  result.Context->getFullLoc(constantNlanes->getBeginLoc())),
              SM, result.Context->getLangOpts())
              .str();
      diag(constantNlanes->getLocation(), "Found nlanes as constant.")
          << FixItHint::CreateReplacement(constantNlanes->getSourceRange(),
                                          "VTraits<" + vecTypeName + ">::max_nlanes");
    }
  }

  if (callExpr && funcDecl && nlanesRefInTemplate) {
    processedNlanes.insert(nlanesRefInTemplate);
    if (auto nlanesDecl = dyn_cast<EnumConstantDecl>(nlanesRefInTemplate->getDecl())) {
      if (auto nlanesExpr = dyn_cast<IntegerLiteral>(nlanesDecl->getInitExpr()->IgnoreCasts())) {
        if (nlanesExpr->getValue().getSExtValue() - 1 ==
            funcDecl->getTemplateSpecializationArgs()->get(0).getAsIntegral().getExtValue()) {
          std::string arg0ExprStr = Lexer::getSourceText(
                                        CharSourceRange::getTokenRange(callExpr->getArg(0)->getSourceRange()),
                                        *result.SourceManager, result.Context->getLangOpts())
                                        .str();
          std::string replaceFuncStr;
          if (funcDecl->getNameAsString().compare("v_broadcast_element") == 0)
            replaceFuncStr = "v_broadcast_highest(";
          else if (funcDecl->getNameAsString().compare("v_extract_n") == 0)
            replaceFuncStr = "v_extract_highest(";
          else
            assert(0 && "more functions?");

          diag(nlanesRefInTemplate->getLocation(), "Found nlanes as template argument")
              << FixItHint::CreateReplacement(callExpr->getSourceRange(),
                                              replaceFuncStr + arg0ExprStr + ")");
        } else {  // not <::nlanes - 1>
          diag(nlanesRefInTemplate->getLocation(),
               "Found nlanes as template argument, but no function can replace it.",
               DiagnosticIDs::Error);
        }
      }
    }  // nlanes is not declared as an enum constant, aka not in the (original) Universal Intrinsic Type.
  }
}

}  // namespace clang::tidy
