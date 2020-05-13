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


int main(void) {

	int conexion;
	char* ip;
	char* puerto;
	char* log_file;

	t_log* logger;
	t_config* config;
	t_list* lista_entrenadores;

	config = leer_config("../team.config");

	asignar_string_property(config, "LOG_FILE", &log_file);
	if(!log_file){
		log_file = "team.log";
	}

	logger = log_create(log_file, "team" , true, LOG_LEVEL_INFO);
	lista_entrenadores = list_create();


	/*****************Leo del config y cargo los entrenadores a una lista*****************/
	planificarEntrenadores(lista_entrenadores, config, logger);


	/*****************Leo del config y cargo los entrenadores a una lista*****************/
	asignar_string_property(config, "IP_BROKER", &ip);
	asignar_string_property(config, "PUERTO_BROKER", &puerto);

	if(!ip || !puerto){
		log_info(logger, "Chequear archivo de configuracion");
		exit(-1);
	}

	printf("\nConfiguraciones:.\nIP = %s.\nPUERTO = %s\nLOG_FILE = %s.\n",ip, puerto, log_file);

	conexion = crear_conexion( ip, puerto);

	proyecto nombre_proyecto = TEAM;
	enviar_mensaje(&nombre_proyecto, conexion, IDENTIFICACION);

	proyecto *respuesta = (proyecto*) recibir_mensaje(conexion);

	printf("la respuesta es %d\n", *respuesta);
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

void planificarEntrenadores(t_list* lista_entrenadores, t_config* config, t_log* logger){
	char* posicionesEntrenadores;
	char* pokemonesEntrenadores;
	char* objetivosEntrenadores;
	asignar_string_property(config, "POSICIONES_ENTRENADORES", &posicionesEntrenadores);
	asignar_string_property(config, "POKEMON_ENTRENADORES", &pokemonesEntrenadores);
	asignar_string_property(config, "OBJETIVOS_ENTRENADORES", &objetivosEntrenadores);

	if((posicionesEntrenadores != NULL) && (pokemonesEntrenadores != NULL) && (objetivosEntrenadores != NULL)){
		cargarEntrenadores(posicionesEntrenadores, pokemonesEntrenadores, objetivosEntrenadores, lista_entrenadores);
	}

	//aca planificaria

}

void cargarEntrenadores(char *posicionesEntrenadores, char* pokemonesEntrenadores, char* objetivosEntrenadores, t_list* lista_entrenadores){

	// elimino corchetes del "vector"
	removeChar(posicionesEntrenadores, '[');
	removeChar(posicionesEntrenadores, ']');
	removeChar(pokemonesEntrenadores, '[');
	removeChar(pokemonesEntrenadores, ']');
	removeChar(objetivosEntrenadores, '[');
	removeChar(objetivosEntrenadores, ']');

	printf("Entrenadores quedo asi: %s\n", posicionesEntrenadores);

	// separa las posiciones de cada entrenados
	char** coodenadasEntrenador = string_split(posicionesEntrenadores,",");
	char** pokemonEntrenador = string_split(pokemonesEntrenadores, ",");
	char** objetivoEntrenador = string_split(objetivosEntrenadores, ",");

	int i = 0;
	while(coodenadasEntrenador[i]!= NULL){
		t_entrenador* entrenador = crearEntrenador(coodenadasEntrenador[i], pokemonEntrenador[i], objetivoEntrenador[i]);
		list_add(lista_entrenadores, entrenador);
		i++;
	}
}

t_entrenador* crearEntrenador(char* coordenadas, char* objetivos, char* pokemones){
	t_entrenador* entrenador = malloc(sizeof(t_entrenador));

	char** xy = string_split(coordenadas,"|");

	entrenador->coordenadas.posx = atoi(xy[0]);
	entrenador->coordenadas.posy = atoi(xy[1]);

	printf("******************\n");
	printf("Coordenada x: %d\n", atoi(xy[0]));
	printf("Coordenada y: %d\n", atoi(xy[1]));
	printf("-------------------\n");

	printf("Objetivos: \n");
	t_list* lista_objetivos = armarLista(objetivos);
	printf("-------------------\n");
	printf("Pokemones: \n");
	t_list* lista_pokemones = armarLista(pokemones);
	printf("******************\n");

	entrenador->objetivo_entrenador = lista_objetivos;
	entrenador->pokemones_entrenador = lista_pokemones;

	return entrenador;
}

t_list* armarLista(char* objetos){
	t_list* lista;
	lista = list_create();
	char** vector_objetos = string_split(objetos,"|");
	int i = 0;
	while(vector_objetos[i] != NULL){

		char* objeto = malloc(strlen(vector_objetos[i]) + 1);
		memcpy(objeto, vector_objetos[i], strlen(vector_objetos[i]) + 1);
		printf("%s\n", objeto);
		list_add(lista, objeto);
		i++;
	}
	return lista;
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
