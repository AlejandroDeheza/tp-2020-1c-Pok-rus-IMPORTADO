#ifndef TEAM_H_
#define TEAM_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "../utils/cliente.h"
#include "../utils/config.h"

void terminar_programa(int, t_log*, t_config*);
void cargarEntrenadores(char *posicionesEntrenadores, char* pokemonesEntrenadores, char* objetivosEntrenadores, t_list* lista_entrenadores);
void planificarEntrenadores(t_list* lista_entrenadores, t_config* config, t_log* logger);
t_entrenador* crearEntrenador(char* coordenadas, char* objetivos, char* pokemones);
t_list* armarLista(char* objetos);

#endif
