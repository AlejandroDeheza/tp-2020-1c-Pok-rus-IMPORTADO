#include "config.h"

void iniciar_logger_y_config(t_log** logger, t_config** config, char* direccion_archivo, char* nombre_proceso){

	char* log_file = NULL;

	*config = leer_config(direccion_archivo);
	asignar_string_property(*config, "LOG_FILE", &log_file);

	if(!log_file){
		error_show(" No se encontro LOG_FILE en el archivo de configuracion\n\n");
		exit(-1);
	}
	*logger = iniciar_logger(log_file, nombre_proceso);

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

t_log* iniciar_logger(char* log_file, char* project)
{
	return log_create(log_file, project , true, LOG_LEVEL_INFO);
}

void asignar_string_property(t_config* config, char* property, char** variable){

	if(config_has_property(config, property)){
		*variable = config_get_string_value(config, property);
	} else {
		*variable =	NULL;
	}
}
