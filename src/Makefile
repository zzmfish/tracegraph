

all:
	gcc -fPIC -I../include -c tracegraph.c
	gcc -shared -o libtracegraph.so tracegraph.o
	gcc -I../include -c tracegraph.c
	ar rcs libtracegraph.a tracegraph.o
