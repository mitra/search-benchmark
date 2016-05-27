#include <pcie.h>

/* Host application which will spawn through pcie the k1 binary on MPPA */ 
int
main(int argc, char **argv)
{

	/* open pcie device */
	mppadesc_t fd = pcie_open_device(0);
	if(!fd)
		return 1;


	/* load on the MPPA the k1 multi-binary */
	pcie_load_io_exec_args_mb(fd, argv[1], NULL, argv, argc, PCIE_LOAD_FULL);


	/* init the the pcie queue and register a console */
	pcie_queue_init(fd);
	pcie_register_console(fd, stdin, stdout);


	int status;
	pcie_queue_barrier(fd, 0, &status);


	/* wait on pcie for an exit message*/
	pcie_queue_exit(fd, 0, &status);
 
 	/* exit */
	return status;
}
