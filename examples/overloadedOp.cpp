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

#endif  // CV_SIMD

}  // namespace cv