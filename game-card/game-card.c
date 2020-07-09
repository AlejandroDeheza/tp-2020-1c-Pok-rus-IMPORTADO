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

	void* recibir_y_dar_ack(int socket_cliente){
		void* response = recibir_mensaje(socket_cliente, &mutex);
		log_info(logger, "recibio mensaje");
		dar_ack(socket_cliente,&logger, &op_code);
		return response;
	}

	while(1){

		pthread_create(&thread_appeared,NULL,(void*)recibir_y_dar_ack,conex_appeared);
		pthread_detach(thread_appeared);

		pthread_create(&thread_get,NULL,(void*)recibir_y_dar_ack,conex_get);
		pthread_detach(thread_get);

		pthread_create(&thread_catch,NULL,(void*)recibir_y_dar_ack,conex_catch);
		pthread_detach(thread_catch);

	}

	terminar_programa(conex_appeared, logger, config);
	terminar_programa(conex_catch, logger, config);
	terminar_programa(conex_get, logger, config);

	return EXIT_SUCCESS;
}

void dar_ack(int socket_cliente,t_log* logger, int* op_code){
	send(socket_cliente, *op_code, sizeof(op_code), 0);
	log_info(logger, "envio Ack");// creo que no hay logs obligatorios para gamecard
}


int suscribirse_a(t_config* config, t_log* logger, char *nombre_proceso, op_code nombre_cola){
	int conexion;

	iniciar_conexion(&conexion, config, logger, nombre_proceso);

	mensaje_de_suscripcion(conexion, nombre_cola);

	return conexion;
};
