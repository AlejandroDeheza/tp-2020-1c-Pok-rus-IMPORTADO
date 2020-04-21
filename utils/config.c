#include "config.h"


t_log* iniciar_logger(char* log_file, char* project)
{
	return log_create(log_file, project , true, LOG_LEVEL_INFO);

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
