#include "game-boy.h"

int main(int argc, char *argv[]) {

	int conexion = 0;
	t_log* logger = NULL;
	t_config* config = NULL;

	verificarEntrada(argc, argv);
	iniciar_logger_y_config(&logger, &config, "../game-boy.config", "game-boy");

	if(strcmp(argv[1],"SUSCRIPTOR")==0){
		//suscribir_a_cola_mensajes();			//TODO
	}else{
		gestionar_envio_de_mensaje(&conexion, config, logger, argc, argv);
	}

	terminar_programa(conexion, logger, config);
	printf("El programa finalizo correctamente.\n\n");
	return EXIT_SUCCESS;
}	//cuando termine el gameboy, este main() va a quedar un poco mas ordenado

void verificarEntrada(int argc, char *argv[]){

	printf("\n");
	if(argc == 1 || argc == 2 || argc == 3 ){
		error_show(" Debe ingresar los argumentos con el siguiente formato:\n"
		"./gameboy [PROCESO] [TIPO_MENSAJE] [ARGUMENTOS]*\n"
		"o\n"
		"./gameboy SUSCRIPTOR [COLA_DE_MENSAJES] [TIEMPO](en segundos)\n\n");
		exit(-1);
	}
	if(strcmp(argv[1],"BROKER")!=0 &&
	   strcmp(argv[1],"TEAM")!=0 &&
	   strcmp(argv[1],"GAMECARD")!=0 &&
	   strcmp(argv[1],"SUSCRIPTOR")!=0){
			error_show(" El primer argumento es incorrecto\n\n");
			exit(-1);
	}
	if(strcmp(argv[1],"SUSCRIPTOR")==0){	//verificar si las entradas posibles van a ser estas, TODO
		//¿Vamos a dejar el nombre de la cola (ej: "NEW") o vamos a poner como en el otro modo (ej: "NEW_POKEMON")?
		//deberiamos preguntar cual es el formato que piden en el TP (no hay ejemplos de modo SUSCRIPTOR en enunciado)
		if(strcmp(argv[2],"NEW")!=0 &&
		   strcmp(argv[2],"APPEARED")!=0 &&
		   strcmp(argv[2],"CATCH")!=0 &&
		   strcmp(argv[2],"CAUGHT")!=0 &&
		   strcmp(argv[2],"GET")!=0){
				error_show(" El segundo argumento es incorrecto\n\n");
				exit(-1);
		}
	}else{		//segun video de planificacion falta "LOCALIZED_POKEMON" para mandarlo a team...
				// pero en el enunciado no dice nada de eso
				//deberiamos preguntar
		if(strcmp(argv[2],"NEW_POKEMON")!=0 &&
		   strcmp(argv[2],"APPEARED_POKEMON")!=0 &&
		   strcmp(argv[2],"CATCH_POKEMON")!=0 &&
		   strcmp(argv[2],"CAUGHT_POKEMON")!=0 &&
		   strcmp(argv[2],"GET_POKEMON")!=0){
				error_show(" El segundo argumento es incorrecto\n\n");
				exit(-1);
		}
	}
}

void gestionar_envio_de_mensaje(int* conexion, t_config* config, t_log* logger, int argc, char* argv[])
{
	char* ip = NULL;
	char* puerto = NULL;

	leer_ip_y_puerto(&ip, &puerto, config, argv[1]);

	*conexion = crear_conexion( ip, puerto);
	logearConexion(logger, argv[1]);

	despacharMensaje(*conexion, argc, argv);
	logearEnvio(logger, argv);
}

void logearConexion(t_log* logger, char *primerArg){

	printf("\n");
	log_info(logger, "Se realizo una conexion con %s", primerArg);
	printf("\n");
}

void despacharMensaje(int conexion, int argc, char *argv[]){

	if(strcmp(argv[2],"NEW_POKEMON")==0){
		enviarNew(conexion, argc, argv);
	}
	if(strcmp(argv[2],"APPEARED_POKEMON")==0){
		enviarAppeared(conexion, argc, argv);
	}
	if(strcmp(argv[2],"CATCH_POKEMON")==0){
		enviarCatch(conexion, argc, argv);
	}
	if(strcmp(argv[2],"CAUGHT_POKEMON")==0){
		enviarCaught(conexion, argc, argv);
	}
	if(strcmp(argv[2],"GET_POKEMON")==0){
		enviarGet(conexion, argc, argv);
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
		printf("\n");
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
		printf("\n");
		error_show(" Para GET_POKEMON debe ingresar los argumentos con el siguiente formato:\n"
						"./gameboy [PROCESO] GET_POKEMON [POKEMON]\n\n");
		exit(-1);
	}

	t_get_pokemon* pokemon = get_pokemon(argv[3]);
	enviar_mensaje(pokemon, conexion, CAUGHT_POKEMON);
	free(pokemon->nombre);
	free(pokemon);
}

void enviarLocalized(int conexion, int argc, char *argv[])
{
	//capaz nunca use esta funcion desde gameboy
	//hay que preguntar
	//TODO
	//tambien la tengo que agregar a despacharMensaje() y a verificarEntrada()

	if(argc < 4){
		printf("\n");
		error_show(" Para GET_POKEMON debe ingresar los argumentos con el siguiente formato:\n"
						"./gameboy [PROCESO] LOCALIZED_POKEMON [PARES DE COORDENADAS]*\n\n");
		exit(-1);
	}
	if(argc%2 == 1){
		printf("\n");
		error_show(" No ingresaste bien las coordenadas. Deben ser grupos de 2 numeros\n\n");
		exit(-1);
	}

	int cantidad_pares_de_coordenadas = (argc - 3) / 2;
	t_list* lista_coordenadas = list_create();
	int posicion_argc = 3;

	for(int i = 0 ; i < cantidad_pares_de_coordenadas ; i++)
	{
		t_coordenadas* coordenadas = malloc(sizeof(t_coordenadas));
		coordenadas->posx = atoi(argv[posicion_argc]);
		posicion_argc++;
		coordenadas->posy = atoi(argv[posicion_argc]);
		posicion_argc++;
		list_add(lista_coordenadas, coordenadas);
		//no hago free() porque eso se hace al hacer list_destroy_and_destroy_elements()
		//eso creo, estoy casi seguro porque vi el codigo de list.c
	}

	t_localized_pokemon* pokemon = localized_pokemon(argv[3], lista_coordenadas);
	enviar_mensaje(pokemon, conexion, LOCALIZED_POKEMON);

	list_destroy_and_destroy_elements(lista_coordenadas, free);
	free(pokemon->nombre);
	list_destroy_and_destroy_elements(pokemon->coordenadas, free);
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
}
