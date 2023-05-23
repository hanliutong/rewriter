#include "nlanes_check.hpp"
#include "clang/Lex/Lexer.h"
// #include "clang/ASTMatchers/ASTMatchFinder.h"

namespace clang::tidy {

NlanesCheck::NlanesCheck(StringRef name, ClangTidyContext *context)
    : ClangTidyCheck(name, context) {}

void NlanesCheck::registerMatchers(ast_matchers::MatchFinder *finder) {
  using namespace ast_matchers;
  finder->addMatcher(
      declRefExpr(isExpansionInMainFile(), hasDeclaration(namedDecl(hasName("nlanes"))), unless(hasAncestor(varDecl(hasType(arrayType()))))).bind("x"), this);
  finder->addMatcher(
      declRefExpr(isExpansionInMainFile(), hasDeclaration(namedDecl(hasName("nlanes"))), hasAncestor(varDecl(hasType(arrayType())))).bind("array"), this);
}
void NlanesCheck::check(const ast_matchers::MatchFinder::MatchResult &result) {
  const DeclRefExpr *matchedExpr = result.Nodes.getNodeAs<DeclRefExpr>("x");
  const DeclRefExpr *arrayNlanes = result.Nodes.getNodeAs<DeclRefExpr>("array");

  if (matchedExpr && matchedExpr->getNameInfo().getAsString().compare("nlanes") == 0) {
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

  if (arrayNlanes && arrayNlanes->getNameInfo().getAsString().compare("nlanes") == 0) {
    const auto *vecStructDecl = dyn_cast<CXXRecordDecl>(arrayNlanes->getDecl()->getDeclContext()->getParent());
    if (vecStructDecl && vecStructDecl->getName().starts_with("v_")) {  // v_type::nlanes
      SourceManager &SM = result.Context->getSourceManager();
      arrayNlanes->getSourceRange().dump(SM);
      std::string vecTypeName =  // get vector type from source code
          clang::Lexer::getSourceText(
              CharSourceRange::getTokenRange(
                  result.Context->getFullLoc(arrayNlanes->getBeginLoc())),
              SM, result.Context->getLangOpts())
              .str();
      diag(arrayNlanes->getLocation(), "Found nlanes as array size.")
          << FixItHint::CreateReplacement(arrayNlanes->getSourceRange(),
                                          "VTraits<" + vecTypeName + ">::max_nlanes");
    }
  }
}

}  // namespace clang::tidy
