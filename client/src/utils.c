#include "utils.h"

//convierte una estructura de datos en secuencia de bytes q se puede guardar en memoria o transmitir
// a traves de wifi
//util para guardar o transferir datos de un lugar a otro eficiente y estructurada

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes); // reserva memoria p/ buffer(magic) con cant bytes
	int desplazamiento = 0; // se usa para rastrear posicion actual en bufer mientras se copian
	//los datos del paquete

//copia el codigo de operacion de la estructura paquete al buffer (magic)de datos que se está serializando
	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int)); 	//se copia el tamaño de un entero en bytes
	// calcula dir de memoria donde se copia el valor "cod de operacion" en bufer serializado
	//magic es ptero a inicio de buffer y desplazamiento es cant bytes desde el inicio donde se hace la copia
	desplazamiento+= sizeof(int); //dsp de copiar, aumenta el despalzamiento x la cant de bytes copiados
	//apunte al prox lugar en bufer donde se copian los prox datos

//copia el valor de size del buffer  del paquete en el buffer serializado(magic)
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int)); 	//se copia el tamaño de un entero en bytes
	//magic+desplazamiento calcula dir de memoria donde se copia el valor size del buffer del paquete
	//magic es puntero al inicio del buffer y desplazamiento es cant bytes desde inicio donde se hace la copia
	desplazamiento+= sizeof(int);//dsp de copiar, aumenta el despalzamiento x la cant de bytes copiados
	//apunte al prox lugar en bufer donde se copian los prox datos


//copia los datos stream del buffer del paquete en el buffer magic serializado
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);//especifica cantidad de bytes que se copiarán a partir del tamño del buffer del paquete
	//magic+desplazamiento calcula dir de memoria donde se copia el valor size del buffer del paquete
	//magic es puntero al inicio del buffer y desplazamiento es cant bytes desde inicio donde se hace la copia
	desplazamiento+= paquete->buffer->size;//dsp de copiar, aumenta el despalzamiento x la cant de bytes copiados
	//apunte al prox lugar en bufer donde se copian los prox datos


	return magic; //me devuelve el buffer serializado con los datos copiados
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);
	//obtiene info sobre direcciones de red q coincidan con IP y puerto especificados
	//usa los criterios de HINT
	//lo que devuelve lo guarda en server_info.

	// Ahora vamos a crear el socket.

	int socket_cliente = socket(server_info->ai_family,
                    server_info->ai_socktype,
                    server_info->ai_protocol);

	// Conecto socket.
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
	//quien se quiere conectar
	//direccion del server al q me quiero conectar, puntero a estructura sockaddr que contiene dir del server al q me quiero conectar
	//tamaño de la estructura sockaddr, garantiza que recibe cantidad correcta de bytes de la direccion

	freeaddrinfo(server_info); //libera memoria asignada para lista de estructuras addrinfo guardada en server info
	//como la conexion se mantiene con el socket cliente y la info de direccion obtenida ya esta en uso --> libero todo
	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
    t_paquete* paquete = malloc(sizeof(t_paquete)); //puntero paquete a estructura paquete
	// asigno memoria dinamica a partir del tamaño del paquete

    paquete->codigo_operacion = MENSAJE; //defino qué tipo de paquete envio.
    paquete->buffer = malloc(sizeof(t_buffer)); //almacena dir de memoria asignada en buffer de paquete
	// para objeto buffer dentro de paquete le asigno memoria a partir del tamaño de estructura buffer (stream y size incluidos)
    
	//Dentro de paquete, dentro de buffer defino size y stream.
	paquete->buffer->size = strlen(mensaje) + 1; // asigna el size dentro del buffer del paquete basado en tmño mensaje +1
	//asegura que buffer tenga espacio para guardar cadena completa y caracter de finalizacion
    paquete->buffer->stream = malloc(paquete->buffer->size); //al stream del buffer le asigna el tamaño de memoria a partir del tamaño que se definió antes
   
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);
	// se copia el mensaje al atributo stream del buffer quie esta dentro del stream
	//lo hace con el maximo de size del buffer del paquete.

    int bytes = paquete->buffer->size + 2*sizeof(int); // calcula nro bytes q se necesitan para guardar
	//contenido del buffer stream y algun adicional y se guarda en bytes.
	void* a_enviar = serializar_paquete(paquete, bytes); // serializo el paquete creado a partir de bytes que necesito

    send(socket_cliente, a_enviar, bytes, 0);
	//socket cliente envía lo serializado con cant bytes que se envian
    free(a_enviar); //vacia lo que se envia
    eliminar_paquete(paquete); // elimina el paquete porque ya lo envie.
}


void crear_buffer(t_paquete* paquete) //recibe paquete en donde creo/instancio buffer
{
	paquete->buffer = malloc(sizeof(t_buffer)); // asigno memoria al buffer a partir de lo que ocupa
	//un objeto buffer
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete)); //asigno a paquete el tamaño que puede tomar
	paquete->codigo_operacion = PAQUETE;// le digo que es un paquete
	crear_buffer(paquete); // creo el buffer que el paquete contiene.
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));
	//intenta redimensionar el buffer de datos (stream) dentro de t_buffer p/ meter mas datos,
	//paquete->buffer->stream lo q se quiere redimensionar a partir de 2do parametro
	//paquete->buffer->size + tamanio + sizeof(int):nuevo tamaño se calcula size actual buffer +tamanio+ tamaño int
	
	//copia bytes q representar valor de 'tamanio' al final de stream --> para incluir tamaño datos adicionales q se agregan
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	//paquete->buffer->stream + paquete->buffer->size: posicion dsp del ult byte de datos guardado en bufer stream
	//se copia contenido de 'tamanio' al final de stream
	
	
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);
	//puntero a final del bufer de datos(stream)+1 entero, posicion dsp del ult byte de datos
	//guardado en el stream(buffer) y al sumar tamaño int, nos movemos despues del espacio reservado
	//para tamaño de los datos
	//copio el contenido "valor" al final de stream del buffer dentro del paquete 
	//tamanio --> cant bytes q se copian
	paquete->buffer->size += tamanio + sizeof(int); 
	//aumenta size del buffer dentro de t_buffer dentro de t_paquete p/
	//tener en cuenta los datos nuevos agregados y el espacio adicional p/ guardar info de tamaño de los datos
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int); // calcula nro bytes q se necesitan para enviar
	//contenido del buffer stream y algun adicional y se guarda en bytes.

	void* a_enviar = serializar_paquete(paquete, bytes);
	//lo q envio lo tengo que serializar
	//envio lo serializado
	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar); //libero el a enviar ya que ya se envio.
}

void eliminar_paquete(t_paquete* paquete)
{ //libero todas las estructuras del paquete y el paquete en sí.
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{ //cierro conexion
	close(socket_cliente);

}
