
# Build commands for bitcode
.SUFFIXES: .c .bc

%.bc : %.c
	clang -g -O1 -c -emit-llvm $< -o $@
# End build commands for bitcode

swap: swap.c
	clang -g -O1 swap.c -o swap

output: swap.bc swap
	./swap > output
