#ifndef TEAM_H_
#define TEAM_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<readline/readline.h>
#include<pthread.h>
#include "../utils/cliente.h"
#include "../utils/config.h"
#include <limits.h>
//#include "planificador.h"

//--------------------------------------------------------------------------
//Argumentos para planificacion
//--------------------------------------------------------------------------
typedef struct {
    t_config* config;
    t_log* logger;
} argumentos_planificador;


//--------------------------------------------------------------------------
// Hilos para suscripcion a broker
//--------------------------------------------------------------------------
pthread_t thread_appeared;
pthread_t thread_localized;
pthread_t thread_caught;

//--------------------------------------------------------------------------
// Estructuras globales
//    1. Una lista por cada estado de planificacion posible
//    2. Listas de pokemones recibidos
//--------------------------------------------------------------------------
t_list* objetivo_global;
t_list* objetivo_pokemones_pendientes;
t_list* entrenadores_new;
t_list* entrenadores_ready;
t_list* entrenadores_blocked_espera;
t_list* entrenadores_blocked_sin_espera;
t_list* pokemones_recibidos;

//--------------------------------------------------------------------------
// Metodos para iniciar proceso
//--------------------------------------------------------------------------
void inicializarGlobales();
void obtenerEntrenadores(t_config* config, t_log* logger);
t_entrenador* crearEntrenador(char* coordenadas, char* objetivos, char* pokemones);
void cargarObjetivoGlobal();

//--------------------------------------------------------------------------
// metodos "utils"
//--------------------------------------------------------------------------
char** formatearPropiedadDelConfig(char* propiedad);
t_list* armarLista(char** objetos);

//--------------------------------------------------------------------------
// metodos funcionales
//--------------------------------------------------------------------------
void hilo_entrenador();
void mover_una_posicion(int* coordenada_entrenador, int coordenada_pokemon);
void mover_entrenador(t_coordenadas* coordenadas_entrenador, t_coordenadas coordenadas_pokemon);
bool coincidenCoordenadas(t_coordenadas coordenadas_entrenador, t_coordenadas coordenadas_pokemon);
int distanciaEntreCoordenadas(t_coordenadas coordenada_A, t_coordenadas coordenada_B);
t_entrenador_tcb* obtenerMasCercano(t_list* entrenadores, t_coordenadas coordenadas_pokemon);
t_entrenador_tcb* buscarEntrenadorQueAplique(t_coordenadas coordenadas);
void terminar_programa(int, t_log*, t_config*);
void* recibir_con_semaforo(int socket_cliente, pthread_mutex_t mutex, t_log* logger, op_code op_code);
void liberar_tcb(t_entrenador_tcb* tcb_entrenador);
void suscribirse_a_colas(t_config* config, t_log* logger);

#endif
