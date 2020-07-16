#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/error.h>
#include <sys/socket.h>

void iniciar_logger(t_log** logger, t_config* config, char* nombre_proceso);
void leer_ip_y_puerto(char** ip, char** puerto, t_config* config, char* nombre_proceso);
t_config* leer_config(char*);

char* asignar_string_property(t_config*, char* property);
int asignar_int_property(t_config* config, char* property);

void configuracion_inicial_planificador(t_config* config, int* retardo_cliclo, char** algoritmo);

void terminar_programa(int conexion, t_log* logger, t_config* config);

#endif /* CONFIGURACION_H_ */
