#include "nlanes_check.hpp"
#include "clang/Lex/Lexer.h"

namespace clang::tidy {

NlanesCheck::NlanesCheck(StringRef name, ClangTidyContext *context)
    : ClangTidyCheck(name, context) {}

void NlanesCheck::registerMatchers(ast_matchers::MatchFinder *finder) {
  using namespace ast_matchers;
  finder->addMatcher(
      declRefExpr(hasDeclaration(namedDecl(matchesName("nlanes")))).bind("x"), this);
}

void NlanesCheck::check(const ast_matchers::MatchFinder::MatchResult &result) {
  const auto *matchedExpr = result.Nodes.getNodeAs<DeclRefExpr>("x");
  if (!matchedExpr)
    return;  // Not found any usage of nlanes

  const auto *vecStructDecl = dyn_cast<CXXRecordDecl>(
      matchedExpr->getDecl()->getDeclContext()->getParent());
  if (!vecStructDecl)
    return;  // Ignore nlanes that are not class members, e.g. `int nlanes = 8`

  if (vecStructDecl->getName().compare("VTraits") == 0)
    return;  // Already using the new API, no modification required.

  if (matchedExpr->getNameInfo().getAsString().compare("nlanes") == 0 &&
      vecStructDecl->getName().starts_with("v_")) {  // v_type::nlanes
    SourceManager &SM = result.Context->getSourceManager();
    std::string vecTypeName =  // get vector type from source code
        clang::Lexer::getSourceText(
            CharSourceRange::getTokenRange(
                result.Context->getFullLoc(matchedExpr->getBeginLoc())),
            SM, result.Context->getLangOpts())
            .str();
    auto parent = result.Context->getParents(*matchedExpr)[0];
    if (const TypeLoc *parentTypeLoc = parent.get<TypeLoc>()) {
      // v_type::nlanes is used as the array size,
      // so it needs to be a compile-time constant
      if (const ArrayTypeLoc arrTypeLoc =
              parentTypeLoc->getAs<ArrayTypeLoc>()) {
        diag(matchedExpr->getLocation(), "Found nlanes as array size.")
            << FixItHint::CreateReplacement(
                   arrTypeLoc.getSizeExpr()->getSourceRange(),
                   "VTraits<" + vecTypeName + ">::max_nlanes");
      }
    } else {
      diag(matchedExpr->getLocation(), "Found nlanes.")
          << FixItHint::CreateReplacement(parent.getSourceRange(),
                                          "VTraits<" + vecTypeName + ">::vlanes()");
    }
  } else if (matchedExpr) {  // vecStructDecl is not start with "v_"
    diag(matchedExpr->getLocation(),
         "Found others nlanes, maybe not a member of the Universal Intrinsic type",
         clang::DiagnosticIDs::Note);
  }
}

}  // namespace clang::tidy
