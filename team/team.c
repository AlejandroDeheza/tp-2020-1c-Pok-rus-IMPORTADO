/*
 ============================================================================
 Name        : team.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "../utils/utils.h"
#include "team.h"

int main(void) {

	int conexion;
	char* ip;
	char* puerto;
	char* log_file;

	t_log* logger;
	t_config* config;

	config = leer_config("team.config");
	ip = config_get_string_value(config, "IP_BROKER");
	puerto = config_get_string_value(config, "PUERTO_BROKER");
	log_file = config_get_string_value(config, "LOG_FILE");
	logger = iniciar_logger(log_file);
	printf("\nConfiguraciones:.\nIP = %s.\nPUERTO = %s\nLOG_FILE = %s.\n",ip, puerto, log_file);
	log_info(logger, "Saliendo");

	terminar_programa(conexion, logger, config);

	exit(0);

}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	printf("terminando \n");
	if (logger != NULL){
		log_destroy(logger);
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
