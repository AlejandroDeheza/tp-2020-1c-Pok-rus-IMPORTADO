#include "game-boy.h"

#include <cliente.h>
#include "../utils/config.h"

int main(int argc, char *argv[]) {

	char* ip = NULL;
	char* puerto = NULL;
	t_log* logger = NULL;
	t_config* config = NULL;

	verificarEntrada(argc);
	int conexion = configuracionInicial(ip, puerto, &logger, &config, argv);
	despacharMensaje(conexion, &logger, argc, argv);
	//enviarAlgoParaProbar(conexion); //PARA PROBAR A MANO
	terminar_programa(conexion, logger, config);
	exit(0);
}

void verificarEntrada(int argc){

	if(argc == 1 || argc == 2 || argc == 3 ){
		printf("Debe ingresar los argumentos con el siguiente formato:\n"
		"./gameboy [PROCESO] [TIPO_MENSAJE] [ARGUMENTOS]*\n"
		"o\n"
		"./gameboy SUSCRIPTOR [COLA_DE_MENSAJES] [TIEMPO]\n");
		exit(-1);
	}
	if(argc > 8){
		printf("Ingresaste demasiados argumentos\n");
		exit(-1);
	}
}

int configuracionInicial(char* ip, char* puerto, t_log** logger, t_config** config, char *argv[]){

	char* log_file = NULL;
	int conexion = 0;

	char *funcion = malloc(20);
	char *ipElegida = malloc(30);
	char *puertoElegido = malloc(30);
	char *logConexion = malloc(40);

	*config = leer_config("../game-boy.config");
	asignar_string_property(*config, "LOG_FILE", &log_file);
	*logger = log_create(log_file, "game-boy" , true, LOG_LEVEL_INFO);

	if(!log_file){
		log_file = "game-boy.log";
	}
	printf("\nConfiguraciones:.\nLOG_FILE = %s\n", log_file);

	seleccionarFuncion(&funcion, argv[1]);
	strcat(ipElegida,"IP_");
	strcat(ipElegida, funcion);
	strcat(puertoElegido,"PUERTO_");
	strcat(puertoElegido,funcion);
	asignar_string_property(*config, ipElegida, &ip);
	asignar_string_property(*config, puertoElegido, &puerto);
	free(ipElegida);
	free(puertoElegido);

	printf("IP = %s.\nPUERTO = %s.\n", ip, puerto);

	if(!ip || !puerto){
		log_info(*logger, "Chequear archivo de configuracion");
		exit(-1);
	}

	conexion = crear_conexion( ip, puerto);
	strcat(logConexion,"Se realizo una conexion con ");
	strcat(logConexion, funcion);
	log_info(*logger, logConexion);
	free(funcion);
	free(logConexion);
	return conexion;
}

void seleccionarFuncion(char **funcion, char *primerArg){

	if(strcmp(primerArg,"SUSCRIPTOR")==0){
		//suscribirAColaDeMensajes();			//TODO
	}else if(strcmp(primerArg,"BROKER")==0){
		strcpy(*funcion, "BROKER");
	}else if(strcmp(primerArg,"TEAM")==0){
		strcpy(*funcion, "TEAM");
	}else if(strcmp(primerArg,"GAMECARD")==0){
		strcpy(*funcion, "GAMECARD");
	}else{
		printf("El primer argumento es incorrecto\n");
		exit(-1);
	}
}

void despacharMensaje(int conexion, t_log** logger, int argc, char *argv[]){

	if(strcmp(argv[2],"NEW_POKEMON")==0){
		enviarNew(conexion, argc, argv);
		logearEnvio(logger, argv);
	}else if(strcmp(argv[2],"APPEARED_POKEMON")==0){
		enviarAppeared(conexion, argc, argv);
		logearEnvio(logger, argv);
	}else if(strcmp(argv[2],"CATCH_POKEMON")==0){
		enviarCatch(conexion, argc, argv);
		logearEnvio(logger, argv);
	}else if(strcmp(argv[2],"CAUGHT_POKEMON")==0){
		enviarCaught(conexion, argc, argv);
		logearEnvio(logger, argv);
	}else if(strcmp(argv[2],"GET_POKEMON")==0){
		enviarGet(conexion, argc, argv);
		logearEnvio(logger, argv);
	}else{
		printf("El segundo argumento es incorrecto\n");
		exit(-1);
	}
}

void logearEnvio(t_log** logger, char *argv[]){

	char *logEnvioMensaje = malloc(100);
	strcpy(logEnvioMensaje, "");

	strcat(logEnvioMensaje,"Se envia el mensaje ");
	strcat(logEnvioMensaje, argv[2]);
	strcat(logEnvioMensaje, " a ");
	strcat(logEnvioMensaje, argv[1]);
	log_info(*logger, logEnvioMensaje);

	free(logEnvioMensaje);
}

/*	//PARA PROBAR A MANO
void enviarAlgoParaProbar(int conexion){

	char* nombre = malloc(20);
	int posx;
	int posy;
	int cantidad;
	printf("Ingrese nombre del pokemon:\n");
	scanf("%s", nombre);
	printf("Ingrese posicion X:\n");
	scanf("%d", &posx);
	printf("Ingrese posicion Y:\n");
	scanf("%d", &posy);
	printf("Ingrese cantidad:\n");
	scanf("%d", &cantidad);

	t_new_pokemon* pokemon = new_pokemon(nombre, posx, posy, cantidad);
	free(nombre);
	enviar_mensaje(pokemon, conexion, NEW_POKEMON);

	//char* response = recibir_mensaje(conexion);
	//printf("Mensaje devuelto: %s", response);

	//por ahora no recibe nada
}
*/

void enviarNew(int conexion, int argc, char *argv[]){

	if(argc==6){
		printf("Te faltan argumentos\n");
		exit(-1);
	}

	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);
	int cantidad = atoi(argv[6]);

	t_new_pokemon* pokemon = new_pokemon(argv[3], posx, posy, cantidad);
	enviar_mensaje(pokemon, conexion, NEW_POKEMON);
}

void enviarAppeared(int conexion, int argc, char *argv[]){

	if(argc==5){
		printf("Te faltan argumentos\n");
		exit(-1);
	}

	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);

	t_appeared_pokemon* pokemon = appeared_pokemon(argv[3], posx, posy);
	enviar_mensaje(pokemon, conexion, APPEARED_POKEMON);
}

void enviarCatch(int conexion, int argc, char *argv[]){

	if(argc==5){
		printf("Te faltan argumentos\n");
		exit(-1);
	}

	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);

	t_catch_pokemon* pokemon = catch_pokemon(argv[3], posx, posy);
	enviar_mensaje(pokemon, conexion, CATCH_POKEMON);
}

void enviarCaught(int conexion, int argc, char *argv[]){

	if(argc==3){
		printf("Te faltan argumentos\n");
		exit(-1);
	}

	int resultado = atoi(argv[3]);

	t_caught_pokemon* pokemon = caught_pokemon(resultado);
	enviar_mensaje(pokemon, conexion, CAUGHT_POKEMON);
}

void enviarGet(int conexion, int argc, char *argv[]){

	if(argc==3){
		printf("Te faltan argumentos\n");
		exit(-1);
	}

	t_get_pokemon* pokemon = get_pokemon(argv[3]);
	enviar_mensaje(pokemon, conexion, CAUGHT_POKEMON);
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

	printf("Corrio re cheto\n");
	printf("Finalizo programa.\n");
}
