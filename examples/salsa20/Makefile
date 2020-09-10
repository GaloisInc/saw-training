salsa20.bc: salsa20.c
	clang -O1 -c -emit-llvm salsa20.c -o salsa20.bc

verify: salsa20.bc salsa20.saw
	saw salsa20.saw

clean:
	rm -rf *.o *.bc
