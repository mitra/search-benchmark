#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mppa_power.h>
#include <pcie.h>
#include <mppa_rpc.h>
#include <mppa_remote.h>
#include <utask.h>

int main(int argc, const char **argv){

	mppadesc_t pcie_fd = 0;
	if (__k1_spawn_type() == __MPPA_PCI_SPAWN) {
		pcie_fd = pcie_open(0);
		pcie_queue_init(pcie_fd);
		pcie_register_console(pcie_fd, stdin, stdout);
	}

	mppa_rpc_server_init(1, 0, 1);
	mppa_remote_server_init(pcie_fd, 1);

	utask_t rm1;

	int blk_sz = atoi(argv[5]);
	int key_sz = atoi(argv[3]);
	int value_sz = atoi(argv[4]);

				
	if(blk_sz < (key_sz + value_sz + 16))
		return 0;
	
	
	utask_create(&rm1, NULL, (void *) mppa_rpc_server_start, NULL);
	mppa_power_base_spawn(0, "search-k1", argv, NULL, MPPA_POWER_SHUFFLING_ENABLED);
   
	
	int status;
	mppa_power_base_waitpid(0, &status, 0);
	utask_join(rm1, NULL);

	pcie_queue_barrier(pcie_fd, 0, &status);

	if (__k1_spawn_type() == __MPPA_PCI_SPAWN) {
		pcie_queue_exit(pcie_fd, 0, &status);
	}
	return 0;
}
