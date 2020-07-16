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
void planificar(){

	configuracion_inicial_planificador(CONFIG, &RETARDO_CICLO_CPU, &ALGORITMO);
	CICLOS_EJECUTADOS = 0;

	log_info(LOGGER, "Se va a planificar utilizando el algoritomo %s", ALGORITMO);

	if (strcmp(ALGORITMO, "FIFO") == 0) {
		planifico_FIFO(LOGGER);
	} else if (strcmp(ALGORITMO, "RR") == 0) {
		planifico_RR(CONFIG, LOGGER);
	} else if (strcmp(ALGORITMO, "SJF-CD") == 0) {
		planifico_SJF_CD(CONFIG, LOGGER);
	} else if (strcmp(ALGORITMO, "SJF-SJ") == 0) {
		planifico_SJF_SD(CONFIG, LOGGER);
	}

}

//----------------------------------------------------------------------------------------------------------
// Planificacion con algoritmo FIFO
// Si existen entrenadores en la lista de ready:
//     1. Obtengo el primero (indice 0)
//     2. Muevo el elemento a EXEC y lo saco de READY
//     3. Lo hago procesar de a 1 ciclo hasta que no tenga mas rafagas
//     4. Segun el estado de la conexion
//        a. Activa: Muevo el tcb a la lista de bloqueados en espera
//        b. Inactiva: Por default, asomo que atrapo al pokemon y muevo el tcb a la lista de bloqueados sin espera
//     5. Elimino proceso de lista de EXEC
//----------------------------------------------------------------------------------------------------------
void planifico_FIFO(t_log* logger) {
	while(1){
		if(!list_is_empty(entrenadores_ready)){
			t_entrenador_tcb* tcb_entrenador = list_get(entrenadores_ready,0);

			log_info(LOGGER, "Entra a EXEC un entrenador con %d rafagas", tcb_entrenador->rafagas);

			list_add(entrenadores_exec, tcb_entrenador);
			list_remove(entrenadores_ready, 0);

			while(tcb_entrenador->rafagas > 0){
				log_info(LOGGER, "Ejecuto 1 ciclo de CPU");
				pthread_mutex_unlock(&(tcb_entrenador->mutex));
				tcb_entrenador->rafagas -= 1;
				CICLOS_EJECUTADOS += 1;
				sleep(RETARDO_CICLO_CPU);
			}

			if(conexion_con_broker == CONECTADO){
				log_info(LOGGER, "El entrenador queda bloqueado esperando la respuesta del CATCH");
				list_add(entrenadores_blocked_espera, tcb_entrenador);
			} else {
				log_info(LOGGER, "Sin conexion con Broker, se asume que CATCH fue exitoso");
				list_add(tcb_entrenador->entrenador->pokemones_entrenador, tcb_entrenador->nombre_pokemon);
				list_add(entrenadores_blocked_sin_espera, tcb_entrenador);
			}

			list_remove(entrenadores_exec, 0);
		}
	}
}

//----------------------------------------------------------------------------------------------------------
// Planificacion con algoritmo Round Robin
// Obtengo el quantum del config
// Si existen entrenadores en la lista de ready:
//     1. Obtengo el primero (indice 0)
//     2. Itero hasta que las rafagas ejecutadas no superen el quantum
//			  y hasta que no tenga mas rafagas para ejecutar
//     3. Muevo el tcb a la lista de bloqueados en espera
//     4. Elimino proceso de lista de ready
//----------------------------------------------------------------------------------------------------------
void planifico_RR(t_config* config, t_log* logger){

	int quantum = asignar_int_property(config, "QUANTUM");

	if(quantum == 0){
		log_error(logger, "No existe la propiedad QUANTUM");
		exit(-1);
	}

	while(1){
		if(!list_is_empty(entrenadores_ready)){
			t_entrenador_tcb* tcb_entrenador = list_get(entrenadores_ready,0);
			int rafagasEjecutadas = 0;

			while(rafagasEjecutadas < quantum && tcb_entrenador->rafagas > 0){
				pthread_mutex_unlock(&(tcb_entrenador->mutex));
				pthread_mutex_lock(&(tcb_entrenador->mutex));
				tcb_entrenador->rafagas -= 1;
				sleep(RETARDO_CICLO_CPU);

				rafagasEjecutadas++;
				tcb_entrenador->rafagas--;
			}

			list_add(entrenadores_blocked_espera, tcb_entrenador);
			list_remove(entrenadores_ready, 0);
		}
	}
}

void planifico_SJF_CD(t_config* config, t_log* logger){


}

void planifico_SJF_SD(t_config* config, t_log* logger){
}

