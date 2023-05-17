#include "opencv2/core/hal/intrin.hpp"

namespace cv {

// #if CV_SIMD
void vadd(float *a, float *b, float *c, size_t n);

class test {
 public:
  enum { nlanes = 1 };  // should NOT be matched
};

void vadd(float *a, float *b, float *c, size_t n) {
  float ArrayDef0[v_int64::nlanes] = {0};  // VTraits<v_Ty>::max_nlanes (A constant)
  (void)ArrayDef0;

  // FIXIT: can not handle now
  float ArrayDef1[v_int8::nlanes - v_int32::nlanes];  // VTraits<v_Ty>::max_nlanes (A constant)
  (void)ArrayDef1;

  int nlanes = 8;  // should NOT be matched

  float ArrayDef2[nlanes];  // should NOT be matched
  (void)ArrayDef2;

  float ArrayDef3[test::nlanes];  // should NOT be matched
  (void)ArrayDef3;

  int usage0 = v_float32::nlanes + (short)v_int8::nlanes;  // VTraits<v_Ty>::vlanes()
  (void)usage0;

  float usage1 = ArrayDef0[v_int64::nlanes - 1];  // VTraits<v_Ty>::vlanes()
  (void)usage1;

  for (size_t i = 0; i < n; i += v_float32::nlanes) {  // VTraits<v_Ty>::vlanes()
    v_float32 va = v_load(a + i);
    v_float32 vb = v_load(b + i);
    v_float32 vc = v_add(va, vb);
    v_store(c, vc);
  }
}

// #endif // CV_SIMD

}  // namespace cv
