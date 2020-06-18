#include <stdio.h>
#include <stdlib.h>
#include "game-card.h"

#include <cliente.h>
#include "../utils/config.h"
#include<readline/readline.h>




int main(void) {

	t_log* logger;
	t_config* config = leer_config("../game-card.config");
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	int conex_appeared = 0;
	pthread_t thread_appeared;

	int conex_catch = 0;
	pthread_t thread_catch;

	int conex_get = 0;
	pthread_t thread_get;

	iniciar_logger(&logger, config, "game-card");
	log_info(logger, "Suscribiendo a las colas de mensajes");

	int op_code;

	void suscribir(t_config* config) {
		suscribirse_a(config, logger, "BROKER", op_code);
	}

	conex_catch = suscribirse_a(config, logger, "BROKER", SUBSCRIBE_CATCH_POKEMON);
	sleep(1);
	conex_appeared = suscribirse_a(config, logger, "BROKER", SUBSCRIBE_APPEARED_POKEMON);
	sleep(1);
	conex_get = suscribirse_a(config, logger, "BROKER", SUBSCRIBE_GET_POKEMON);

	void* recibir_con_semaforo(int socket_cliente){
		pthread_mutex_lock(&mutex);
		void* response = recibir_mensaje(socket_cliente, &mutex);
		log_info(logger, "recibio mensaje");
		pthread_mutex_unlock(&mutex);
		return response;
	}

	while(1){

		pthread_create(&thread_appeared,NULL,(void*)recibir_con_semaforo,conex_appeared);
		pthread_detach(thread_appeared);

		pthread_create(&thread_get,NULL,(void*)recibir_con_semaforo,conex_get);
		pthread_detach(thread_get);

		pthread_create(&thread_catch,NULL,(void*)recibir_con_semaforo,conex_catch);
		pthread_detach(thread_catch);

	}

	terminar_programa(conex_appeared, logger, config);
	terminar_programa(conex_catch, logger, config);
	terminar_programa(conex_get, logger, config);

	return EXIT_SUCCESS;
}



int suscribirse_a(t_config* config, t_log* logger, char *nombre_proceso, op_code nombre_cola){
	int conexion;

	iniciar_conexion(&conexion, config, logger, nombre_proceso);

	suscribirse_a_cola(conexion, nombre_cola);

	return conexion;
};
