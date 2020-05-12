#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <stdlib.h>
#include <stdio.h>
#include<commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/error.h>

void iniciar_logger_y_config(t_log** logger, t_config** config,
		char* direccion_archivo_configuracion, char* nombre_proceso);

void leer_ip_y_puerto(char** ip, char** puerto, t_config* config, char* nombre_proceso);

t_config* leer_config(char*);
void asignar_string_property(t_config*, char*, char**);
t_log* iniciar_logger(char*, char*);

#endif /* CONFIGURACION_H_ */
