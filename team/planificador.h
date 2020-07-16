/*
 * planificador.h
 *
 *  Created on: 13 jun. 2020
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <stdio.h>
#include <stdlib.h>

#include "team.h"
#include "config.h"
#include "../utils/config.h"

int RETARDO_CICLO_CPU;
char* ALGORITMO;
int CICLOS_EJECUTADOS;

void planificar();
void planifico_FIFO(t_log* logger);
void planifico_RR(t_config* config, t_log* logger);
void planifico_SJF_CD(t_config* config, t_log* logger);
void planifico_SJF_SD(t_config* config, t_log* logger);

#endif /* PLANIFICADOR_H_ */
