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

	t_log* logger;
	t_config* config;

	config = leer_config("../team.config");

	inicializarGlobales();

	iniciar_logger(&logger, config, "team");


	/*****************Leo del config y cargo los entrenadores a una lista*****************/
	obtenerEntrenadores(config, logger);

	// carga los objetivos globales del team
	cargarObjetivoGlobal();

	/*****************Leo del config y cargo los entrenadores a una lista*****************/


	asociarseACola(APPEARED_POKEMON, config, logger);



	//terminar_programa(conexion, logger, config);

	exit(0);

}

void obtenerEntrenadores(t_config* config, t_log* logger){

	char* posicionesEntrenadores;
	char* pokemonesEntrenadores;
	char* objetivosEntrenadores;

	asignar_string_property(config, "POSICIONES_ENTRENADORES", &posicionesEntrenadores);
	asignar_string_property(config, "POKEMON_ENTRENADORES", &pokemonesEntrenadores);
	asignar_string_property(config, "OBJETIVOS_ENTRENADORES", &objetivosEntrenadores);

	char** coodenadasEntrenador = formatearPropiedadDelConfig(posicionesEntrenadores);
	char** pokemonEntrenador = formatearPropiedadDelConfig(pokemonesEntrenadores);
	char** objetivoEntrenador = formatearPropiedadDelConfig(objetivosEntrenadores);

	int i = 0;
	while(coodenadasEntrenador[i]!= NULL){
		t_entrenador* entrenador = crearEntrenador(coodenadasEntrenador[i], pokemonEntrenador[i], objetivoEntrenador[i]);

		pthread_t thread;
		pthread_create(&thread,NULL,(void*)hilo_entrenador,NULL);
		pthread_detach(thread);

		t_entrenador_tcb* tcb_entrenador = malloc(sizeof(t_entrenador_tcb));
		pthread_mutex_t mutex;
		pthread_mutex_init(&mutex, NULL)
		pthread_mutex_lock(&mutex);

		tcb_entrenador->id_hilo_entrenador = thread;
		tcb_entrenador->entrenador = entrenador;
		tcb_entrenador->mutex = mutex;

		list_add(entrenadores_new, tcb_entrenador);

		free(entrenador);

		i++;
	}

}

void removeChar(char *str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}


char** formatearPropiedadDelConfig(char* propiedad){
	removeChar(propiedad, '[');
	removeChar(propiedad, ']');

	return string_split(propiedad,",");
}


t_entrenador* crearEntrenador(char* coordenadas, char* objetivos, char* pokemones){
	t_entrenador* entrenador = malloc(sizeof(t_entrenador));

	char** xy = string_split(coordenadas,"|");

	entrenador->coordenadas.posx = atoi(xy[0]);
	entrenador->coordenadas.posy = atoi(xy[1]);

	t_list* lista_objetivos = armarLista(string_split(objetivos,"|"));
	t_list* lista_pokemones = armarLista(string_split(pokemones, "|"));

	entrenador->objetivo_entrenador = lista_objetivos;
	entrenador->pokemones_entrenador = lista_pokemones;

	return entrenador;
}

t_list* armarLista(char** objetos){

	t_list* lista = list_create();

	int i = 0;
	while(objetos[i] != NULL){

		char* objeto = malloc(strlen(objetos[i]) + 1);
		memcpy(objeto, objetos[i], strlen(objetos[i]) + 1);
		list_add(lista, objeto);
		i++;
	}


	free(objetos);

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

t_log* asignarLogger(t_config* config) {
	char* log_file;

	asignar_string_property(config, "LOG_FILE", &log_file);
	if(!log_file){
		log_file = "team.log";
	}

	t_log* log = log_create(log_file, "team" , true, LOG_LEVEL_INFO);
	log_info(log, "LOG FILE = %s \n", log_file);

	free(log_file);

	return log;
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
		list_add_all(objetivo_global, listaParcial);
		i++;
	}
}

void atenderSolicitud(char* pokemon, t_coordenadas coordenadas){
	bool esIgualAPokemon(void* elemento){
		return strcmp(pokemon, elemento);
	}

	if(list_any_satisfy(objetivo_pokemones_pendientes, esIgualAPokemon)){
		buscarEntrenadorQueAplique(coordenadas);
	}
}

t_entrenador_tcb* buscarEntrenadorQueAplique(t_coordenadas coordenadas){
	//Obtener mi lista de entrenadores para planificar
	//Los que estan en new + Los que estan bloqueados en espera activa

	t_list* listaDisponibles;

	return obtenerMasCercano(listaDisponibles, coordenadas);

}

void cambiarEstadoEntrenador(t_entrenador_tcb* entrenador, char* nuevoEstado){
	//entrenador->status = nuevoEstado;
}


void cargarObjetivoGlobal(){

	t_link_element* elemento = entrenadores_new->head;

	while(elemento != NULL) {
		list_add_all(objetivo_global, ((t_entrenador_tcb*) elemento->data)->entrenador->objetivo_entrenador);
		elemento = elemento->next;
	}

	objetivo_pokemones_pendientes = list_duplicate(objetivo_global);

	free(elemento);

}

void asociarseACola(op_code op_code, t_config* config, t_log* logger){
	int socket;

	iniciar_conexion(&socket, config, logger, "BROKER");

	if(socket < 0){
		log_info(logger, "Inicio de proceso de reintento de comunicación con el Broker");
		reintentar_conexion(&socket, config, logger, "BROKER");
	}

	while(1) {
		void* mensaje = recibir_mensaje(socket);

		//Aca planificamos y
	}
}

void moverPosisionArriba(t_coordena)
