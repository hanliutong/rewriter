# Work-in-process

# Code Rewriter for OpenCV Universal Intrinsic

The API of OpenCV Universal Intrinsic is introduced some compatibility-breaking changes, in order to support variable-length backends including RVV. Most code blocks written using Universal Intrinsic will need to be rewritten. However, there are more than 400 code blocks that need to be rewritten, distributed in about 50 different files. This will be a horrible work if done by manually, so we decided to develop an automatic rewriter.

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

## Usage

Need to compile OpenCV (with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`) and LLVM (enable clang and clang-tools-extra) firstly,

and then:

```bash
# build
mkdir build && cd build
cmake .. -G Ninja -DLLVM_BUILD_DIR=<LLVM build directory>
ninja

# run
./ocv_rewriter -p <path to opencv build directory> <source file>
```