libadt.a: error.o string.o vector.o map.o list.o linereader.o tokenizer.o subprocess.o argparse.o file.o http.o regex.o threading.o
	ar rcs $@ $^

error.o: error.c error.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c error.c

string.o: stdtyp/string.c stdtyp/string.h adt.h error.h stdtyp/vector.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c stdtyp/string.c

vector.o: stdtyp/vector.c stdtyp/vector.h adt.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c stdtyp/vector.c

map.o: stdtyp/map.c stdtyp/map.h adt.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c stdtyp/map.c

list.o: stdtyp/list.c stdtyp/list.h adt.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c stdtyp/list.c

subprocess.o: stdtyp/subprocess.c stdtyp/subprocess.h adt.h error.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c stdtyp/subprocess.c

file.o: stdtyp/file.c stdtyp/file.h adt.h error.h stdtyp/string.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c stdtyp/file.c

linereader.o: stdtyp/linereader.c stdtyp/linereader.h adt.h error.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c stdtyp/linereader.c

tokenizer.o: stdtyp/tokenizer.c stdtyp/tokenizer.h adt.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c stdtyp/tokenizer.c

argparse.o: stdtyp/argparse.c stdtyp/map.h adt.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c stdtyp/argparse.c

regex.o: stdtyp/regex.c stdtyp/regex.h adt.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c stdtyp/regex.c

threading.o: stdtyp/threading.c stdtyp/threading.h adt.h error.h stdtyp/vector.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c stdtyp/threading.c

http.o: extyp/http.c extyp/http.h adt.h
	c99 -I./ -D_POSIX_C_SOURCE -D_C99_SOURCE -g -c extyp/http.c

clean:
	rm -rf *.o libadt.a cscope.* *.dSYM

