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

char* generar_mensaje_para_loggear(void* mensaje, op_code codigo_operacion)
{
	char* mensaje_para_loguear = NULL;

	switch (codigo_operacion)
	{
		case SUBSCRIBE_NEW_POKEMON:
			mensaje_para_loguear = generar_new_pokemon_para_loguear(mensaje);
			break;

		case SUBSCRIBE_APPEARED_POKEMON:
			mensaje_para_loguear = generar_appeared_pokemon_para_loggear(mensaje);
			break;

		case SUBSCRIBE_CATCH_POKEMON:
			mensaje_para_loguear = generar_catch_pokemon_para_loggear(mensaje);
			break;

		case SUBSCRIBE_CAUGHT_POKEMON:
			mensaje_para_loguear = generar_caught_pokemon_para_loggear(mensaje);
			break;

		case SUBSCRIBE_GET_POKEMON:
			mensaje_para_loguear = generar_get_pokemon_para_loggear(mensaje);
			break;

		case SUBSCRIBE_LOCALIZED_POKEMON:
			mensaje_para_loguear = generar_localized_pokemon_para_loggear(mensaje);
			break;

		default:
			break;
	}

	return mensaje_para_loguear;
}

char* generar_new_pokemon_para_loguear(void* mensaje_a_imprimir)
{
	t_new_pokemon* mensaje = mensaje_a_imprimir;

	char* mensaje_para_loguear = string_from_format("NEW_POKEMON nombre: %s x: %i y: %i cantidad: %i",
			(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy, mensaje->cantidad);

	free(mensaje->nombre);
	free(mensaje);

	return mensaje_para_loguear;
}

char* generar_appeared_pokemon_para_loggear(void* mensaje_a_imprimir)
{
	t_appeared_pokemon* mensaje = mensaje_a_imprimir;

	char* mensaje_para_loguear = string_from_format("APPEARED_POKEMON nombre: %s x: %i y: %i",
			(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);

	free(mensaje->nombre);
	free(mensaje);

	return mensaje_para_loguear;
}

char* generar_catch_pokemon_para_loggear(void* mensaje_a_imprimir)
{
	t_catch_pokemon* mensaje = mensaje_a_imprimir;

	char* mensaje_para_loguear = string_from_format("CATCH_POKEMON nombre: %s x: %i y: %i",
			(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);

	free(mensaje->nombre);
	free(mensaje);

	return mensaje_para_loguear;
}

char* generar_caught_pokemon_para_loggear(void* mensaje_a_imprimir)
{
	t_caught_pokemon* mensaje = mensaje_a_imprimir;

	char* mensaje_para_loguear = string_from_format("CAUGHT_POKEMON resultado: %i", (char*) mensaje->resultado);

	free(mensaje);

	return mensaje_para_loguear;
}

char* generar_get_pokemon_para_loggear(void* mensaje_a_imprimir)
{
	t_get_pokemon* mensaje = mensaje_a_imprimir;

	char* mensaje_para_loguear = string_from_format("GET_POKEMON nombre: %s", (char*) mensaje->nombre);

	free(mensaje->nombre);
	free(mensaje);

	return mensaje_para_loguear;
}

char* generar_localized_pokemon_para_loggear(void* mensaje_a_imprimir)
{
	t_localized_pokemon* mensaje = mensaje_a_imprimir;

	char* mensaje_para_loguear = string_from_format("LOCALIZED_POKEMON nombre: %s cantidad de posiciones: %i", (char*) mensaje->nombre, (mensaje->coordenadas->elements_count)/2);

	for(int i = 0 ; i < mensaje->coordenadas->elements_count ; i++)
	{
		int posx = *((int*) list_get(mensaje->coordenadas, i));
		i++;
		int posy = *((int*) list_get(mensaje->coordenadas, i));

		string_append_with_format(&mensaje_para_loguear, " x: %i y: %i", posx, posy);
	}
	list_destroy_and_destroy_elements(mensaje->coordenadas, free);
	free(mensaje->nombre);
	free(mensaje);

	return mensaje_para_loguear;
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
