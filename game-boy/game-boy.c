#include "game-boy.h"

#include <cliente.h>
#include "../utils/config.h"

int main(int argc, char *argv[]) {

	int conexion = 0;
	char* ip = NULL;
	char* puerto = NULL;
	t_log* logger = NULL;
	t_config* config = NULL;

	verificarEntrada(argc, argv[1]);
	iniciar_logger_y_config(&logger, &config);

	if(strcmp(argv[1],"SUSCRIPTOR")==0){
		//suscribirAColaDeMensajes();			//TODO
	}else{
		setear_ip_y_puerto(&ip, &puerto, config, argv[1]);
		conexion = crear_conexion( ip, puerto);
		logearConexion(logger, argv[1]);

		despacharMensaje(conexion, argc, argv);
		logearEnvio(logger, argv);
	}

	terminar_programa(conexion, logger, config);
	return EXIT_SUCCESS;
}

void verificarEntrada(int argc, char *primerArg){

	printf("\n");
	if(argc == 1 || argc == 2 || argc == 3 ){
		error_show(" Debe ingresar los argumentos con el siguiente formato:\n"
		"./gameboy [PROCESO] [TIPO_MENSAJE] [ARGUMENTOS]*\n"
		"o\n"
		"./gameboy SUSCRIPTOR [COLA_DE_MENSAJES] [TIEMPO]\n\n");
		exit(-1);
	}
	if(strcmp(primerArg,"BROKER")!=0 &&
	   strcmp(primerArg,"TEAM")!=0 &&
	   strcmp(primerArg,"GAMECARD")!=0 &&
	   strcmp(primerArg,"SUSCRIPTOR")!=0){
			error_show(" El primer argumento es incorrecto\n\n");
			exit(-1);
	}
}

void iniciar_logger_y_config(t_log** logger, t_config** config){

	char* log_file = NULL;

	*config = leer_config("../game-boy.config");
	asignar_string_property(*config, "LOG_FILE", &log_file);

	if(!log_file){
		error_show(" No se encontro LOG_FILE en el game-boy.config\n\n");
		exit(-1);
	}
	*logger = iniciar_logger(log_file, "game-boy");

	printf("Configuraciones:\n"
			"LOG_FILE = %s\n", log_file);
}

void setear_ip_y_puerto(char** ip, char** puerto, t_config* config, char *primerArg){

	char *ipElegida = string_new();
	char *puertoElegido = string_new();

	string_append_with_format(&ipElegida, "IP_%s", primerArg);
	string_append_with_format(&puertoElegido, "PUERTO_%s", primerArg);
	asignar_string_property(config, ipElegida, ip);
	asignar_string_property(config, puertoElegido, puerto);

	if(!(*ip) || !(*puerto)){
		printf("\n");
		error_show(" No se encontro %s o %s en el gameboy.config\n\n", ipElegida, puertoElegido);
		exit(-1);
	}
	printf("IP = %s.\n"
			"PUERTO = %s.\n", *ip, *puerto);

	free(ipElegida);
	free(puertoElegido);
}

void logearConexion(t_log* logger, char *primerArg){

	printf("\n");
	log_info(logger, "Se realizo una conexion con %s", primerArg);
	printf("\n");
}

void despacharMensaje(int conexion, int argc, char *argv[]){

	if(strcmp(argv[2],"NEW_POKEMON")==0){
		enviarNew(conexion, argc, argv);
	}else if(strcmp(argv[2],"APPEARED_POKEMON")==0){
		enviarAppeared(conexion, argc, argv);
	}else if(strcmp(argv[2],"CATCH_POKEMON")==0){
		enviarCatch(conexion, argc, argv);
	}else if(strcmp(argv[2],"CAUGHT_POKEMON")==0){
		enviarCaught(conexion, argc, argv);
	}else if(strcmp(argv[2],"GET_POKEMON")==0){
		enviarGet(conexion, argc, argv);
	}else{
		printf("\n");
		error_show(" El segundo argumento es incorrecto\n\n");
		exit(-1);
	}
}

void logearEnvio(t_log* logger, char *argv[]){

	printf("\n");
	log_info(logger, "Se envio el mensaje %s a %s", argv[2], argv[1]);
	printf("\n");
}

void enviarNew(int conexion, int argc, char *argv[]){

	if(argc!=7){
		printf("\n");
		error_show(" Para NEW_POKEMON debe ingresar los argumentos con el siguiente formato:\n"
				"./gameboy [PROCESO] NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD]\n\n");
		exit(-1);
	}

	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);
	int cantidad = atoi(argv[6]);

	t_new_pokemon* pokemon = new_pokemon(argv[3], posx, posy, cantidad);
	enviar_mensaje(pokemon, conexion, NEW_POKEMON);
	free(pokemon->nombre);
	free(pokemon);
}

void enviarAppeared(int conexion, int argc, char *argv[]){

	if(argc!=6){
		printf("\n");
		error_show(" Para APPEARED_POKEMON debe ingresar los argumentos con el siguiente formato:\n"
						"./gameboy [PROCESO] APPEARED_POKEMON [POKEMON] [POSX] [POSY]\n\n");
		exit(-1);
	}

	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);

	t_appeared_pokemon* pokemon = appeared_pokemon(argv[3], posx, posy);
	enviar_mensaje(pokemon, conexion, APPEARED_POKEMON);
	free(pokemon->nombre);
	free(pokemon);
}

void enviarCatch(int conexion, int argc, char *argv[]){

	if(argc!=6){
		printf("\n");
		error_show(" Para CATCH_POKEMON debe ingresar los argumentos con el siguiente formato:\n"
						"./gameboy [PROCESO] CATCH_POKEMON [POKEMON] [POSX] [POSY]\n\n");
		exit(-1);
	}

	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);

	t_catch_pokemon* pokemon = catch_pokemon(argv[3], posx, posy);
	enviar_mensaje(pokemon, conexion, CATCH_POKEMON);
	free(pokemon->nombre);
	free(pokemon);
}

void enviarCaught(int conexion, int argc, char *argv[]){

	if(argc!=4){
		error_show(" Para CAUGHT_POKEMON debe ingresar los argumentos con el siguiente formato:\n"
						"./gameboy [PROCESO] CAUGHT_POKEMON [OK/FAIL]\n\n");
		exit(-1);
	}

	int resultado = atoi(argv[3]);

	t_caught_pokemon* pokemon = caught_pokemon(resultado);
	enviar_mensaje(pokemon, conexion, CAUGHT_POKEMON);
	free(pokemon);
}

void enviarGet(int conexion, int argc, char *argv[]){

	if(argc!=4){
		error_show(" Para GET_POKEMON debe ingresar los argumentos con el siguiente formato:\n"
						"./gameboy [PROCESO] GET_POKEMON [POKEMON]\n\n");
		exit(-1);
	}

	t_get_pokemon* pokemon = get_pokemon(argv[3]);
	enviar_mensaje(pokemon, conexion, CAUGHT_POKEMON);
	free(pokemon->nombre);
	free(pokemon);
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

	printf("El programa finalizo correctamente.\n\n");
}
