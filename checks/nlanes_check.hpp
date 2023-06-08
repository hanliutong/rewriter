#ifndef NLANES_CHECK_HPP
#define NLANES_CHECK_HPP

#include <clang-tidy/ClangTidyCheck.h>

namespace clang::tidy {

class NlanesCheck : public ClangTidyCheck {
 public:
  NlanesCheck(StringRef name, ClangTidyContext* context);
  void registerMatchers(ast_matchers::MatchFinder* finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult& result) override;

 private:
  std::set<const DeclRefExpr*> processedNlanes;
};

}  // namespace clang::tidy

#endif  // NLANES_CHECK_HPP
