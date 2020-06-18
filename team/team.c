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

//--------------------------------------------------------------------------
// Constantes
//--------------------------------------------------------------------------
const int RAFAGAS_CATCH = 1;


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

	suscribirse_a_colas(config, logger);

	//suscribirse_a(config, logger, "BROKER", SUBSCRIBE_CATCH_POKEMON);



	//terminar_programa(conexion, logger, config);

	exit(0);

}

//----------------------------------------------------------------------------------------------------------
// Crea las listas que se van a utilizar globalmente en el proceso
//----------------------------------------------------------------------------------------------------------
void inicializarGlobales(){
	objetivo_global = list_create();
	entrenadores_new = list_create();
	entrenadores_ready = list_create();
	entrenadores_blocked_sin_espera = list_create();
	entrenadores_blocked_espera = list_create();
}

//----------------------------------------------------------------------------------------------------------
// Funcion para obtener los entrenadores desde el archivo de configuracion
// Por cada uno:
//    1. Se crea un t_entrenador
//    2. Se crea un tcb y un hilo en donde ejecutaŕa el mismo
//    3. Se asigna un semaforo (inicializado y bloqueado) a ese hilo para que el proceso no ejecute.
//    4. Se agrega tcb a la lista de entrenadores en estado new
//----------------------------------------------------------------------------------------------------------
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

		t_entrenador_tcb* tcb_entrenador = malloc(sizeof(t_entrenador_tcb));
		pthread_mutex_t mutex;
		pthread_mutex_init(&mutex, NULL);
		pthread_mutex_lock(&mutex);

		pthread_t thread;
		pthread_create(&thread,NULL,(void*)hilo_entrenador, tcb_entrenador);
		pthread_detach(thread);

		tcb_entrenador->id_hilo_entrenador = thread;
		tcb_entrenador->entrenador = entrenador;
		tcb_entrenador->mutex = mutex;
		tcb_entrenador->nombre_pokemon = NULL;
		tcb_entrenador->rafagas = 0;

		list_add(entrenadores_new, tcb_entrenador);

		free(entrenador);

		i++;
	}

}

//----------------------------------------------------------------------------------------------------------
// Elimina un caracter de una cadena
//----------------------------------------------------------------------------------------------------------
void removeChar(char *str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

//----------------------------------------------------------------------------------------------------------
// Separa las propiedades del entrenador que vienen en listas
//----------------------------------------------------------------------------------------------------------
char** formatearPropiedadDelConfig(char* propiedad){
	removeChar(propiedad, '[');
	removeChar(propiedad, ']');

	return string_split(propiedad,",");
}

//----------------------------------------------------------------------------------------------------------
// Funcion para crear un entrenador con sus coordenadas, pokemones y objetivos
//----------------------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------------------
// A partir de una array de pokemones, devuelve una lista
//----------------------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------------------
// Funcion para la ejecucion de cada proceso entrenador
// Por cada ciclo, espera el semaforo del hilo. El planificador es el que realiza el signal
// Si no coincide la posicion del entrendaor con la del pokemon a capturar, se mueve.
//    - Primero de manera horizontal, luego vertical
//    - 1 movimiento por ciclo
// Si coincide, realiza el catch
//----------------------------------------------------------------------------------------------------------
void hilo_entrenador(t_entrenador_tcb* tcb_entrenador){

	while(1){

		pthread_mutex_lock(&(tcb_entrenador->mutex));

		if(coincidenCoordenadas(tcb_entrenador->coordenadas_del_pokemon, tcb_entrenador->entrenador->coordenadas)){
			mover_entrenador(&(tcb_entrenador->entrenador->coordenadas), tcb_entrenador->coordenadas_del_pokemon);
		} else {
//			realizar_catch();
		}
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

//----------------------------------------------------------------------------------------------------------
// Calcula la distancia entre dos coordenadas
// Se calcula el modulo de las distancias parciales
//----------------------------------------------------------------------------------------------------------
int distanciaEntreCoordenadas(t_coordenadas coordenada_A, t_coordenadas coordenada_B){
	return (fabs(coordenada_B.posx - coordenada_A.posx)) + (fabs(coordenada_B.posy - coordenada_A.posy));
}

//----------------------------------------------------------------------------------------------------------
// Busca el entrenador mas cercano al pokemon que aparecio
// Si encuentra alguno, le asigna la cantidad de rafagas a procesar
//----------------------------------------------------------------------------------------------------------
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

	if(tcb_entrenador != NULL){
		tcb_entrenador->rafagas = distancia + RAFAGAS_CATCH;
	}

	return tcb_entrenador;

}

//----------------------------------------------------------------------------------------------------------
// Funcion para liberar un t_entrenador_tcb
// Elimina las listas, el nombre del pokemon, el entrenador y el tcb en si mismo
//----------------------------------------------------------------------------------------------------------
void liberar_tcb(t_entrenador_tcb* tcb_entrenador){
	free(tcb_entrenador->nombre_pokemon);
	list_destroy_and_destroy_elements(tcb_entrenador->entrenador->objetivo_entrenador, free);
	list_destroy_and_destroy_elements(tcb_entrenador->entrenador->pokemones_entrenador, free);
	free(tcb_entrenador->entrenador);
	free(tcb_entrenador);
}

//----------------------------------------------------------------------------------------------------------
// Atiende la solicitud del broker
//   1. Evalua si es un pokemon requerido
//   2. Si lo es
//      2.1 Busca entrenador cercano (entrenadores en NEW o bloqueados sin hacer nada)
//      2.2 TODO Si no hay ninfuno, lo deberia dejar encolado
//      2.3 Elimina la lista parcial creada
//----------------------------------------------------------------------------------------------------------
void atenderSolicitud(char* pokemon, t_coordenadas coordenadas){
	bool esIgualAPokemon(void* elemento){
		return strcmp(pokemon, elemento);
	}

	if(list_any_satisfy(objetivo_pokemones_pendientes, esIgualAPokemon)){
		t_list* lista_disponibles;
		lista_disponibles = list_create();

		list_add_all(lista_disponibles, entrenadores_new);
		list_add_all(lista_disponibles, entrenadores_blocked_sin_espera);


		t_entrenador_tcb* tcb_entrenador = obtenerMasCercano(lista_disponibles, coordenadas);
		list_destroy_and_destroy_elements(lista_disponibles, liberar_tcb);
	}
}

//----------------------------------------------------------------------------------------------------------
// Carga el objetivo global del proceso
// Recorre todos los entrenadores cargados y carga sus objetivos en la lista global
// Una vez finalizado, copia la lista a la de pokemones pendientes, que se utilizará para actualizar
//----------------------------------------------------------------------------------------------------------
void cargarObjetivoGlobal(){

	t_link_element* elemento = entrenadores_new->head;

	while(elemento != NULL) {
		list_add_all(objetivo_global, ((t_entrenador_tcb*) elemento->data)->entrenador->objetivo_entrenador);
		elemento = elemento->next;
	}

	objetivo_pokemones_pendientes = list_duplicate(objetivo_global);

	free(elemento);

}

//----------------------------------------------------------------------------------------------------------
// Genera las conexiones con el broker para suscribirse a las colas de mensajes:
//    1. APPEARED_POKEMON
//    2. LOCALIZED_POKEMON
//    3. CAUGHT_POKEMON
// Cada conexion se maneja desde un hilo en particular
// Dentro se define la funcion para manejar cada hilo.
//----------------------------------------------------------------------------------------------------------
void suscribirse_a_colas(t_config* config, t_log* logger){

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	// Funcion que ejecuta dentro del hilo
  	void suscribirse_a(op_code queue_suscripcion){
		int conexion = 0;
		conexion = conectarse_a(config, logger, "BROKER");

		op_code tipo_mensaje;

		switch(queue_suscripcion){
			case SUBSCRIBE_APPEARED_POKEMON:
				tipo_mensaje = APPEARED_POKEMON;
				break;
			case SUBSCRIBE_LOCALIZED_POKEMON:
				tipo_mensaje = LOCALIZED_POKEMON;
				break;
			case SUBSCRIBE_CAUGHT_POKEMON:
				tipo_mensaje = CAUGHT_POKEMON;
				break;
		}

		recibir_con_semaforo(conexion, mutex, logger, tipo_mensaje);
	}

	pthread_create(&thread_appeared,NULL,(void*)suscribirse_a, SUBSCRIBE_APPEARED_POKEMON);
	pthread_create(&thread_localized,NULL,(void*)suscribirse_a, SUBSCRIBE_LOCALIZED_POKEMON);
	pthread_create(&thread_caught,NULL,(void*)suscribirse_a, SUBSCRIBE_CAUGHT_POKEMON);

}


//----------------------------------------------------------------------------------------------------------
// Abre conexion con el broker.
// Si no puede, invoca al proceso de reintento de comunicacion.
//----------------------------------------------------------------------------------------------------------
int conectarse_a(t_config* config, t_log* logger, char* proceso) {
	int socket;

	iniciar_conexion(&socket, config, logger, proceso);

	if(socket < 0){
		log_info(logger, "Inicio de proceso de reintento de comunicación con el Broker");
		reintentar_conexion(&socket, config, logger, proceso);
	}

	return socket;

}

//----------------------------------------------------------------------------------------------------------
// Proceso de reintento de comunicacion con Broker
// Obtiene la cantidad de segundos para volver a intentar desde el config
//----------------------------------------------------------------------------------------------------------
void reintentar_conexion(int* conexion, t_config* config, t_log* logger, char* proceso){
	int tiempo_reconexion;

	asignar_int_property(config, "TIEMPO_RECONEXION", &tiempo_reconexion);

	if(tiempo_reconexion == NULL){
		log_error(logger, "No existe la propiedad TIEMPO_RECONEXION");
		exit(-1);
	}

	while(*conexion < 0){
		sleep(tiempo_reconexion);
		log_info(logger, "Reintentando conexion con proceso %s", proceso);
		iniciar_conexion(&(*conexion), config, logger, proceso);
	}
}

void* recibir_con_semaforo(int socket_cliente, pthread_mutex_t mutex, t_log* logger, op_code tipo_mensaje){
		pthread_mutex_lock(&mutex);

		void* response = recibir_mensaje(socket_cliente, &mutex);

		switch(tipo_mensaje){
			case APPEARED_POKEMON:
				// TODO metodo para procesar appeared
				break;
			case LOCALIZED_POKEMON:
				// TODO metodo para procesar localized
				break;
			case CAUGHT_POKEMON:
				// TODO metodo para procesar caught
				break;
		}

		log_info(logger, "Se recibio un mensaje del tipo %s", tipo_mensaje);

		pthread_mutex_unlock(&mutex);
		return response;
}


//----------------------------------------------------------------------------------------------------------
// Evalua movimiento, primero coordenada x, luego y
//----------------------------------------------------------------------------------------------------------
void mover_entrenador(t_coordenadas* coordenadas_entrenador, t_coordenadas coordenadas_pokemon){
	if(coordenadas_entrenador->posx != coordenadas_pokemon.posx){
		mover_una_posicion(&(coordenadas_entrenador->posx), coordenadas_pokemon.posx);
	} else {
		mover_una_posicion(&(coordenadas_entrenador->posy), coordenadas_pokemon.posy);
	}
}

//----------------------------------------------------------------------------------------------------------
// Suma o resta 1 a la posicion para acercarse
//----------------------------------------------------------------------------------------------------------
void mover_una_posicion(int* coordenada_entrenador, int coordenada_pokemon){

	if(*coordenada_entrenador > coordenada_pokemon){
		*coordenada_entrenador -= 1;
	} else {
		*coordenada_entrenador += 1;
	}
}

//----------------------------------------------------------------------------------------------------------
// Chequea si el pokemon y el entrenador estan en la misma posicion
//----------------------------------------------------------------------------------------------------------
bool coincidenCoordenadas(t_coordenadas coordenadas_entrenador, t_coordenadas coordenadas_pokemon){
	return (coordenadas_entrenador.posx == coordenadas_pokemon.posx) && (coordenadas_entrenador.posy == coordenadas_pokemon.posy);
}
