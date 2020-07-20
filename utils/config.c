#include "config.h"

t_log* generar_logger(t_config* config, char* nombre_proceso)
{
	char* log_file = asignar_string_property(config, "LOG_FILE");

	if(!log_file) imprimir_error_y_terminar_programa("No se encontro LOG_FILE en el archivo de configuracion");

	return log_create(log_file, nombre_proceso , true, LOG_LEVEL_INFO);
}

void leer_ip_y_puerto(char** ip, char** puerto, t_config* config, char* nombre_proceso)
{
	char* ipElegida = string_new();
	char* puertoElegido = string_new();

	string_append_with_format(&ipElegida, "IP_%s", nombre_proceso);
	string_append_with_format(&puertoElegido, "PUERTO_%s", nombre_proceso);
	*ip = asignar_string_property(config, ipElegida);
	*puerto = asignar_string_property(config, puertoElegido);

	if(!(*ip) || !(*puerto)){
		printf("\n");
		error_show(" No se encontro %s o %s en el archivo de configuracion\n\n", ipElegida, puertoElegido);
		exit(-1);
	}

	free(ipElegida);
	free(puertoElegido);
}

t_config* leer_config(char* file)
{
	return config_create(file);
}

char* asignar_string_property(t_config* config, char* property)
{
	if(config_has_property(config, property)){
		return config_get_string_value(config, property);
	} else {
		return	NULL;
	}
}

int asignar_int_property(t_config* config, char* property)
{
	if(config_has_property(config, property)){
		return config_get_int_value(config, property);
	} else {
		return	0;
	}
}

void configuracion_inicial_planificador(t_config* config, int* retardo_cliclo, char** algoritmo)
{
	*retardo_cliclo = asignar_int_property(config, "RETARDO_CICLO_CPU");
	*algoritmo = asignar_string_property(config, "ALGORITMO_PLANIFICACION");
}

void imprimir_error_y_terminar_programa(const char* mensaje)
{
	printf("\n");
	error_show(" %s\n", mensaje);
	perror("Descripcion ");

	printf("\n");
	printf("\n");
	exit(-1);
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	int retorno;

	if (logger != NULL){
		log_destroy(logger);
	}
	if (config != NULL) {
		config_destroy(config);
	}
	if (conexion != 0) {
		retorno = close(conexion);
	}

	if(retorno == -1) imprimir_error_y_terminar_programa("Ocurrio un error al cerrar el socket");
}
