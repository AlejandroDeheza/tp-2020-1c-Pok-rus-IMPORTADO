#include "config.h"

t_log* generar_logger(t_config* config, char* nombre_proceso)
{
	char* log_file = asignar_string_property(config, "LOG_FILE");

	if(!log_file) imprimir_error_y_terminar_programa("No se encontro LOG_FILE en el archivo de configuracion");

	return log_create(log_file, nombre_proceso , false, LOG_LEVEL_INFO);
}

int leer_ip_y_puerto(char** ip, char** puerto, t_config* config, char* nombre_proceso)
{
	char* ipElegida = string_new();
	char* puertoElegido = string_new();

	string_append_with_format(&ipElegida, "IP_%s", nombre_proceso);
	string_append_with_format(&puertoElegido, "PUERTO_%s", nombre_proceso);

	*ip = asignar_string_property(config, ipElegida);
	*puerto = asignar_string_property(config, puertoElegido);

	int retorno = 0;

	if((*ip) == NULL || (*puerto) == NULL ) retorno = -1;

	free(ipElegida);
	free(puertoElegido);

	return retorno;
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

		case NEW_POKEMON:
			mensaje_para_loguear = generar_new_pokemon_para_loguear(mensaje);
			break;

		case APPEARED_POKEMON:
			mensaje_para_loguear = generar_appeared_pokemon_para_loggear(mensaje);
			break;

		case CATCH_POKEMON:
			mensaje_para_loguear = generar_catch_pokemon_para_loggear(mensaje);
			break;

		case CAUGHT_POKEMON:
			mensaje_para_loguear = generar_caught_pokemon_para_loggear(mensaje);
			break;

		case GET_POKEMON:
			mensaje_para_loguear = generar_get_pokemon_para_loggear(mensaje);
			break;

		case LOCALIZED_POKEMON:
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

	char* mensaje_para_loguear = string_from_format("NEW_POKEMON    nombre: %s    x: %i    y: %i    cantidad: %i",
			(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy, mensaje->cantidad);

	//free(mensaje->nombre);
	//free(mensaje);

	return mensaje_para_loguear;
}

char* generar_appeared_pokemon_para_loggear(void* mensaje_a_imprimir)
{
	t_appeared_pokemon* mensaje = mensaje_a_imprimir;

	char* mensaje_para_loguear = string_from_format("APPEARED_POKEMON    nombre: %s    x: %i    y: %i",
			(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);

	//free(mensaje->nombre);
	//free(mensaje);

	return mensaje_para_loguear;
}

char* generar_catch_pokemon_para_loggear(void* mensaje_a_imprimir)
{
	t_catch_pokemon* mensaje = mensaje_a_imprimir;

	char* mensaje_para_loguear = string_from_format("CATCH_POKEMON    nombre: %s    x: %i    y: %i",
			(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);

	//free(mensaje->nombre);
	//free(mensaje);

	return mensaje_para_loguear;
}

char* generar_caught_pokemon_para_loggear(void* mensaje_a_imprimir)
{
	t_caught_pokemon* mensaje = mensaje_a_imprimir;

	char* mensaje_para_loguear;

	if(mensaje->resultado == 0)
	{
		mensaje_para_loguear = string_from_format("CAUGHT_POKEMON    resultado: FAIL");
	}
	else
	{
		mensaje_para_loguear = string_from_format("CAUGHT_POKEMON    resultado: OK");
	}

	//free(mensaje);

	return mensaje_para_loguear;
}

char* generar_get_pokemon_para_loggear(void* mensaje_a_imprimir)
{
	t_get_pokemon* mensaje = mensaje_a_imprimir;

	char* mensaje_para_loguear = string_from_format("GET_POKEMON    nombre: %s", (char*) mensaje->nombre);

	//free(mensaje->nombre);
	//free(mensaje);

	return mensaje_para_loguear;
}

char* generar_localized_pokemon_para_loggear(void* mensaje_a_imprimir)
{
	t_localized_pokemon* mensaje = mensaje_a_imprimir;

	char* mensaje_para_loguear = string_from_format("LOCALIZED_POKEMON    nombre: %s    cantidad de posiciones: %i"
			"    posiciones:", (char*) mensaje->nombre, mensaje->coordenadas->elements_count);

	for(int i = 0 ; i < mensaje->coordenadas->elements_count ; i++)
	{
		t_coordenadas* coordenadas = list_get(mensaje->coordenadas, i);

		int posx = coordenadas->posx;
		int posy = coordenadas->posy;

		string_append_with_format(&mensaje_para_loguear, "   %i-%i", posx, posy);
	}
	//list_destroy_and_destroy_elements(mensaje->coordenadas, free);
	//free(mensaje->nombre);
	//free(mensaje);

	return mensaje_para_loguear;
}

void imprimir_error_y_terminar_programa(char* mensaje)
{
	error_show(" %s\n", mensaje);
	perror("Descripcion ");
	printf("\n");
	fflush(stdout);
	exit(-1);
}

void imprimir_error_y_terminar_programa_perzonalizado(char* mensaje, void(*funcion_para_finalizar)(void), pthread_mutex_t* mutex_logger)
{

	if(mutex_logger != NULL)
	{
		pthread_mutex_lock(mutex_logger);
		error_show(" %s\n", mensaje);
		perror("Descripcion ");
		printf("\n");
		fflush(stdout);
		pthread_mutex_unlock(mutex_logger);
	}
	else
	{
		error_show(" %s\n", mensaje);
		perror("Descripcion ");
		printf("\n");
		fflush(stdout);
	}

	if(funcion_para_finalizar != NULL)
	{
		funcion_para_finalizar();
		exit(-1);
	}
	else
	{
		exit(-1);
	}
}

void imprimir_error_y_terminar_hilo(char* mensaje, t_log* logger)
{
	log_error(logger, "%s. Strerror : ", mensaje, strerror(errno));
	perror("Descripcion ");
	printf("\n");
	fflush(stdout);
	pthread_exit(NULL);
}

void terminar_programa(int conexion, t_log* logger, t_config* config, pthread_mutex_t* mutex_logger)
{
	int retorno = 0;

	if (logger != NULL){
		log_destroy(logger);
	}
	if (config != NULL) {
		config_destroy(config);
	}
	if (conexion != 0) {
		retorno = close(conexion);
	}

	if(retorno == -1) imprimir_error_y_terminar_programa_perzonalizado("Ocurrio un error al cerrar el socket", NULL, mutex_logger);
}
