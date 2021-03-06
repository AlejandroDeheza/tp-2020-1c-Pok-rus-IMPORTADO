#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/error.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>

#include "estructuras.h"

t_log* generar_logger(t_config* config, char* nombre_proceso);
int leer_ip_y_puerto(char** ip, char** puerto, t_config* config, char* nombre_proceso);
t_config* leer_config(char*);

char* asignar_string_property(t_config*, char* property);
int asignar_int_property(t_config* config, char* property);

void configuracion_inicial_planificador(t_config* config, int* retardo_cliclo, char** algoritmo);

char* generar_mensaje_para_loggear(void* mensaje, op_code codigo_operacion);
char* generar_new_pokemon_para_loguear(void* mensaje_a_imprimir);
char* generar_appeared_pokemon_para_loggear(void* mensaje_a_imprimir);
char* generar_catch_pokemon_para_loggear(void* mensaje_a_imprimir);
char* generar_caught_pokemon_para_loggear(void* mensaje_a_imprimir);
char* generar_get_pokemon_para_loggear(void* mensaje_a_imprimir);
char* generar_localized_pokemon_para_loggear(void* mensaje_a_imprimir);

void imprimir_error_y_terminar_programa(char* mensaje);
void imprimir_error_y_terminar_programa_perzonalizado(char* mensaje, void(*funcion_para_finalizar)(void), pthread_mutex_t* mutex_logger);
void imprimir_error_y_terminar_hilo(char* mensaje, t_log* logger);
void terminar_programa(int conexion, t_log* logger, t_config* config, pthread_mutex_t* mutex_logger);

#endif /* CONFIGURACION_H_ */
