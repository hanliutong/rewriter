#include "lanetype_check.hpp"

#include "clang/Lex/Lexer.h"

namespace clang::tidy {

LaneTypeCheck::LaneTypeCheck(StringRef name, ClangTidyContext *context)
    : ClangTidyCheck(name, context) {}

void LaneTypeCheck::registerMatchers(ast_matchers::MatchFinder *finder) {
  using namespace ast_matchers;
  finder->addMatcher(
      varDecl(hasType(arrayType(hasElementType(qualType(hasDeclaration(typedefDecl(hasName("lane_type"), hasParent(recordDecl())))))))).bind("var"), this);
  finder->addMatcher(
      varDecl(hasType(qualType(hasDeclaration(typedefDecl(hasName("lane_type"), hasParent(recordDecl())))))).bind("var"), this);
}

void LaneTypeCheck::check(const ast_matchers::MatchFinder::MatchResult &result) {
  const auto *var = result.Nodes.getNodeAs<VarDecl>("var");
  // const auto *vType = result.Nodes.getNodeAs<RecordDecl>("vType");
  if (var) {
    auto typeLoc = var->getTypeSourceInfo()->getTypeLoc();
    std::string typeName;
    auto &srcMgr = *result.SourceManager;
    // Check if the type is an array type
    if (auto arrayTypeLoc = typeLoc.getAs<ArrayTypeLoc>()) {
      auto elementTypeLoc = arrayTypeLoc.getElementLoc();
      typeName = Lexer::getSourceText(CharSourceRange::getTokenRange(elementTypeLoc.getBeginLoc(), elementTypeLoc.getEndLoc()),
                                      srcMgr, result.Context->getLangOpts())
                     .str();
    } else {
      typeName = Lexer::getSourceText(CharSourceRange::getTokenRange(typeLoc.getBeginLoc(), typeLoc.getEndLoc()),
                                      srcMgr, result.Context->getLangOpts())
                     .str();
    }
    // Remove "::lane_type" and "typename " from typeName
    auto pos_lane_type = typeName.find("::lane_type");
    auto pos_typename = typeName.find("typename ");
    if (pos_lane_type != std::string::npos) {
      typeName.erase(pos_lane_type, std::strlen("::lane_type"));
    }
    if (pos_typename != std::string::npos) {
      typeName.erase(pos_typename, std::strlen("typename "));
    }
    // Generate the fix-it hint
    if (typeLoc.getAs<ArrayTypeLoc>()) {
      auto elementTypeLoc = typeLoc.castAs<ArrayTypeLoc>().getElementLoc();
      diag(elementTypeLoc.getBeginLoc(), "Found lane_type")
          << FixItHint::CreateReplacement(elementTypeLoc.getSourceRange(),
                                          "typename VTrait<" + typeName + ">::lane_type");
    } else {
      diag(typeLoc.getBeginLoc(), "Found lane_type")
          << FixItHint::CreateReplacement(typeLoc.getSourceRange(),
                                          "VTrait<" + typeName + ">::lane_type");
    }
  }
}

}  // namespace clang::tidy
