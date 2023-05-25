#ifndef OPERATOR_CHECK_HPP
#define OPERATOR_CHECK_HPP

#include <clang-tidy/ClangTidyCheck.h>

namespace clang::tidy {

class OperatorCheck : public ClangTidyCheck {
 public:
  OperatorCheck(StringRef name, ClangTidyContext* context);
  void registerMatchers(ast_matchers::MatchFinder* finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult& result) override;

 private:
  std::string rewriteExpr(const Expr* expr, ASTContext* context);
};

}  // namespace clang::tidy

#endif  // OPERATOR_CHECK_HPP
