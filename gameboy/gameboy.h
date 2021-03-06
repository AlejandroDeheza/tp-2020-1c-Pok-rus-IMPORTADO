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

char* ARGV1;
int HILO;

int CONEXION = 0;
t_config* CONFIG = NULL;
t_log* LOGGER = NULL;

int ID_MANUAL_DEL_PROCESO = 0;
char* ID_PROCESOS_TP;

typedef struct
{
	int tiempo_suscripcion;
	int conexion_con_broker;
} argumentos_contador_de_tiempo;

void ejecutar_antes_de_terminar(int numero_senial);
void finalizar_gameboy();

void verificar_Entrada(int argc, char *argv[]);

/* MODO SUSCRIPTOR  */
void iniciar_modo_suscriptor(int conexion, char* cola_a_suscribirse, int tiempo_suscripcion);

op_code obtener_codigo_suscripcion(char* cola_a_suscribirse);
void iniciar_hilo_para_desuscripcion(int tiempo_suscripcion, int conexion_con_broker);
void* contador_de_tiempo(void* argumentos);

/* DESPACHAR MENSAJES  */
int despachar_Mensaje(int conexion, char *argv[]);

int despachar_New(int conexion, char *argv[]);
int despachar_Appeared(int conexion, char *argv[]);
int despachar_Catch(int conexion, char *argv[]);
int despachar_Caught(int conexion, char *argv[]);
int despachar_Get(int conexion, char *argv[]);
int despachar_Localized(int conexion, int argc, char *argv[]);

#endif
