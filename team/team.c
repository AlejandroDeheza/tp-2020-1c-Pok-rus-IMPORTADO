#include "team.h"

int main(void)
{
	int conexion;
	char* ip;
	char* puerto;
	char* log_file;

	t_log* logger;
	t_config* config;

	config = leer_config();
	ip = config_get_string_value(config, "IP_BROKER");
	puerto = config_get_string_value(config, "PUERTO_BROKER");
	log_file = config_get_string_value(config, "LOG_FILE");
	printf("\nConfiguraciones:.\nIP = %s.\nPUERTO = %s\nLOG_FILE = %s.\n",ip, puerto, log_file);

	logger = iniciar_logger(log_file);
	log_info(logger, "Inicio el team");


	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	//antes de continuar, tenemos que asegurarnos que el servidor esté corriendo porque lo necesitaremos para lo que sigue.

	//crear conexion
	conexion = crear_conexion(ip, puerto);
	if(conexion==-1){
		printf("Hubo un error.\n");
	} else {
		printf("Conexión exitosa: %d.\n", conexion);
	}


	//enviar mensaje
	char* mensaje = malloc(32);
	printf("Ingrese su mensaje;.\n-> ");
	scanf("%s", mensaje);
	printf("Usted ingreso: \"%s\".\n\n", mensaje);

	enviar_mensaje(mensaje, conexion);


	//recibir mensaje
	char* mensajerecibido = recibir_mensaje(conexion);


	//loguear mensaje recibido
	log_info(logger, mensajerecibido);

	printf("asdadd\n");
	terminar_programa(conexion, logger, config);
	printf("memoria liberada.\n");

	exit(0);
}//main

//TODO
t_log* iniciar_logger(char* log_file)
{

	t_log* unLogger = log_create(log_file, "Team", true, LOG_LEVEL_INFO);

	return unLogger;
}

//TODO
t_config* leer_config(void)
{
	return config_create("team1.config");
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	printf("terminando \n");
	if (logger != NULL){
		log_destroy(logger);
		printf("Logger destruido.\n");
	}
	if (config != NULL) {
		config_destroy(config);
		printf("Config destruido");
	}
	if (conexion != 0) {
		liberar_conexion(conexion);
		printf("Conexion liberada.\n");
	}
}
