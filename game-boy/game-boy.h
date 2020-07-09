#ifndef GAMEBOY_H_
#define GAMEBOY_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <commons/error.h>
#include <cliente.h>
#include "../utils/config.h"
#include<pthread.h>

void verificarEntrada(int argc, char *argv[]);

void iniciar_modo_suscriptor(int conexion_con_broker, char* cola_a_suscribirse, int tiempo_suscripcion);
void contador_tiempo_suscripcion(int segundos);

void despacharMensaje(int conexion, char *argv[]);

void enviarNew(int conexion, char *argv[]);
void enviarAppeared(int conexion, char *argv[]);
void enviarCatch(int conexion, char *argv[]);
void enviarCaught(int conexion, char *argv[]);
void enviarGet(int conexion, char *argv[]);
void enviarLocalized(int conexion, int argc, char *argv[]);

void terminar_programa(int, t_log*, t_config*);

#endif
