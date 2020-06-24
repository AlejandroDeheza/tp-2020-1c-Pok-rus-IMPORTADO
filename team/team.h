#ifndef TEAM_H_
#define TEAM_H_

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<readline/readline.h>
#include<pthread.h>
#include "../utils/cliente.h"
#include "../utils/config.h"
#include <limits.h>
#include "planificador.h"

//--------------------------------------------------------------------------
// Sockets de conexion con broker
//--------------------------------------------------------------------------
int socket_appeared;
int socket_localized;
int socket_caught;
int socket_servidor_gameboy;
int socket_cliente_gameboy;

//--------------------------------------------------------------------------
// Estados de la conexion con el Broker
//--------------------------------------------------------------------------
typedef enum {
	CONECTADO,
	DESCONECTADO,
	NULO
} estado_conexion;

estado_conexion conexion_con_broker;
estado_conexion conexion_con_gameboy;

//--------------------------------------------------------------------------
// Semaforos
//--------------------------------------------------------------------------
pthread_mutex_t mutex_entrenadores_ready;
pthread_mutex_t mutex_hilos;

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
t_list* entrenadores_exec;
t_list* pokemones_recibidos;

//--------------------------------------------------------------------------
// Metodos para iniciar proceso
//--------------------------------------------------------------------------
void inicializarGlobales();
void conexion_inicial();
void obtenerEntrenadores();
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
bool coincidenCoordenadas(t_coordenadas coordenadas_entrenador, t_coordenadas coordenadas_pokemon);
void mover_una_posicion(int* coordenada_entrenador, int coordenada_pokemon);
void mover_entrenador(t_coordenadas* coordenadas_entrenador, t_coordenadas coordenadas_pokemon);

int distanciaEntreCoordenadas(t_coordenadas coordenada_A, t_coordenadas coordenada_B);
void atender_solicitud_appeared(char* pokemon, t_coordenadas coordenadas);
t_entrenador_tcb* obtener_entrenador_mas_cercano(t_list* entrenadores, t_coordenadas coordenadas_pokemon);
void liberar_tcb(t_entrenador_tcb* tcb_entrenador);
void realizar_catch(int conexion, char* pokemon, t_coordenadas coordenadas);

int conectarse_a(char* proceso);
void enviar_get_pokemones_requeridos();
void reintentar_conexion(int* conexion, char* proceso);
void suscribirse_a_colas();
void suscribirse_a(char *nombre_proceso, op_code nombre_cola);
void recibir_con_semaforo(int socket_cliente, pthread_mutex_t mutex, op_code op_code);
void escuchar_conexion();
void terminar_proceso();

#endif
