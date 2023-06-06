#include "opencv2/core/hal/intrin.hpp"

namespace cv {

#if CV_SIMD
void foo(float f) {
  (void)f;
}

void get0(float* a) {
  v_float32 va = vx_load(a);
  float a0 = va.get0();
  (void)a0;
  v_float32 vb;
  vb.get0();
  foo((va + vb).get0());
}

#endif  // CV_SIMD

}  // namespace cv
