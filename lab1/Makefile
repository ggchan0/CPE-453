liblwp.a : lwp.o lwp.c lwp.h
	gcc -m32 -Wall -pedantic -std=gnu99 lwp.c -c
	ar r liblwp.a lwp.o

clean :
	rm *.o
	rm *.a
