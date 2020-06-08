#include "config.h"

void iniciar_logger(t_log** logger, t_config* config, char* nombre_proceso){

	char* log_file = NULL;


	asignar_string_property(config, "LOG_FILE", &log_file);

	if(!log_file){
		error_show(" No se encontro LOG_FILE en el archivo de configuracion\n\n");
		exit(-1);
	}
	*logger = log_create(log_file, nombre_proceso , true, LOG_LEVEL_INFO);

	printf("Configuraciones:\n"
			"LOG_FILE = %s\n", log_file);
}

void leer_ip_y_puerto(char** ip, char** puerto, t_config* config, char* nombre_proceso){

	char *ipElegida = string_new();
	char *puertoElegido = string_new();

	string_append_with_format(&ipElegida, "IP_%s", nombre_proceso);
	string_append_with_format(&puertoElegido, "PUERTO_%s", nombre_proceso);
	asignar_string_property(config, ipElegida, ip);
	asignar_string_property(config, puertoElegido, puerto);

	if(!(*ip) || !(*puerto)){
		printf("\n");
		error_show(" No se encontro %s o %s en el archivo de configuracion\n\n", ipElegida, puertoElegido);
		exit(-1);
	}
	printf("IP = %s.\n"
		   "PUERTO = %s.\n", *ip, *puerto);

	free(ipElegida);
	free(puertoElegido);
}

t_config* leer_config(char* file)
{
	return config_create(file);
}

void asignar_string_property(t_config* config, char* property, char** variable){

	if(config_has_property(config, property)){
		*variable = config_get_string_value(config, property);
	} else {
		*variable =	NULL;
	}
}


void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	if (logger != NULL){
		log_destroy(logger);
	}
	if (config != NULL) {
		config_destroy(config);
	}
	if (conexion != 0) {
		liberar_conexion(conexion);
	}
}
