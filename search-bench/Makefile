all: search-k1 search-x86 ../libgpl/libgpl/libgpl.a

../libgpl/libgpl/libgpl.a:
	(cd ../libgpl/libgpl/; make -f Makefile.linux)

search-x86: search-bench.c ../libgpl/libgpl/libgpl.a
	gcc  -Wall -Werror -I ../libgpl/include/ -L../libgpl/libgpl  search-bench.c -o search-x86 -lgpl

search-k1: search-bench.c ../libgpl/libgpl/libgpl.a
	k1-gcc -Wall -Werror -march=k1a -I ../libgpl/include/ -D MPPA search-bench.c -o search-k1 -lutask -mhypervisor -lvbsp -lutask -lmppapower -lmppanoc -lmpparouting

run-bench: search-x86 search-k1
	   ./search-x86
	   k1-cluster --cycle-based -- search-k1
