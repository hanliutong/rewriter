#include "get0_check.hpp"

#include "clang/Lex/Lexer.h"

namespace clang::tidy {

Get0Check::Get0Check(StringRef name, ClangTidyContext *context)
    : ClangTidyCheck(name, context) {}

void Get0Check::registerMatchers(ast_matchers::MatchFinder *finder) {
  using namespace ast_matchers;
  auto get0Call = cxxMemberCallExpr(on(hasType(namedDecl(matchesName("v_"))))).bind("call");
  finder->addMatcher(get0Call, this);
}

void Get0Check::check(const ast_matchers::MatchFinder::MatchResult &result) {
  const CXXMemberCallExpr *callExpr = result.Nodes.getNodeAs<CXXMemberCallExpr>("call");
  if (callExpr) {
    const MemberExpr *memberExpr = llvm::dyn_cast<MemberExpr>(callExpr->getCallee());
    if (!memberExpr) {
      return;
    }
    const Expr *baseExpr = memberExpr->getBase()->IgnoreParenImpCasts();
    std::string baseExprStr = Lexer::getSourceText(
                                  CharSourceRange::getTokenRange(baseExpr->getSourceRange()),
                                  *result.SourceManager, result.Context->getLangOpts())
                                  .str();
    diag(callExpr->getBeginLoc(), "Found get0() calling")
        << FixItHint::CreateReplacement(callExpr->getSourceRange(), "v_get0(" + baseExprStr + ")");
  }
}

}  // namespace clang::tidy
