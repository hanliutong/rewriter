#include <clang-tidy/ClangTidyModule.h>
#include <clang-tidy/ClangTidyModuleRegistry.h>

#include "checks/nlanes_check.hpp"

namespace clang::tidy {

class OCVModule : public ClangTidyModule {
 public:
  void addCheckFactories(ClangTidyCheckFactories& checkFactories) override {
    checkFactories.registerCheck<NlanesCheck>("nlanes-check");
  }
};

static const char* desc = "Checks and refactor for OpenCV library and its' users";
static ClangTidyModuleRegistry::Add<OCVModule> X("ocv_intrinsic_tidy", desc);

}  // namespace clang::tidy
