#include "opencv2/core/hal/intrin.hpp"

namespace cv {

struct testStruct {
  int value;

  testStruct(int v) : value(v) {}

  testStruct operator+(const testStruct &other) {
    return testStruct(value + other.value);
  }
};

void doNotMatchThis() {
  testStruct a = testStruct(1);
  testStruct b = testStruct(1);
  testStruct c = a + b;
  (void)c;
}

#if CV_SIMD
void vcompare(v_float32 &a, v_float32 &b);
void vcompute(v_float32 &a, v_float32 &b);
void vlogic(v_uint16 &a, v_uint16 &b);

void vcompare(v_float32 &a, v_float32 &b) {
  v_float32 res = a == b;
  res = a != b;
  res = a < b;
  res = a > b;
  res = a <= b;
  res = a >= b;

  (void)res;
}

void vcompute(v_float32 &a, v_float32 &b, v_float32 &c, v_float32 &d) {
  v_float32 res = a + b;
  res = a - b;
  res = a * b;
  res = a / b;
  res = res + a + b;
  res = res * a * b;
  res = a + b - (c + d);
  res = (a + b) - (c + d);
  res = a + b * c - d;
  res = a + b * (c - a) / d;
  res += a;
  res -= b;
  res *= c;
  res /= d;

  (void)res;
}

void vshift(v_uint32 &a) {
  a << 5;
  a >> 10;
}

void vlogic(v_uint16 &a, v_uint16 &b) {
  v_uint16 res = a & b;
  res = res & a & b;
  res = a | b;
  res = res | a | b;
  res = a ^ b;
  res = res ^ a ^ b;
  res = ~res;
  res = res | (~a) & b;
  res &= a;
  res |= a;
  res ^= a;

  (void)res;
}

template <class T>
T matchThis(T &a, T &b) {
  return a - b;
}

void foo(v_uint16 &a, v_uint16 &b) {
  matchThis(a, b);
}

testStruct getObj(int input) {
  return testStruct(input);
}

void vecTypeDef(float *src) {
  typedef v_float32 v_type;
  v_type v0 = vx_load(src);
  v_type v1 = vx_load(src);
  v0 |= v1;
  v_type v2 = vx_load(src) + vx_load(src);

  typedef testStruct s_type;
  s_type s0 = s_type(0);
  s_type s1 = s_type(1);
  // should not match the following code.
  s0 = s0 + s1;
  s_type s2 = getObj(1) + getObj(2);
}

#endif  // CV_SIMD

}  // namespace cv