arch := k1b
system-name := bare

exe: search-k1 search-x86 ../libgpl/libgpl/libgpl.a
all : $(exe)

../libgpl/libgpl/libgpl.a:
	(cd ../libgpl/libgpl/; make -f Makefile.linux)

search-x86: search-bench.c ../libgpl/libgpl/libgpl.a
	gcc -O2 -Wall -Werror -I ../libgpl/include/ -L../libgpl/libgpl  search-bench.c -o search-x86 -lgpl

search-k1-name   := search-k1
search-k1-srcs   := search-bench.c k_memcmp.c
search-k1-cflags := -O3 -g -Wall -Werror -march=k1b -Werror -Wl,--gc-section -I. -DMPPA
search-k1-lflags := -lutask -mhypervisor -lvbsp -lutask -lmppapower -lmppanoc -lmpparouting \

cluster-bin += search-k1

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray

run-bench: search-x86 search-k1
	./search-x86
	k1-jtag-runner --exec-file=Cluster0:output/bin/search-k1

#clean:
#	(cd ../libgpl/libgpl/; make -f Makefile.linux clean)
#	rm -f $(exe)
