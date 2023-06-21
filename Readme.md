# Work-in-process

# Code Rewriter for OpenCV Universal Intrinsic

The API of OpenCV Universal Intrinsic is introduced some compatibility-breaking changes, in order to support variable-length backends including RVV. Most code blocks written using Universal Intrinsic will need to be rewritten. However, there are more than 400 code blocks that need to be rewritten, distributed in about 50 different files. This will be a horrible work if done by manually, so we decided to develop an automatic rewriter.

We are going to create a clang-tidy plugin to match and fix the Universal Intrinsic API that needs to be rewritten.

## Example

```diff
void vadd(float *a, float *b, float *c, size_t n) {
-   for (size_t i = 0; i < n; i +=  v_float32::nlanes) {
+   for (size_t i = 0; i < n; i +=  Vtraits<v_float32>::vlanes()) {
            v_float32 va = v_load(a + i);
            v_float32 vb = v_load(b + i);
-           v_float32 vc = va + vb;
+           v_float32 vc = v_add(va, vb);
            v_store(c, vc);
    }
}
```

## Capability

| Code usage             | Match              | Fix                |
| ---------------------- | ------------------ | ------------------ |
| v_type::nlanes            | √                  | √                  |
| v_type::nlanes (constant) | only as array size | only as array size |
| overloaded operator    | √                  | √                  |
| v_type::lane_type      | √                  | √                  |
| get0()                 | √                  | √                  |
| v_extract_n            | √                  | √                  |
| v_broadcast_element    | √                  | √                  |
| ...                    |                    |                    |



## Usage

Need to compile OpenCV (with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`) and LLVM (enable clang and clang-tools-extra) firstly,

and then:

```bash
# build
mkdir build && cd build
cmake .. -G Ninja -DClang_DIR=<LLVM build directory>/lib/cmake/clang
ninja

# run
<LLVM build directory>/bin/clang-tidy --load ./build/libocv_intrinsic_tidy.so '--checks=-*,nlanes-check' -p <OpenCV build directory> ../examples/nlanes.cpp

# run and rewrite
<LLVM build directory>/bin/clang-tidy --load ./build/libocv_intrinsic_tidy.so '--checks=-*,nlanes-check' -p <OpenCV build directory> ../examples/nlanes.cpp -fix
```
