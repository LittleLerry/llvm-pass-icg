# PART2: Runtime Indirect Call Detection

## Methodology
### Introduction
We run the target programme and monitor any indirect call during runtime. Before each 
indirect call, we will insert an instruction and a function call. The first instruction 
will set a global variable `icf` to `1`, which means that we will run do an indirect 
jump. The function call will print the caller function to STDOUT, cause the author cannot
handle the `String` modification in LLVM PASS.

And for each basic block of Function `F`, we will check the global variable `icf`. We should
print the name of current running function if `icf == 1` and set `icf` to 0 after the checking. 
However, the author also doesn't know how to insert extra basic blocks in LLVM PASS. So he 
has to output all function names with indirect function call flags attached to them. We will 
use another programme to filter out suitable outputs.

** Above statement will probabily NOT be hold for multi-threaded programmes. And the author 
** has completely no idea what would happen if the target program were multi-threaded.

### Indirect Call Instrumentation and Monitoring

TBD



## Integrated into AFLGO

TBD

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

