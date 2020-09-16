s2n_hmac_old.bc: s2n_hmac_old.c
	clang -O1 -c -emit-llvm s2n_hmac_old.c -o s2n_hmac_old.bc

s2n_hmac_new.bc: s2n_hmac_new.c
	clang -O1 -c -emit-llvm s2n_hmac_new.c -o s2n_hmac_new.bc

clean:
	rm -rf *.o *.bc
