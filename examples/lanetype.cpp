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

template <typename scalar_t>
struct zero_vec_type_of;

template <typename scalar_t>
using zero_vec_type_of_t = typename zero_vec_type_of<scalar_t>::type;

template <>
struct zero_vec_type_of<ushort> {
  using type = v_int16;
};
template <>
struct zero_vec_type_of<short> {
  using type = v_int16;
};
template <typename SRC>
int div_simd_common(const SRC in1[]) {
  zero_vec_type_of_t<SRC> v_zero = vx_setall<typename zero_vec_type_of_t<SRC>::lane_type>(0);
}
int div_simd_common(const short in1[]);
int div_simd_common(const ushort in1[]);

template <typename _Tdvec>
static inline void vx_load_pair_as(const double* ptr, _Tdvec& a, _Tdvec& b) {
  // opencv/modules/imgproc/src/morph.simd.hpp
  typedef typename _Tdvec::lane_type template_stype;  // typedef typename VTraits<_Tdvec>::lane_type template_stype;
  const int VECSZ = _Tdvec::nlanes;
  // opencv/modules/core/src/convert.hpp
  typename _Tdvec::lane_type buf[VECSZ * 2];

  buf[0] = saturate_cast<typename _Tdvec::lane_type>(ptr[0]);
}

void foo(double* p, v_int16& a, v_uint16& b) {
  typedef typename v_int32::lane_type stype;
  typedef int stype_int;
  vx_load_pair_as(p, a, a);
  vx_load_pair_as(p, b, b);
}
#endif
}  // namespace cv
