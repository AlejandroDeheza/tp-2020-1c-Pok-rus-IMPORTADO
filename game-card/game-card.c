#include <stdio.h>
#include <stdlib.h>
#include "game-card.h"
#include <conexion.h>
#include "../utils/conexion.h"
#include "../utils/config.h"
#include "../utils/serializacion.h"

int main(void) {

	int conexion;
	char* ip;
	char* puerto;
	char* log_file;

	t_log* logger;
	t_config* config;

	config = leer_config("../game-card.config");

	asignar_string_property(config, "IP_BROKER", &ip);
	asignar_string_property(config, "PUERTO_BROKER", &puerto);
	asignar_string_property(config, "LOG_FILE", &log_file);

	logger = log_create(log_file, "game-card" , true, LOG_LEVEL_INFO);
	if(!log_file){
		log_file = "game-card.log";
	}

	if(!ip || !puerto){
		log_info(logger, "Chequear archivo de configuracion");
		exit(-1);
	}

	printf("\nConfiguraciones:.\nIP = %s.\nPUERTO = %s\nLOG_FILE = %s.\n",ip, puerto, log_file);

	conexion = crear_conexion( ip, puerto);

	char* nombre;
	int posx;
	int posy;
	int cantidad;
	printf("Ingrese nombre del pokemon:\n");
	scanf("%s", nombre);
	printf("Ingrese posicion X:\n");
	scanf("%d", &posx);
	printf("Ingrese posicion Y:\n");
	scanf("%d", &posy);
	printf("Ingrese cantidad:\n");
	scanf("%d", &cantidad);
	new_pokemon(nombre, posx, posy, cantidad, conexion);

	char* response = recibir_mensaje(conexion);
	printf("Menaje devuelto: %s", response);

	terminar_programa(conexion, logger, config);

	exit(0);

}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	if (logger != NULL){
		log_destroy(logger);
	}
	if (config != NULL) {
		config_destroy(config);
	}
	if (conexion != 0) {
		liberar_conexion(conexion);
	}
	printf("Finalizo programa.\n");
}
