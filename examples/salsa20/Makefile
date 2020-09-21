salsa20.bc: salsa20.c
	clang -O1 -c -emit-llvm salsa20.c -o salsa20.bc

verify-compositional: salsa20.bc salsa20_compositional.saw
	saw salsa20_compositional.saw

verify-noncompositional: salsa20.bc salsa20_noncompositional.saw
	saw salsa20_noncompositional.saw

clean:
	rm -rf *.o *.bc
