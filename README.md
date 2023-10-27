# PART2: Runtime Indirect Call Detection



## How to build and test the dylib:

```bash
# bulid the dylib
cd llvm-pass-icg
rm -rf build && mkdir build && cd build
cmake .. && make
cd ..
# build test case
clang -fpass-plugin=`echo build/icg/icgPass.*` main.c -o main -g
# create input file
echo 1 > input.txt
# indirect call detection
./main < input.txt | grep "__ICG_STDOUT__:" | sort | uniq
```

