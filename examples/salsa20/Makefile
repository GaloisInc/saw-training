%.bc : %.c
	clang -g -O1 -c -emit-llvm $< -o $@
	
verify-compositional: salsa20.bc salsa20_compositional.saw
	saw salsa20_compositional.saw

verify-noncompositional: salsa20.bc salsa20_noncompositional.saw
	saw salsa20_noncompositional.saw

clean:
	rm -rf *.o *.bc
