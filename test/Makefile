all_tests: test_main.o map_test set_test linereader_test vector_test list_test file_test http_test argparse_test regex_test ../libadt.a
	(cd ..; make)
	c99 -I../ -g -o $@ test_main.o ../libadt.a

test_main.o: test_main.c ../adt.h
	c99 -I../ -g -c test_main.c 

map_test: map_test.c ../adt.h ../stdtyp/map.h ../libadt.a
	c99 -I../ -g -o $@ map_test.c ../libadt.a

set_test: set_test.c ../adt.h ../stdtyp/set.h ../libadt.a
	c99 -I../ -g -o $@ set_test.c ../libadt.a

linereader_test: linereader_test.c ../adt.h ../stdtyp/linereader.h ../libadt.a
	c99 -I../ -g -o $@ linereader_test.c  ../libadt.a

vector_test: vector_test.c ../adt.h ../stdtyp/vector.h ../libadt.a
	c99 -I../ -g -o $@ vector_test.c  ../libadt.a

list_test: list_test.c ../adt.h ../stdtyp/list.h ../libadt.a
	c99 -I../ -g -o $@ list_test.c  ../libadt.a

file_test: file_test.c ../adt.h ../stdtyp/string.h ../stdtyp/file.h ../libadt.a
	c99 -I../ -g -o $@ file_test.c  ../libadt.a

http_test: http_test.c ../extyp/http.h ../libadt.a
	c99 -I../ -g -o $@ http_test.c  ../libadt.a

argparse_test: argparse_test.c ../stdtyp/argparse.h ../libadt.a
	c99 -I../ -g -o $@ argparse_test.c  ../libadt.a

regex_test: regex_test.c ../stdtyp/regex.h ../libadt.a
	c99 -I../ -g -o $@ regex_test.c  ../libadt.a
clean:
	rm -fr all_tests *.o *_test *.dSYM
