#include <stdio.h>
#include <stdlib.h>
#include "game-card.h"

#include <cliente.h>
#include "../utils/config.h"

int main(void) {

	int conexion = 0;
	t_log* logger;
	t_config* config = leer_config("../game-card.config");

	iniciar_logger(&logger, config, "game-card");

	suscribirse_a(&conexion, config, logger, "BROKER", SUBSCRIBE_NEW_POKEMON);
	suscribirse_a(&conexion, config, logger, "BROKER", SUBSCRIBE_CATCH_POKEMON);
	suscribirse_a(&conexion, config, logger, "BROKER", SUBSCRIBE_GET_POKEMON);


	terminar_programa(conexion, logger, config);

	return EXIT_SUCCESS;

}

void suscribirse_a(int* conexion, t_config* config, t_log* logger, char *nombre_proceso, op_code nombre_cola){
	iniciar_conexion(&conexion, config, logger, nombre_proceso);

	while(1){
		pthread_t thread;
		pthread_create(&thread,NULL,(void*)recibir_mensaje,&conexion);
	}
};
