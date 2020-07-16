#include "game-card.h"

int main(void) {

	t_config* config = leer_config("../game-card.config");
	t_log* logger = NULL;
	iniciar_logger(&logger, config, "game-card");

	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	int conex_appeared = 0;
	pthread_t thread_appeared;

	int conex_catch = 0;
	pthread_t thread_catch;

	int conex_get = 0;
	pthread_t thread_get;

	//log_info(logger, "Suscribiendo a las colas de mensajes");

	int op_code = 0;

	/*
	void suscribir(t_config* config) {
		suscribirse_a_broker(config, "BROKER", op_code);
		log_info(logger, "Se realizo una suscripcion a la cola de mensajes ...");
	}
	*/

	conex_catch = suscribirse_a_broker(config, SUBSCRIBE_CATCH_POKEMON);
	log_info(logger, "Se realizo una suscripcion a la cola de mensajes CATCH_POKEMON");
	sleep(1);

	conex_appeared = suscribirse_a_broker(config, SUBSCRIBE_APPEARED_POKEMON);
	log_info(logger, "Se realizo una suscripcion a la cola de mensajes APPEARED_POKEMON");
	sleep(1);

	conex_get = suscribirse_a_broker(config, SUBSCRIBE_GET_POKEMON);
	log_info(logger, "Se realizo una suscripcion a la cola de mensajes GET_POKEMON");

	void* recibir_y_dar_ack(void* arg){

		int socket_cliente = *((int*)arg);

		pthread_mutex_lock(&mutex);
		void* response = recibir_mensaje_como_cliente(socket_cliente);
		pthread_mutex_unlock(&mutex);
		if(response == NULL){
			pthread_exit(NULL);
		}
		log_info(logger, "recibio mensaje");
		enviar_ack(socket_cliente, op_code);
		log_info(logger, "envio Ack");// creo que no hay logs obligatorios para gamecard
		return response;
	}

	while(1){

		pthread_create(&thread_appeared,NULL,(void*)recibir_y_dar_ack, (void*)&conex_appeared);
		pthread_detach(thread_appeared);

		pthread_create(&thread_get,NULL,(void*)recibir_y_dar_ack, (void*)&conex_get);
		pthread_detach(thread_get);

		pthread_create(&thread_catch,NULL,(void*)recibir_y_dar_ack, (void*)&conex_catch);
		pthread_detach(thread_catch);

	}

	terminar_programa(conex_appeared, logger, config);
	terminar_programa(conex_catch, logger, config);
	terminar_programa(conex_get, logger, config);

	return EXIT_SUCCESS;
}

int suscribirse_a_broker(t_config* config, op_code nombre_cola)
{
	int conexion = iniciar_conexion_como_cliente("BROKER", config);

	if(conexion <= 0){
		printf("\n");
		error_show(" Error de conexion\n\n");
		exit(-1);
	}

	enviar_mensaje_de_suscripcion(conexion, nombre_cola);

	return conexion;
};
