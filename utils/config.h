#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include<commons/config.h>
#include <stdio.h>
#include<commons/log.h>

t_config* leer_config(char*);
void asignar_string_property(t_config*, char*, char**);
t_log* iniciar_logger(char*, char*);

#endif /* CONFIGURACION_H_ */
