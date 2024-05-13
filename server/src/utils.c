#include"utils.h"

t_log* logger;

// Funcion en la que inicio servidor
int iniciar_servidor(void)
{
	int socket_servidor; // id del servidor --> el q escucha

	struct addrinfo hints, *servinfo, *p;
	// hints me dice criterios de busqueda de direcciones de ip
	//servinfo guarda listas de estructuras addrinfo generadas al hacer getaddrinfo()
	//p puntero para iterar sobre esas estructuras generadas al hacer getaddrinfo
	
	memset(&hints, 0, sizeof(hints)); // memset llena estructura hints con ceros --> estructura inicializa
	//hints establece criterios para q getaddrinfo() busque dir de socket
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; // socket de flujo TCP--> Conexiones confiables, transf datos sin perdida ni duplicacion
	hints.ai_flags = AI_PASSIVE; //desea usar dir IP local para enlazar socket

	
	getaddrinfo(NULL, PUERTO, &hints, &servinfo);
	//NULL=sistena deternuba nombre de host local
	//PUERTO representa nro de puerto q busca
	//hints que son las configuraciones q puse
	//servinfo es en donde meto la informacion sobre las direcciones q se encuentren

// Crea socket a partir de la info de getaddrinfo.
	socket_servidor = socket(servinfo->ai_family, //tipo de direccion del socketm se obtiene de addrinfo, llenada por getaddrinfo()
                         servinfo->ai_socktype, //tipo de socket (TCP)
                         servinfo->ai_protocol);//protocolo q usa.
						 
// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	//puntero a estructura sockaddr qye tiene dir a la que se asocia el socket, esto lo sabe como resultado de getaddrinfo()
	//tanmaño de direccion guardada en servinfo->ai_addr
	
// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo); // cuando ya creé el socket, asocié una direccion y lo configuro para escuchar --> ya no es necesaria
	log_trace(logger, "Listo para escuchar a mi cliente");
	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	// Aceptamos un nuevo cliente
	int socket_cliente= accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
} // se conecta al servidor y me devuelve al cliente así puede ser usado para comunicarse.

// recibe codigo de operacion enviado y devuelve ese codigo
int recibir_operacion(int socket_cliente)
{
	int cod_op;
	//recibo codigo de operacion del cliente
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else //tiro error si recibi otra cosa y te cierro el socket (conexion) del cliente
	{
		close(socket_cliente);
		return -1;
	}
}

// Recibe buffer de datos enviado x cliente a traves de socket y devuelve puntero a ese buffer
//size puntero a entero q idnica tamaño buffer recibido
void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;// ptero generico p/ guardar el buffer de datos recibido
	
	//recibe tamaño del buffer de datos del cliente a traves del socket especificado, tamaño guardado en size
	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	//de donde quiero recibir datos
	//size guarda tamaño de datos q se reciben
	//sizeof(int) tamaño en bytes quue se espera recibir
	//MSG_WAITALL debe esperar a recibir todos los datos antes de devolver

	buffer = malloc(*size); // asigna memoria dinámica p/ buffer de datos basado en tamaño recibido del cliente
	
	// recibe  datos del cliente y los guarda en buffer
	recv(socket_cliente, buffer, *size, MSG_WAITALL);
	//de donde recibo los datos
	//buffer --> ptero al buffer donde se guarda lo recibido
	//size tamaño en bytes q se esperan recibir

	return buffer; //devuelve ptero al buffer de datos recibido
}

void recibir_mensaje(int socket_cliente)
{
	int size; // almacena tamaño msje recibido
	char* buffer = recibir_buffer(&size, socket_cliente);
	//puntero a buffer de caracteres q guarda msj recibido
	//recibe msje completo del cliente, guarda el tamaño en size
	//y devuelve un puntero a al buffer de caracteres q contiene el msje
	log_info(logger, "Me llego el mensaje %s", buffer);
	//avisa q llego

	free(buffer); // libera memoria asignada al bufer cuando se registra msje
// el msje puede estar en memoria pero NO se puede acceder mediante el puntero original
}

//recibe paquete de datos enviado x cliente a traves de socket
t_list* recibir_paquete(int socket_cliente) // de donde espero recibir
{
	int size; // guarda tamaño total del cliente
	int desplazamiento = 0; // rastrea posicion actual en buffer mientras se procesa paquete
	void * buffer; //almacena paquete completo recibido del cliente
	t_list* valores = list_create(); //lista enlazada q guarda valores recibidos
	int tamanio; // guarda tamaño de cada valor individual dentro del paquete

	//recibe paquete COMPLETO y devuelve puntero al buffer de datos y
	//almacena tamaño del paquete en size
	buffer = recibir_buffer(&size, socket_cliente);
	
	//bucle para procesar c/ valor dentro del paquete
	//mientras el desplazamiento sea menor q el tamaño del arhivo
	while(desplazamiento < size)
	{	
		//obtiene tamaño del prox valor del paquete
		//memcpy copia tamaño del prox valor 'tamanio' desde el buffer al espacio de memoria tamanio
		//copia 'sizeof(int)' bytes desde posicion acutal en buffer (buffer + desplazamiento) a la direccion de memoria de 'tamanio'
		//tamanio es int que almacena tamaño del prox valor paquete
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int); //incrementa desplazamiento con el tamaño de un int p/ avanzar a prox dato en buffer

		//reserva memoria para guardar el proximo valor en el paquete
		char* valor = malloc(tamanio); 

		//copia el valor desde el buffer al espacio de memoria reservado
		memcpy(valor, buffer+desplazamiento, tamanio); // copia 'tamanio' bytes desde posicion actual en el buffer (buffer+ desplazamiento)
		//al espacio de memoria reservado para valor
		desplazamiento+=tamanio; // incrementa el desplazamiento con tamanio para avanzar a prox dato en buffer

		//agrega el valor a la lista de valores
		list_add(valores, valor);
	}
	free(buffer); // libera memoria asignada para el buffer
	return valores; // devuelve lista de valores
}
