
# Build commands for swap
swap.bc: swap.c
	clang -O1 -c -emit-llvm swap.c -o swap.bc
# End build commands for swap

swap: swap.c
	clang -O1 swap.c -o swap

output: swap.bc swap
	./swap > output
