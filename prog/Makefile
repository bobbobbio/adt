all_progs: nsfind wc ncl http_server

nsfind: nsfind.c ../adt.h ../libadt.a
	c99 -I../ -g -o $@ nsfind.c ../libadt.a

wc: wc.c ../adt.h ../libadt.a
	c99 -I../ -g -o $@ wc.c ../libadt.a

ncl: ncl.c ../adt.h ../libadt.a
	c99 -I../ -g -o $@ ncl.c ../libadt.a

http_server: http_server.c ../adt.h ../libadt.a
	c99 -I../ -g -o $@ http_server.c ../libadt.a

clean:
	rm -fr all_tests *.o nsfind wc ncl *.dSYM
