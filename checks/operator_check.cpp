#include "operator_check.hpp"

#include "clang/Lex/Lexer.h"

namespace clang::tidy {

OperatorCheck::OperatorCheck(StringRef name, ClangTidyContext *context)
    : ClangTidyCheck(name, context) {}

void OperatorCheck::registerMatchers(ast_matchers::MatchFinder *finder) {
  using namespace ast_matchers;
  // match the overloaded operator in Unversal Intrinsc.
  finder->addMatcher(
      // match an overloaded operator
      cxxOperatorCallExpr(
          // which is in the input file and
          isExpansionInMainFile(),
          // has the child that references to
          hasDescendant(declRefExpr(to(
              // a variable with Universal Intrinsic type("v_xxx"), whatever declaration or reference.
              varDecl(hasType(qualType(anyOf(hasDeclaration(namedDecl(matchesName("v_"))),
                                             references(namedDecl(matchesName("v_")))))))))))
          .bind("x"),
      this);
}

void OperatorCheck::check(const ast_matchers::MatchFinder::MatchResult &result) {
  const CXXOperatorCallExpr *matchedExpr = result.Nodes.getNodeAs<CXXOperatorCallExpr>("x");
  if (matchedExpr && matchedExpr->getOperator() != OverloadedOperatorKind::OO_Equal) {
    std::string code = rewriteExpr(matchedExpr, result.Context);
    if (!code.empty()) {
      diag(matchedExpr->getExprLoc(), "Found operator.")
          << FixItHint::CreateReplacement(matchedExpr->getSourceRange(), code);
    } else {  // Unknown operator
      // an error diagnostic has been sent.
    }
  }
}

std::string OperatorCheck::rewriteExpr(const Expr *expr, ASTContext *context) {
  if (const auto *opExpr = dyn_cast<CXXOperatorCallExpr>(expr->IgnoreCasts())) {
    std::string result;
    switch (opExpr->getOperator()) {
      case OverloadedOperatorKind::OO_PlusEqual:  // "+="
      case OverloadedOperatorKind::OO_Plus:       // "+"
        result = "v_add(";
        break;
      case OverloadedOperatorKind::OO_MinusEqual:  // "-="
      case OverloadedOperatorKind::OO_Minus:       // "-"
        result = "v_sub(";
        break;
      case OverloadedOperatorKind::OO_StarEqual:  // "*="
      case OverloadedOperatorKind::OO_Star:       // "*"
        result = "v_mul(";
        break;
      case OverloadedOperatorKind::OO_SlashEqual:  // "/="
      case OverloadedOperatorKind::OO_Slash:       // "/"
        result = "v_div(";
        break;
      case OverloadedOperatorKind::OO_AmpEqual:  // "&="
      case OverloadedOperatorKind::OO_Amp:       // "&"
        result = "v_and(";
        break;
      case OverloadedOperatorKind::OO_PipeEqual:  // "|="
      case OverloadedOperatorKind::OO_Pipe:       // "|"
        result = "v_or(";
        break;
      case OverloadedOperatorKind::OO_CaretEqual:  // "^="
      case OverloadedOperatorKind::OO_Caret:       // "^"
        result = "v_xor(";
        break;
      case OverloadedOperatorKind::OO_EqualEqual:  // "=="
        result = "v_eq(";
        break;
      case OverloadedOperatorKind::OO_ExclaimEqual:  // "!="
        result = "v_ne(";
        break;
      case OverloadedOperatorKind::OO_Less:  // "<"
        result = "v_lt(";
        break;
      case OverloadedOperatorKind::OO_Greater:  // ">"
        result = "v_gt(";
        break;
      case OverloadedOperatorKind::OO_LessEqual:  // "<="
        result = "v_le(";
        break;
      case OverloadedOperatorKind::OO_GreaterEqual:  // ">="
        result = "v_ge(";
        break;
      case OverloadedOperatorKind::OO_LessLess:  // "<<"
        result = "v_shl<";
        result += rewriteExpr(opExpr->getArg(1)->IgnoreCasts(), context);
        result += ">(";
        result += rewriteExpr(opExpr->getArg(0)->IgnoreCasts(), context);
        result += ")";
        return result;
        break;
      case OverloadedOperatorKind::OO_GreaterGreater:  // ">>"
        result = "v_shr<";
        result += rewriteExpr(opExpr->getArg(1)->IgnoreCasts(), context);
        result += ">(";
        result += rewriteExpr(opExpr->getArg(0)->IgnoreCasts(), context);
        result += ")";
        return result;
        break;
      case OverloadedOperatorKind::OO_Tilde:
        result = "v_not(" + rewriteExpr(opExpr->getArg(0)->IgnoreCasts(), context) + ")";
        return result;
        break;
      default:
        diag(opExpr->getExprLoc(), "Unknown operator %0 in Unviersal Intrinsic.\n", DiagnosticIDs::Error)
            << getOperatorSpelling(opExpr->getOperator());
        return "";
        break;
    }  // switch

    if (opExpr->isAssignmentOp()) {  // "res += a;" -> "res = v_add(res, a);"
      std::string leftSrc = rewriteExpr(opExpr->getArg(0)->IgnoreCasts(), context);
      result += leftSrc;
      result += ", ";
      result += rewriteExpr(opExpr->getArg(1)->IgnoreCasts(), context);
      result = leftSrc + " = " + result + ")";
    } else {  // "res = a + b;" -> "res = v_add(a, b);"
      result += rewriteExpr(opExpr->getArg(0)->IgnoreCasts(), context);
      result += ", ";
      result += rewriteExpr(opExpr->getArg(1)->IgnoreCasts(), context);
      result += ")";
    }
    return result;

  } else if (const auto *parenExpr = dyn_cast<ParenExpr>(expr->IgnoreCasts())) {
    return rewriteExpr(parenExpr->getSubExpr()->IgnoreCasts(), context);

  } else {  // expr is neither ParenExpr nor CXXOperatorCallExpr.
    std::string result;
    llvm::raw_string_ostream OS(result);
    expr->printPretty(OS, nullptr, PrintingPolicy(context->getLangOpts()));
    return result;
  }
}

}  // namespace clang::tidy
