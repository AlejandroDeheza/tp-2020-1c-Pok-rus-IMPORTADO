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


void inicializarGlobales(){
	objetivo_global = list_create();
	entrenadores_new = list_create();
	entrenadores_ready = list_create();
	entrenadores_blocked = list_create();
}

int main(void) {

	int conexion;
	char* ip;
	char* puerto;
	char* log_file;

	t_log* logger;
	t_config* config;

	// obtiene la configuracion desde el archivo team.config
	config = leer_config("../team.config");

	// crea listas y queues globales
	inicializarGlobales();

	// asigna el logger al archivo team.log
	logger = asignarLogger(config, &log_file);

	// asigna propiedades de conexion
	asignar_string_property(config, "IP_BROKER", &ip);
	asignar_string_property(config, "PUERTO_BROKER", &puerto);

	if(!ip || !puerto){
		log_info(logger, "Chequear archivo de configuracion");
		exit(-1);
	}

	printf("\nConfiguraciones:.\nIP = %s.\nPUERTO = %s\nLOG_FILE = %s.\n",ip, puerto, log_file);

	/*****************Leo del config y cargo los entrenadores a una lista*****************/
	obtenerEntrenadores(config, logger);

	// muestra info de los entrenadores
	t_link_element* elemento = entrenadores_new->head;

	while(elemento != NULL) {
		printf("Id %lu\n", ((t_entrenador_tcb*) elemento->data)->id_hilo_entrenador);
		printf("Pokemon %s\n", ((t_entrenador_tcb*) elemento->data)->entrenador->pokemones_entrenador->head->data);
		printf("Status %d\n", ((t_entrenador_tcb*) elemento->data)->status);
		elemento = elemento->next;
	}

	/*****************Leo del config y cargo los entrenadores a una lista*****************/

/*
	conexion = crear_conexion( ip, puerto);

	proyecto nombre_proyecto = TEAM;

	enviar_mensaje(&nombre_proyecto, conexion, IDENTIFICACION);

	proyecto *respuesta = (proyecto*) recibir_mensaje(conexion);

	printf("la respuesta es %d\n", *respuesta);
	printf("Return\n");
*/
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

void obtenerEntrenadores(t_config* config, t_log* logger){

	char* posicionesEntrenadores;
	char* pokemonesEntrenadores;
	char* objetivosEntrenadores;
	asignar_string_property(config, "POSICIONES_ENTRENADORES", &posicionesEntrenadores);
	asignar_string_property(config, "POKEMON_ENTRENADORES", &pokemonesEntrenadores);
	asignar_string_property(config, "OBJETIVOS_ENTRENADORES", &objetivosEntrenadores);

	calcularObjetivoGlobal(config, objetivosEntrenadores);

	if((posicionesEntrenadores != NULL) && (pokemonesEntrenadores != NULL) && (objetivosEntrenadores != NULL)){
		cargarEntrenadores(posicionesEntrenadores, pokemonesEntrenadores, objetivosEntrenadores);
	}
}

void cargarEntrenadores(char *posicionesEntrenadores, char* pokemonesEntrenadores, char* objetivosEntrenadores) {
	// elimino corchetes del "vector"
	removeChar(posicionesEntrenadores, '[');
	removeChar(posicionesEntrenadores, ']');
	removeChar(pokemonesEntrenadores, '[');
	removeChar(pokemonesEntrenadores, ']');
	removeChar(objetivosEntrenadores, '[');
	removeChar(objetivosEntrenadores, ']');

	// separa las posiciones de cada entrenados
	char** coodenadasEntrenador = string_split(posicionesEntrenadores,",");
	char** pokemonEntrenador = string_split(pokemonesEntrenadores, ",");
	char** objetivoEntrenador = string_split(objetivosEntrenadores, ",");

	int i = 0;
	while(coodenadasEntrenador[i]!= NULL){
		t_entrenador* entrenador = crearEntrenador(coodenadasEntrenador[i], pokemonEntrenador[i], objetivoEntrenador[i]);
		pthread_t thread;
		pthread_create(&thread,NULL,(void*)hilo_entrenador,NULL);
		pthread_detach(thread);
		t_entrenador_tcb* tcb_entrenador = malloc(sizeof(t_entrenador_tcb));
		tcb_entrenador->id_hilo_entrenador = thread;
		tcb_entrenador->entrenador = entrenador;
		tcb_entrenador->status = NEW;
		list_add(entrenadores_new, tcb_entrenador);
		i++;
	}
}

t_entrenador* crearEntrenador(char* coordenadas, char* objetivos, char* pokemones){
	t_entrenador* entrenador = malloc(sizeof(t_entrenador));

	char** xy = string_split(coordenadas,"|");

	entrenador->coordenadas.posx = atoi(xy[0]);
	entrenador->coordenadas.posy = atoi(xy[1]);

	t_list* lista_objetivos = armarLista(objetivos);
	t_list* lista_pokemones = armarLista(pokemones);

	entrenador->objetivo_entrenador = lista_objetivos;
	entrenador->pokemones_entrenador = lista_pokemones;

	return entrenador;
}

t_list* armarLista(char* objetos){
	t_list* lista = list_create();
	char** vector_objetos = string_split(objetos,"|");
	int i = 0;
	while(vector_objetos[i] != NULL){

		char* objeto = malloc(strlen(vector_objetos[i]) + 1);
		memcpy(objeto, vector_objetos[i], strlen(vector_objetos[i]) + 1);
		list_add(lista, objeto);
		i++;
	}
	return lista;
}

void hilo_entrenador(){
	sleep(10);
	while(1){

	}
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

t_log* asignarLogger(t_config* config, char* log_file) {
	asignar_string_property(config, "LOG_FILE", &log_file);
	if(!log_file){
		log_file = "team.log";
	}

	return log_create(log_file, "team" , true, LOG_LEVEL_INFO);
}

int distanciaEntreCoordenadas(t_coordenadas coordenada_A, t_coordenadas coordenada_B){
	return fabs(coordenada_B.posx - coordenada_A.posx) + fabs(coordenada_B.posy - coordenada_A.posy);
}

t_entrenador_tcb* obtenerMasCercano(t_list* entrenadores, t_coordenadas coordenadas_pokemon){

	int distancia = INT_MAX;

	t_entrenador_tcb* tcb_entrenador = NULL;
	t_link_element* elemento = entrenadores->head;

	while(elemento != NULL) {
		t_entrenador_tcb* tcb_actual = (t_entrenador_tcb*) elemento->data;
		int distancia_actual = distanciaEntreCoordenadas(tcb_actual->entrenador->coordenadas, coordenadas_pokemon);
		if(distancia_actual < distancia){
			tcb_entrenador = tcb_actual;
			distancia = distancia_actual;
		}
		elemento = elemento->next;
	}

	return tcb_entrenador;

}

void calcularObjetivoGlobal(t_config* config, char* objetivosEntrenadores){
	removeChar(objetivosEntrenadores, '[');
	removeChar(objetivosEntrenadores, ']');
	char** objetivoEntrenador = string_split(objetivosEntrenadores, ",");


	int i = 0;
	while(objetivoEntrenador[i]!= NULL){
		t_list* listaParcial = armarLista(objetivoEntrenador[i]);
		list_add(objetivo_global, listaParcial);
		i++;
	}
}
