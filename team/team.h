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

// estructuras globales;
t_list* objetivo_global;
t_list* objetivo_pokemones_pendientes;
t_list* entrenadores_new;
t_list* entrenadores_ready;
t_list* entrenadores_blocked;

// metodos para iniciar proceso
t_log* asignarLogger(t_config* config);
void obtenerEntrenadores(t_config* config, t_log* logger);
t_entrenador* crearEntrenador(char* coordenadas, char* objetivos, char* pokemones);
void cargarObjetivoGlobal();

// metodos "utils"
char** formatearPropiedadDelConfig(char* propiedad);
t_list* armarLista(char** objetos);

// metodos funcionales
void hilo_entrenador();
int distanciaEntreCoordenadas(t_coordenadas coordenada_A, t_coordenadas coordenada_B);
t_entrenador_tcb* obtenerMasCercano(t_list* entrenadores, t_coordenadas coordenadas_pokemon);
t_entrenador_tcb* buscarEntrenadorQueAplique(t_coordenadas coordenadas);
void terminar_programa(int, t_log*, t_config*);


#endif
