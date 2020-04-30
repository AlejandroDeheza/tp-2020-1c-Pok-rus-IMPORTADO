#include "broker.h"

int main(void)
{
	char* ip;
	char* puerto;

	t_config* config;

	config = leer_config("../broker.config");

	asignar_string_property(config, "IP", &ip);
	asignar_string_property(config, "PUERTO", &puerto);

	iniciar_servidor(ip, puerto);

	return EXIT_SUCCESS;
}


