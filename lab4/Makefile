all : libDisk.o libTinyFS.o tinyFsDemo.o
	gcc libDisk.o libTinyFS.o tinyFsDemo.o -o tinyFsDemo

libDisk.o : libDisk.c libDisk.h tinyFS.h
	gcc -std=gnu99 -Wall -pedantic -c libDisk.c

libTinyFS.o : libTinyFS.h tinyFS.h libTinyFS.c TinyFS_errno.h
	gcc -std=gnu99 -Wall -pedantic -c libTinyFS.c

tinyFsDemo.o : tinyFsDemo.c
	gcc -std=gnu99 -Wall -pedantic -c tinyFsDemo.c

clean :
	rm *.o
