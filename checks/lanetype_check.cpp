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

  finder->addMatcher(
      varDecl(hasDescendant(callExpr(hasDescendant(unresolvedLookupExpr().bind("lookup"))))), this);

  finder->addMatcher(
      typedefDecl(hasType(elaboratedType(hasQualifier(nestedNameSpecifier())).bind("contextType"))).bind("typedefDecl"), this);
}

void LaneTypeCheck::check(const ast_matchers::MatchFinder::MatchResult &result) {
  const auto *var = result.Nodes.getNodeAs<VarDecl>("var");
  const auto *typedefDecl = result.Nodes.getNodeAs<TypedefDecl>("typedefDecl");
  const auto *contextType = result.Nodes.getNodeAs<ElaboratedType>("contextType");
  const auto *unresolvedLookupExpr = result.Nodes.getNodeAs<UnresolvedLookupExpr>("lookup");
  if (unresolvedLookupExpr &&
      unresolvedLookupExpr->getTemplateArgs() &&
      unresolvedLookupExpr->getTemplateArgs()->getArgument().isNull() == false &&
      unresolvedLookupExpr->getTemplateArgs()->getArgument().getKind() ==
          TemplateArgument::ArgKind::Type) {
    auto typeStr = unresolvedLookupExpr->getTemplateArgs()->getArgument().getAsType().getUnqualifiedType().getAsString();
    auto pos_lane_type = typeStr.find("::lane_type");
    if (pos_lane_type != std::string::npos) {
      typeStr.insert(pos_lane_type, ">");
      auto pos_typename = typeStr.find("typename ");
      typeStr.insert(pos_typename == std::string::npos ? 0 : pos_typename + std::strlen("typename "), "VTraits<");
      diag(unresolvedLookupExpr->getTemplateArgs()->getLocation(), "Found lane_type as template parameter")
          << FixItHint::CreateReplacement(unresolvedLookupExpr->getTemplateArgs()->getSourceRange(), typeStr);
    }
  }

  if (typedefDecl && contextType) {
    auto *td = contextType->getAs<TypedefType>();
    if (td && td->getDecl()->getNameAsString().compare("lane_type") == 0) {
      std::string typedefDeclStr = Lexer::getSourceText(CharSourceRange::getTokenRange(typedefDecl->getSourceRange()),
                                                        *result.SourceManager, result.Context->getLangOpts())
                                       .str();
      if (auto *templateParmType = contextType->getQualifier()->getAsType()->getAs<SubstTemplateTypeParmType>()) {
        std::string parmStr = templateParmType->getReplacedParameter()->getNameAsString();
        std::string replacement = "VTraits<" + parmStr + ">";
        typedefDeclStr.replace(typedefDeclStr.find(parmStr), parmStr.length(), replacement);
        diag(typedefDecl->getLocation(), "Found lane_type as typedef")
            << FixItHint::CreateReplacement(typedefDecl->getSourceRange(), typedefDeclStr);
      } else if (auto *recordType = contextType->getQualifier()->getAsType()->getAs<RecordType>()) {
        std::string typedefDeclStr = Lexer::getSourceText(CharSourceRange::getTokenRange(typedefDecl->getSourceRange()),
                                                          *result.SourceManager, result.Context->getLangOpts())
                                         .str();
        std::string typeStr = Lexer::getSourceText(CharSourceRange::getTokenRange(typedefDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange()),
                                                   *result.SourceManager, result.Context->getLangOpts())
                                  .str();
        // Remove "::lane_type" and "typename " from typeName
        auto pos_lane_type = typeStr.find("::lane_type");
        auto pos_typename = typeStr.find("typename ");
        if (pos_lane_type != std::string::npos) {
          typeStr.erase(pos_lane_type, std::strlen("::lane_type"));
        }
        if (pos_typename != std::string::npos) {
          typeStr.erase(pos_typename, std::strlen("typename "));
        }
        std::string replacement = "VTraits<" + typeStr + ">";
        typedefDeclStr.replace(typedefDeclStr.find(typeStr), typeStr.length(), replacement);
        diag(typedefDecl->getLocation(), "Found lane_type as typedef")
            << FixItHint::CreateReplacement(typedefDecl->getSourceRange(), typedefDeclStr);
      }
    }
  }

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
