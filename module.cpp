#include <clang-tidy/ClangTidyModule.h>
#include <clang-tidy/ClangTidyModuleRegistry.h>

#include "checks/get0_check.hpp"
#include "checks/lanetype_check.hpp"
#include "checks/nlanes_check.hpp"
#include "checks/operator_check.hpp"

namespace clang::tidy {

class OCVModule : public ClangTidyModule {
 public:
  void addCheckFactories(ClangTidyCheckFactories& checkFactories) override {
    checkFactories.registerCheck<NlanesCheck>("opencv-dev-intrin-nlanes");
    checkFactories.registerCheck<OperatorCheck>("opencv-dev-intrin-operator");
    checkFactories.registerCheck<Get0Check>("opencv-dev-intrin-get0");
    checkFactories.registerCheck<LaneTypeCheck>("opencv-dev-intrin-lanetype");
  }
};

static const char* desc = "Checks and refactor for OpenCV library and its' users";
static ClangTidyModuleRegistry::Add<OCVModule> X("ocv_intrinsic_tidy", desc);

}  // namespace clang::tidy
