#include "opencv2/core/hal/intrin.hpp"

namespace cv {
#if CV_SIMD

template <typename T1, typename T2, typename Tvec>
struct op_div_scale {
  typedef typename Tvec::lane_type T;
  static inline void pre(const Tvec& denom, const Tvec& res) {
    denom = vx_setall<typename Tvec::lane_type>(0);
  }
};

template <typename _Tdvec>
static inline void vx_load_pair_as(const double* ptr, _Tdvec& a, _Tdvec& b) {
  typedef typename _Tdvec::lane_type template_stype;
  const int VECSZ = _Tdvec::nlanes;
  typename _Tdvec::lane_type buf[VECSZ * 2];
  buf[0] = saturate_cast<typename _Tdvec::lane_type>(ptr[0]);
}
void foo(double* p, v_int32& a, v_uint32& b) {
  typedef typename v_int32::lane_type stype;
  vx_load_pair_as(p, a, a);
  vx_load_pair_as(p, b, b);
}
#endif
}  // namespace cv
