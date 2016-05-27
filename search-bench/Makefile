k1-exe := io_main host_main
exe := output.mpk search-k1 search-x86 ../libgpl/libgpl/libgpl.a $(k1-exe)

K1_DIR=$(shell echo $$K1_TOOLCHAIN_DIR)

all : $(exe)

../libgpl/libgpl/libgpl.a:
	(cd ../libgpl/libgpl/; make -f Makefile.linux)

search-x86: search-bench.c ../libgpl/libgpl/libgpl.a
	gcc -O3 -Wall -Werror -I ../libgpl/include/ -L../libgpl/libgpl  search-bench.c -o search-x86 -lgpl

search-k1: search-bench.c ../libgpl/libgpl/libgpl.a kmemcmp/kmemcmp.h io_main host_main
	k1-gcc -g -O3 -Wall -Werror -march=k1b -I ../libgpl/include/ -D MPPA search-bench.c -o search-k1 -mhypervisor -lmppapower -lmppanoc -lmpparouting -lmppa_remote -lmppa_request_engine -lmppanoc

io_main: io_main.c
	k1-gcc -g -O2 -Wall -Werror -march=k1b -mcore=k1bio -o io_main io_main.c -lmppapower -lmppanoc -lmpparouting -lpcie_queue -lmppa_remote -lmppa_request_engine -lmppanoc -mhypervisor -lutask -lvbsp

host_main: host.c
	gcc -o host_main host.c -O2 -I$(K1_DIR)/include -L$(K1_DIR)/lib64 -lmppa_remote -lpcie -pthread

output.mpk: search-k1 io_main host_main
	k1-create-multibinary -T output.mpk -b io_main -c search-k1 -f

PARAMS:=500 16384 32768 65536

run-bench: search-x86 output.mpk $(k1-exe)
	@echo "MHZ  -  keySZ  -  ValueSZ  -  BlkSZ  -  loopNB  -  cache"
	./search-x86 $(PARAMS) 10 0
	./search-x86 $(PARAMS) 2 0
	# k1-jtag-runner --exec-file=Cluster0:search-k1 -- $(PARAMS) 10 0
	# k1-jtag-runner --exec-file=Cluster0:search-k1 -- $(PARAMS) 2 0
	# k1-cluster --march="bostan" --mcore="cluster" --cycle-based -- search-k1 $(PARAMS) 10 0
	# k1-cluster --march="bostan" --mcore="cluster" --cycle-based -- search-k1 $(PARAMS) 2 0
	./host_main output.mpk $(PARAMS) 10 2

clean:
	(cd ../libgpl/libgpl/; make -f Makefile.linux clean)
	rm -f $(exe)

