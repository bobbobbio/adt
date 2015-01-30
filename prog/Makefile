all_progs: nsfind wc ncl

nsfind: nsfind.c ../adt.h ../libadt.a
	c99 -I../ -g -o $@ nsfind.c ../libadt.a

wc: wc.c ../adt.h ../libadt.a
	c99 -I../ -g -o $@ wc.c ../libadt.a

ncl: ncl.c ../adt.h ../libadt.a
	c99 -I../ -g -o $@ ncl.c ../libadt.a

clean:
	rm -fr all_tests *.o nsfind wc ncl *.dSYM
