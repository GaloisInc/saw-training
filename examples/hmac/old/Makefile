s2n_hmac.bc: s2n_hmac.c
	clang -O1 -c -emit-llvm s2n_hmac.c -o s2n_hmac.bc

clean:
	rm -rf *.o *.bc
