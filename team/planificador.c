/*
 * planificador.c
 *
 *  Created on: 13 jun. 2020
 *      Author: utnso
 */

#include "planificador.h"

//----------------------------------------------------------------------------------------------------------
// Proceso para planificar a los entrenadores
// Obtiene el algoritmo a utilizar y los segundos que dura cada ciclo de cpu desde el archivo de configuracion
// Este proceso ejecuta en un hilo.
// Va planificando los entrenadores que ingresen a la lista de ready segun el algoritmo seleccionado.
//----------------------------------------------------------------------------------------------------------
void planificar(argumentos_planificador* argumentos){

	configuracion_inicial_planificador(argumentos->config, &RETARDO_CICLO_CPU, &ALGORITMO);

	if (strcmp(ALGORITMO, "FIFO") == 0) {
		planifico_FIFO(argumentos->logger);
	} else if (strcmp(ALGORITMO, "RR") == 0) {
		planifico_RR(argumentos->config, argumentos->logger);
	} else if (strcmp(ALGORITMO, "SJF-CD") == 0) {
		planifico_SJF_CD(argumentos->config, argumentos->logger);
	} else if (strcmp(ALGORITMO, "SJF-SJ") == 0) {
		planifico_SJF_SD(argumentos->config, argumentos->logger);
	}

}

//----------------------------------------------------------------------------------------------------------
// Planificacion con algoritmo FIFO
// Si existen entrenadores en la lista de ready:
//     1. Obtengo el primero (indice 0)
//     2. Lo hago procesar de a 1 ciclo hasta que no tenga mas rafagas
//     3. Muevo el tcb a la lista de bloqueados en espera
//     4. Elimino proceso de lista de ready
//----------------------------------------------------------------------------------------------------------
void planifico_FIFO(t_log* logger) {
	while(1){
		if(!list_is_empty(entrenadores_ready)){
			t_entrenador_tcb* tcb_entrenador = list_get(entrenadores_ready,0);

			while(tcb_entrenador->rafagas > 0){
				pthread_mutex_unlock(&(tcb_entrenador->mutex));
				pthread_mutex_lock(&(tcb_entrenador->mutex));
				tcb_entrenador->rafagas -= 1;
				sleep(RETARDO_CICLO_CPU);
			}

			list_add(entrenadores_blocked_espera, tcb_entrenador);
			list_remove(entrenadores_ready, 0);
		}
	}
}

void planifico_RR(t_config* config, t_log* logger){

}

void planifico_SJF_CD(t_config* config, t_log* logger){


}

void planifico_SJF_SD(t_config* config, t_log* logger){
}

