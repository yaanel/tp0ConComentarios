#include "server.h"

int main(void) {

	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	int server_fd = iniciar_servidor();
	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd);

	t_list* lista; // puntero a lista
	while (1) { // hasta que se desconecte cliente u ocurra error
		int cod_op = recibir_operacion(cliente_fd); // se recibe una operacion
		switch (cod_op) {
		case MENSAJE: // si es un mensaje --> lo recibe
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd); // en la lista se guarda el paquete
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator); //recorre todos los elementos de la lista y les aplica 
			//iterator--> cada valor de la lista se registra en log.
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;
}

void iterator(char* value) { // registra en el log c/ elemento de una lista cuando es invocada
	log_info(logger,"%s", value);
}
