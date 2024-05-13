#include "client.h"
int main(void)
{
	
	int conexion; // id de la conexion
	char* ip; // puntero a IP, p/ guardar direccion IP como cadena de char en memoria
	char* puerto; // puntero a puerto, p/ guardar nro de puerto como cadena char en memoria
	char* valor; // puntero a valor, para guardar un valor generico como cadena char en memoria

	t_log* logger; // logger es puntero q puede apuntar a instancia t_log.
	t_config* config; //config es puntero q apunta a instancia de estructura t_congig--> tiene info para el programa

	/* ---------------- LOGGING ---------------- */

	logger = iniciar_logger();
	log_info(logger,"Soy un Log");


	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

	config = iniciar_config(); // si no tengo un archivo de configuracion me tira error
	if (config == NULL) {
    exit(1);
};

	// Usando el config creado previamente, leemos los valores del config y los 
	// dejamos en las variables 'ip', 'puerto' y 'valor'
	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");
	valor = config_get_string_value(config, "CLAVE");

	/* ---------------- LEER DE CONSOLA ---------------- */

	leer_consola(logger);

	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	// ADVERTENCIA: Antes de continuar, tenemos que asegurarnos que el servidor esté corriendo para poder conectarnos a él

	// Creamos una conexión hacia el servidor
	conexion = crear_conexion(ip, puerto);

	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_mensaje(valor,conexion);
	// Armamos y enviamos el paquete
	paquete(conexion);

	terminar_programa(conexion, logger, config);

	/*---------------------------------------------------PARTE 5-------------------------------------------------------------*/
	// Proximamente
}

t_log* iniciar_logger(void)
{	//puntero a t_log
	//log_create crea un nuevo log--> tp0 nombre del archivo
	// nombre del programa asociado con el logger
	// true: se deben mostrar registros en consola y guardarlos
	//log_level_info: se registran los msj de info y de niveles + severos
	t_log* nuevo_logger = log_create("tp0.log","YANEL",true, LOG_LEVEL_INFO);
	if (nuevo_logger == NULL){ //si no se crea bien el LOG
		printf("No se pudo crear el LOG\n");
		exit(1);
	}
	else{
		return nuevo_logger; //devuelve el puntero nuevo_logger
	}
}

t_config* iniciar_config(void) // inicializa instancia de configuracion a partir de cliente.config
{
	t_config* nuevo_config; // puntero a t_config
	nuevo_config = config_create("cliente.config"); // crea una instancia de configuracion a partir
	//de los datos del cliente.config
	if(nuevo_config == NULL){ //si devuelve NULL (No se pudo crear la config, no existe o está mal) tira error
		printf("No se pudo crear el config");
		exit(1);
	}else return nuevo_config; // devuelve puntero nuevo_config
}

void leer_consola(t_log* logger) // lee entradas de texto metidas x user desde consola
{								//las registra como msj de informacion en el logger proporcionado
	char* leido; // almacena entrada de texto leída desde la consola

	// La primera te la dejo de yapa
	leido = readline("> "); //lee 1 linea de texto desde consola y lo guarda en leido.
	while (leido[0]!='\0'){ // se ejecuta hasta mientras primer caracter no sea nulo
		log_info(logger,"%s",leido);
		leido = readline("> ");
	};

	free(leido);
}

void paquete(int conexion){

	char* leido;
	t_paquete* paquete = crear_paquete();
	// Leemos y esta vez agregamos las lineas al paquete
	leido = readline("> ");
	while (leido[0]!='\0'){
		agregar_a_paquete(paquete, leido, strlen(leido) + 1);
		leido = readline("> ");
	};
	enviar_paquete(paquete,conexion);
	free(leido);
	eliminar_paquete(paquete);	//libero todas las estructuras del paquete y el paquete en si.
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{	
	config_destroy(config); //destruyo la configuracion
	log_destroy(logger); //destruyo el logger
	liberar_conexion(conexion); //cierro la conexion
}
