#ifndef GET0_CHECK_HPP
#define GET0_CHECK_HPP

#include <clang-tidy/ClangTidyCheck.h>

namespace clang::tidy {

class Get0Check : public ClangTidyCheck {
 public:
  Get0Check(StringRef name, ClangTidyContext* context);
  void registerMatchers(ast_matchers::MatchFinder* finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult& result) override;
};

}  // namespace clang::tidy

#endif  // GET0_CHECK_HPP
