#ifndef LANETYPE_CHECK_HPP
#define LANETYPE_CHECK_HPP

#include <clang-tidy/ClangTidyCheck.h>

namespace clang::tidy {

class LaneTypeCheck : public ClangTidyCheck {
 public:
  LaneTypeCheck(StringRef name, ClangTidyContext* context);
  void registerMatchers(ast_matchers::MatchFinder* finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult& result) override;
};

}  // namespace clang::tidy

#endif  // LANETYPE_CHECK_HPP
