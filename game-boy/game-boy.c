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
	despacharMensaje(conexion, argc, argv);
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

	char primerArg[TAMANIO_MAXIMO_ARGUMENTO];
	char* log_file = NULL;
	int conexion = 0;

	*config = leer_config("../game-boy.config");
	asignar_string_property(*config, "LOG_FILE", &log_file);
	*logger = log_create(log_file, "game-boy" , true, LOG_LEVEL_INFO);

	if(!log_file){
		log_file = "game-boy.log";
	}
	printf("\nConfiguraciones:.\nLOG_FILE = %s\n", log_file);

	strcpy(primerArg, argv[1]);

	if(strcmp(primerArg,"SUSCRIPTOR")==0){
		//suscribirAColaDeMensajes();			//TODO
		}else if(strcmp(primerArg,"BROKER")==0){
			destinoBroker(&ip, &puerto, *config);
		}else if(strcmp(primerArg,"TEAM")==0){
			destinoTeam(&ip, &puerto, *config);
		}else if(strcmp(primerArg,"GAMECARD")==0){
			destinoGamecard(&ip, &puerto, *config);
		}else{
			printf("El primer argumento es incorrecto\n");
			exit(-1);
			}

		printf("IP = %s.\nPUERTO = %s.\n", ip, puerto);

	if(!ip || !puerto){
		log_info(*logger, "Chequear archivo de configuracion");
		exit(-1);
	}

	conexion = crear_conexion( ip, puerto);
	return conexion;
}

void destinoBroker(char** ip, char** puerto, t_config* config){

	asignar_string_property(config, "IP_BROKER", ip);
	asignar_string_property(config, "PUERTO_BROKER", puerto);
}

void destinoTeam(char** ip, char** puerto, t_config* config){

	asignar_string_property(config, "IP_TEAM", ip);
	asignar_string_property(config, "PUERTO_TEAM", puerto);
}

void destinoGamecard(char** ip, char** puerto, t_config* config){

	asignar_string_property(config, "IP_GAMECARD", ip);
	asignar_string_property(config, "PUERTO_GAMECARD", puerto);
}

void despacharMensaje(int conexion, int argc, char *argv[]){

	char segundoArg[TAMANIO_MAXIMO_ARGUMENTO];
	char tercerArg[TAMANIO_MAXIMO_ARGUMENTO];
	char cuartoArg[TAMANIO_MAXIMO_ARGUMENTO];
	char quintoArg[TAMANIO_MAXIMO_ARGUMENTO];
	char sextoArg[TAMANIO_MAXIMO_ARGUMENTO];
	char septimoArg[TAMANIO_MAXIMO_ARGUMENTO];
	char octavoArg[TAMANIO_MAXIMO_ARGUMENTO];

	//tengo que pensar mejor esto
	strcpy(segundoArg, argv[2]);
	strcpy(tercerArg, argv[3]);
	strcpy(cuartoArg, argv[4]);
	strcpy(quintoArg, argv[5]);
	strcpy(sextoArg, argv[6]);
	//strcpy(septimoArg, argv[7]);
	//strcpy(octavoArg, argv[8]);

/*	//PARA PROBAR SIN CONSOLA
	strcpy(primerArg, "BROKER");
	strcpy(segundoArg, "NEW_POKEMON");
	strcpy(tercerArg, "asd");
	strcpy(cuartoArg, "1");
	strcpy(quintoArg, "2");
	strcpy(sextoArg, "3");
*/

	if(strcmp(segundoArg,"NEW_POKEMON")==0){
		enviarNew(conexion, tercerArg, cuartoArg, quintoArg, sextoArg);
		}else if(strcmp(segundoArg,"APPEARED_POKEMON")==0){
			enviarAppeared(conexion, tercerArg, cuartoArg, quintoArg);
		}else if(strcmp(segundoArg,"CATCH_POKEMON")==0){
			enviarCatch(conexion, tercerArg, cuartoArg, quintoArg);
		}else if(strcmp(segundoArg,"CAUGHT_POKEMON")==0){
			enviarCaught(conexion, tercerArg);
		}else if(strcmp(segundoArg,"GET_POKEMON")==0){
			enviarGet(conexion, tercerArg);
		}else{
			printf("El segundo argumento es incorrecto\n");
			exit(-1);
		}
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

void enviarNew(int conexion, char nombre[], char cuartoArg[], char quintoArg[], char sextoArg[]){

	int posx = atoi(cuartoArg);
	int posy = atoi(quintoArg);
	int cantidad = atoi(sextoArg);

	t_new_pokemon* pokemon = new_pokemon(nombre, posx, posy, cantidad);
	enviar_mensaje(pokemon, conexion, NEW_POKEMON);
}

void enviarAppeared(int conexion, char nombre[], char cuartoArg[], char quintoArg[]){

	int posx = atoi(cuartoArg);
	int posy = atoi(quintoArg);

	t_appeared_pokemon* pokemon = appeared_pokemon(nombre, posx, posy);
	enviar_mensaje(pokemon, conexion, APPEARED_POKEMON);
}

void enviarCatch(int conexion, char nombre[], char cuartoArg[], char quintoArg[]){

	int posx = atoi(cuartoArg);
	int posy = atoi(quintoArg);

	t_catch_pokemon* pokemon = catch_pokemon(nombre, posx, posy);
	enviar_mensaje(pokemon, conexion, CATCH_POKEMON);
}

void enviarCaught(int conexion, char tercerArg[]){

	int resultado = atoi(tercerArg);

	t_caught_pokemon* pokemon = caught_pokemon(resultado);
	enviar_mensaje(pokemon, conexion, CAUGHT_POKEMON);
}

void enviarGet(int conexion, char nombre[]){

	t_get_pokemon* pokemon = get_pokemon(nombre);
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
