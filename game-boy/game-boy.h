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

typedef struct
{
	int tiempo_suscripcion;
	int conexion_con_broker;
	op_code codigo_desuscripcion;
} argumentos_contador_de_tiempo;

void verificarEntrada(int argc, char *argv[]);

void obtener_codigos(char* cola_a_suscribirse, op_code* codigo_suscripcion, op_code* codigo_desuscripcion);
void iniciar_hilo_para_desuscripcion(int tiempo_suscripcion, int conexion_con_broker, op_code codigo_desuscripcion);
void* contador_de_tiempo(void* argumentos);
void imprimir_mensajes_recibidos(op_code codigo_operacion, int conexion_con_broker);

void imprimir_new_pokemon(void* mensaje_a_imprimir);
void imprimir_appeared_pokemon(void* mensaje_a_imprimir);
void imprimir_catch_pokemon(void* mensaje_a_imprimir);
void imprimir_caught_pokemon(void* mensaje_a_imprimir);
void imprimir_get_pokemon(void* mensaje_a_imprimir);
void imprimir_localized_pokemon(void* mensaje_a_imprimir);

void despacharMensaje(int conexion, char *argv[]);

void enviarNew(int conexion, char *argv[]);
void enviarAppeared(int conexion, char *argv[]);
void enviarCatch(int conexion, char *argv[]);
void enviarCaught(int conexion, char *argv[]);
void enviarGet(int conexion, char *argv[]);
void enviarLocalized(int conexion, int argc, char *argv[]);

#endif
