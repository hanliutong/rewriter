#include <clang-tidy/ClangTidyModule.h>
#include <clang-tidy/ClangTidyModuleRegistry.h>

#include "checks/get0_check.hpp"
#include "checks/nlanes_check.hpp"
#include "checks/operator_check.hpp"

namespace clang::tidy {

class OCVModule : public ClangTidyModule {
 public:
  void addCheckFactories(ClangTidyCheckFactories& checkFactories) override {
    checkFactories.registerCheck<NlanesCheck>("nlanes-check");
    checkFactories.registerCheck<OperatorCheck>("operator-check");
    checkFactories.registerCheck<Get0Check>("get0-check");
  }
};

static const char* desc = "Checks and refactor for OpenCV library and its' users";
static ClangTidyModuleRegistry::Add<OCVModule> X("ocv_intrinsic_tidy", desc);

}  // namespace clang::tidy
