all_progs: nsfind wc

nsfind: nsfind.c ../adt.h ../libadt.a
	c99 -I../ -g -o $@ nsfind.c ../libadt.a

wc: wc.c ../adt.h ../libadt.a
	c99 -I../ -g -o $@ wc.c ../libadt.a
clean:
	rm -fr all_tests *.o nsfind wc *.dSYM
