#include "uteisClient.h"

//---------------------------------------------------------------------------------
//Configurações do sistema

void configurarCliente(int argc, char **argv, int* socketCliente){
    if (argc < 3) {
		usageCliente(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usageCliente(argc, argv);
	}

	*socketCliente = socket(storage.ss_family, SOCK_STREAM, 0);
	if (*socketCliente == -1) {
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(*socketCliente, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);
}

void usageCliente(int argc, char **argv) {
	printf("usage: %socketCliente <server IP> <server port>\n", argv[0]);
	printf("example: %socketCliente 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}