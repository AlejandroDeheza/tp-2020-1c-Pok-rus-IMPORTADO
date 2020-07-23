#ifndef GAMEBOY_H_
#define GAMEBOY_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <commons/error.h>
#include<pthread.h>

#include <cliente.h>
#include "../utils/config.h"

typedef struct
{
	int tiempo_suscripcion;
	int conexion_con_broker;
	op_code codigo_desuscripcion;
} argumentos_contador_de_tiempo;

void verificar_Entrada(int argc, char *argv[]);

/* MODO SUSCRIPTOR  */
void iniciar_modo_suscriptor(int conexion, t_log* logger, char* cola_a_suscribirse, int tiempo_suscripcion);

void obtener_codigos(char* cola_a_suscribirse, op_code* codigo_suscripcion, op_code* codigo_desuscripcion);
void iniciar_hilo_para_desuscripcion(int tiempo_suscripcion, int conexion_con_broker, op_code codigo_desuscripcion);
void* contador_de_tiempo(void* argumentos);

/* IMPRIMIR MENSAJES  */
char* imprimir_mensaje_recibido(void* mensaje_a_imprimir, op_code codigo_operacion);

char* imprimir_new_pokemon(void* mensaje_a_imprimir);
char* imprimir_appeared_pokemon(void* mensaje_a_imprimir);
char* imprimir_catch_pokemon(void* mensaje_a_imprimir);
char* imprimir_caught_pokemon(void* mensaje_a_imprimir);
char* imprimir_get_pokemon(void* mensaje_a_imprimir);
char* imprimir_localized_pokemon(void* mensaje_a_imprimir);

/* DESPACHAR MENSAJES  */
int despachar_Mensaje(int conexion, char *argv[]);

int despachar_New(int conexion, char *argv[]);
int despachar_Appeared(int conexion, char *argv[]);
int despachar_Catch(int conexion, char *argv[]);
int despachar_Caught(int conexion, char *argv[]);
int despachar_Get(int conexion, char *argv[]);
int despachar_Localized(int conexion, int argc, char *argv[]);

#endif
