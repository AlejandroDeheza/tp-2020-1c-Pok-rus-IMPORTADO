#include <stdio.h>
#include <stdlib.h>
#include "game-card.h"

#include <cliente.h>
#include "../utils/config.h"
#include<readline/readline.h>


int main(void) {

	t_log* logger;
	t_config* config = leer_config("../game-card.config");

	iniciar_logger(&logger, config, "game-card");
	log_info(logger, "Suscribiendo a las colas de mensajes");

	suscribirse_a(config, logger, "BROKER", SUBSCRIBE_APPEARED_POKEMON);
	//suscribirse_a(&conexionCHP, config, logger, "BROKER", SUBSCRIBE_CATCH_POKEMON);
	//suscribirse_a(&conexionGP, config, logger, "BROKER", SUBSCRIBE_GET_POKEMON);


//	terminar_programa(conexion, logger, config);

//	return EXIT_SUCCESS;

}



void suscribirse_a(t_config* config, t_log* logger, char *nombre_proceso, op_code nombre_cola){
	int conexion;

	iniciar_conexion(&conexion, config, logger, nombre_proceso);

	pthread_t thread;

	suscribirse_a_cola(conexion, nombre_cola);

	while(1){
//	pthread_create(&thread,NULL,(void*)recibir_mensaje,&conexion);
//	pthread_detach(thread);
		recibir_mensaje(conexion);
	}
};
