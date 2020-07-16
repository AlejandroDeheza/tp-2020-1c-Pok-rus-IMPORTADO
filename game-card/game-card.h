#ifndef GAME_CARD_H_
#define GAME_CARD_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<semaphore.h>
#include<pthread.h>

#include <cliente.h>
#include "../utils/config.h"

void terminar_programa(int conexion, t_log* log, t_config* config);
int suscribirse_a(t_config* config, char *nombre_proceso, op_code nombre_cola);

#endif
