exe := search-k1 search-x86 ../libgpl/libgpl/libgpl.a
all : $(exe)

../libgpl/libgpl/libgpl.a:
	(cd ../libgpl/libgpl/; make -f Makefile.linux)

search-x86: search-bench.c ../libgpl/libgpl/libgpl.a
	gcc -O3 -Wall -Werror -I ../libgpl/include/ -L../libgpl/libgpl  search-bench.c -o search-x86 -lgpl

search-k1: search-bench.c ../libgpl/libgpl/libgpl.a
	k1-gcc -O3 -Wall -Werror -march=k1b -I ../libgpl/include/ -D MPPA search-bench.c -o search-k1 -lutask -mhypervisor -lvbsp -lutask -lmppapower -lmppanoc -lmpparouting

run-bench: search-x86 search-k1
	@echo "MHZ  -  keySZ  -  ValueSZ  -  BlkSZ  -  loopNB  -  cache"
	./search-x86 600 10 100 65536 10 0
	k1-jtag-runner --exec-file=Cluster0:search-k1 -- 600 10 100 65536 10 0
	k1-cluster --march="bostan" --mcore="cluster" --cycle-based -- search-k1 600 10 100 65536 10 0
clean:
	(cd ../libgpl/libgpl/; make -f Makefile.linux clean)
	rm -f $(exe)

