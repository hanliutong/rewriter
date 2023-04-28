#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/CommandLine.h>
#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <iostream>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;

class NlanesVisitor : public RecursiveASTVisitor<NlanesVisitor> {
public:
  explicit NlanesVisitor(ASTContext *Context, Rewriter &TheRewriter)
      : Context(Context), TheRewriter(TheRewriter) {}

  bool VisitDeclRefExpr(DeclRefExpr *Expr) {
    if (EnumConstantDecl *EnumDecl = dyn_cast<EnumConstantDecl>(Expr->getDecl())) {
      if (EnumDecl->getNameAsString() == "nlanes") {
        printNlanesRefInfo(Expr);
        rewriteNlanesRef(Expr);
      }
    }
    return true;
  }

private:
  void printNlanesRefInfo(DeclRefExpr *Expr);
  void rewriteNlanesRef(DeclRefExpr *Expr);
  ASTContext *Context;
  Rewriter &TheRewriter;
};

void NlanesVisitor::printNlanesRefInfo(DeclRefExpr *Expr) {
  SourceManager &SM = Context->getSourceManager();
  SourceLocation startLoc = Expr->getBeginLoc();
  SourceLocation endLoc = Expr->getEndLoc();
  FullSourceLoc FullLocation = Context->getFullLoc(Expr->getBeginLoc());
  if (FullLocation.isValid() && SM.isWrittenInMainFile(FullLocation)) {
    std::string code = Lexer::getSourceText(CharSourceRange::getTokenRange(startLoc, endLoc),
                                            SM, Context->getLangOpts()).str();
    llvm::outs() << "Found " <<  code << "\t" << "reference at "
                 << SM.getFilename(FullLocation)
                 << ":" << FullLocation.getSpellingLineNumber()
                 << "\n";
  }
}

void NlanesVisitor::rewriteNlanesRef(DeclRefExpr *Expr) {
  SourceManager &SM = Context->getSourceManager();
  FullSourceLoc FullLocation = Context->getFullLoc(Expr->getBeginLoc());
  if (FullLocation.isValid() && SM.isWrittenInMainFile(FullLocation)) {
    // Get the type of the enclosing class
    if (auto *ParentDecl = dyn_cast<CXXRecordDecl>(Expr->getDecl()->getDeclContext()->getParent())) {
        std::string vecTypeName = Lexer::getSourceText(CharSourceRange::getTokenRange(Context->getFullLoc(Expr->getBeginLoc())),
                                            SM, Context->getLangOpts()).str();
        std::string Replacement = "VTraits<" + vecTypeName + ">::vlanes()";
        TheRewriter.ReplaceText(Expr->getSourceRange(), Replacement);
    }
  }
}


class NlanesASTConsumer : public clang::ASTConsumer {
public:
  explicit NlanesASTConsumer(ASTContext *Context, Rewriter &R)
      : Visitor(Context, R) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  NlanesVisitor Visitor;
};

class NlanesFrontendAction : public clang::ASTFrontendAction {
public:
  NlanesFrontendAction() {}

  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, StringRef file) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<NlanesASTConsumer>(&CI.getASTContext(), TheRewriter);
  }

  void EndSourceFileAction() override {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
  }

private:
  Rewriter TheRewriter;
};

using namespace clang::tooling;

int main(int argc, const char **argv) {
  llvm::cl::OptionCategory MyToolCategory("My tool options");

  auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
  if (!ExpectedParser) {
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser &OptionsParser = ExpectedParser.get();

  std::vector<std::string> sourceFiles;
  std::string inputPath = OptionsParser.getSourcePathList()[0];

  if (llvm::sys::fs::is_directory(inputPath)) {
    // This lambda function checks if a file has a .cpp or .hpp extension
    auto fileFilter = [](const llvm::StringRef &filename) -> bool {
      std::string ext = filename.str();
      ext = llvm::sys::path::extension(ext);
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
      return ext == ".cpp" || ext == ".hpp";
    };

    // Recursively find files with .cpp and .hpp extensions in the input directory
    std::error_code ec;
    llvm::sys::fs::recursive_directory_iterator dir_itr(inputPath, ec), dir_end;

    while (dir_itr != dir_end) {
      llvm::StringRef pathStr = dir_itr->path();
      if (fileFilter(pathStr)) {
        sourceFiles.push_back(pathStr.str());
      }
      dir_itr.increment(ec);
    }
  } else {
    // Single file input
    sourceFiles = OptionsParser.getSourcePathList();
  }

  ClangTool Tool(OptionsParser.getCompilations(), sourceFiles);

  return Tool.run(newFrontendActionFactory<NlanesFrontendAction>().get());
}
