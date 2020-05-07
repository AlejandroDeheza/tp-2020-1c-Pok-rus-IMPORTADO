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
#include "team.h"

#include <cliente.h>
#include "../utils/cliente.h"
#include "../utils/config.h"


int main(void) {

	int conexion;
	char* ip;
	char* puerto;
	char* log_file;

	t_log* logger;
	t_config* config;
	t_list* lista_entrenadores;

	config = leer_config("../team.config");

	/*****************Leo del config y cargo los entrenadores a una lista*****************/
	char* posicionesEntrenadores;
	asignar_string_property(config, "POSICIONES_ENTRENADORES", &posicionesEntrenadores);
	removeChar(posicionesEntrenadores, '[');
	removeChar(posicionesEntrenadores, ']');
	printf("Entrenadores quedo asi: %s\n", posicionesEntrenadores);

	char** posiciones = string_split(posicionesEntrenadores,",");

	lista_entrenadores = list_create();

	int i=0;
	while(posiciones[i]!=NULL){
		char** xy = string_split(posiciones[i],"|");
		int posicionX = atoi(xy[0]);
		int posicionY = atoi(xy[1]);
		printf("Elemento %d, PosicionX: %d \n", i, posicionX);
		printf("Elemento %d, PosicionY: %d \n", i, posicionY);
		t_entrenador* entrenador = malloc(sizeof(t_entrenador));
		entrenador->coordenadas.posx = posicionX;
		entrenador->coordenadas.posy = posicionY;
		list_add(lista_entrenadores, entrenador);
		i++;
	}

	/*****************Leo del config y cargo los entrenadores a una lista*****************/

	asignar_string_property(config, "IP_BROKER", &ip);
	asignar_string_property(config, "PUERTO_BROKER", &puerto);
	asignar_string_property(config, "LOG_FILE", &log_file);

	logger = log_create(log_file, "team" , true, LOG_LEVEL_INFO);
	if(!log_file){
		log_file = "team.log";
	}

	if(!ip || !puerto){
		log_info(logger, "Chequear archivo de configuracion");
		exit(-1);
	}

	printf("\nConfiguraciones:.\nIP = %s.\nPUERTO = %s\nLOG_FILE = %s.\n",ip, puerto, log_file);

	conexion = crear_conexion( ip, puerto);

	char* mensaje;
	printf("Ingrese un mensaje:\n");
	scanf("%s", mensaje);
	enviar_mensaje(mensaje, conexion, MENSAJE);

	recibir_mensaje(conexion);
	printf("Return\n");

    //log_info(logger, "Saliendo");

	terminar_programa(conexion, logger, config);

	exit(0);

}

void removeChar(char *str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
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
