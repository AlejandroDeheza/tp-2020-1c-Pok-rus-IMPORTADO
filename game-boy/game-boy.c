#include "game-boy.h"

int main(int argc, char *argv[]) {

	int conexion = 0;
	t_log* logger = NULL;
	t_config* config = leer_config("../game-boy.config");

	verificarEntrada(argc, argv);
	iniciar_logger(&logger, config, "game-boy");

	if(strcmp(argv[1],"SUSCRIPTOR")==0){

		char *orden_de_suscripcion = string_new();
		string_append_with_format(&orden_de_suscripcion, "SUBSCRIBE_%s", argv[2]);
		iniciar_conexion(&conexion, config, logger, "BROKER", orden_de_suscripcion);
		free(orden_de_suscripcion);

		op_code codigo_suscripcion = 0;
		op_code codigo_desuscripcion = 0;
		obtener_codigos(argv[2], &codigo_suscripcion, &codigo_desuscripcion);
		printf("\nenviando mensaje para suscripcion:\n");
		enviar_mensaje_de_suscripcion(conexion, codigo_suscripcion);
		log_info(logger, "Se realizo una suscripcion a la cola de mensajes %s", argv[2]);
		iniciar_hilo_para_desuscripcion(atoi(argv[3]), conexion, codigo_desuscripcion);

		while(true)
		{
			int retorno = imprimir_mensaje_recibido(codigo_suscripcion, conexion);
			if(retorno == -1)break;
			log_info(logger, "Se recibio un mensaje de la cola %s", argv[2]);
		}

	}else{
		iniciar_conexion(&conexion, config, logger, argv[1], argv[2]);
		despacharMensaje(conexion, argv);
	}

	terminar_programa(conexion, logger, config);
	printf("\nEl Game-Boy finalizo correctamente.\n\n");
	return EXIT_SUCCESS;
}

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
	if(strcmp(argv[1],"SUSCRIPTOR")==0){

		if(strcmp(argv[2],"NEW_POKEMON")!=0 &&
		   strcmp(argv[2],"APPEARED_POKEMON")!=0 &&
		   strcmp(argv[2],"CATCH_POKEMON")!=0 &&
		   strcmp(argv[2],"CAUGHT_POKEMON")!=0 &&
		   strcmp(argv[2],"GET_POKEMON")!=0 &&
		   strcmp(argv[2],"LOCALIZED_POKEMON")!=0){
				error_show(" El segundo argumento es incorrecto\n\n");
				exit(-1);
		}
	}else{
		if(strcmp(argv[2],"NEW_POKEMON")!=0 &&
		   strcmp(argv[2],"APPEARED_POKEMON")!=0 &&
		   strcmp(argv[2],"CATCH_POKEMON")!=0 &&
		   strcmp(argv[2],"CAUGHT_POKEMON")!=0 &&
		   strcmp(argv[2],"GET_POKEMON")!=0){
				error_show(" El segundo argumento es incorrecto\n\n");
				exit(-1);
		}
	}


	if(strcmp(argv[1],"BROKER")==0)
	{
		if(strcmp(argv[2],"NEW_POKEMON")==0 && argc != 7){
			error_show(" Escribiste una cantidad incorrecta de argumentos\n\n");
			error_show(" Para enviar NEW_POKEMON a BROKER debe ingresar los argumentos con el siguiente formato:\n"
			"./gameboy BROKER NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD]\n\n");
			exit(-1);
		}

		if(strcmp(argv[2],"APPEARED_POKEMON")==0 && argc != 7){
			error_show(" Escribiste una cantidad incorrecta de argumentos\n\n");
			error_show(" Para enviar APPEARED_POKEMON a BROKER debe ingresar los argumentos con el siguiente formato:\n"
			"./gameboy BROKER APPEARED_POKEMON [POKEMON] [POSX] [POSY] [ID_MENSAJE_CORRELATIVO]\n\n");
			exit(-1);
		}

		if(strcmp(argv[2],"CATCH_POKEMON")==0 && argc != 6){
			error_show(" Escribiste una cantidad incorrecta de argumentos\n\n");
			error_show(" Para enviar CATCH_POKEMON a BROKER debe ingresar los argumentos con el siguiente formato:\n"
			"./gameboy BROKER CATCH_POKEMON [POKEMON] [POSX] [POSY]\n\n");
			exit(-1);
		}

		if(strcmp(argv[2],"CAUGHT_POKEMON")==0 && argc != 5){
			error_show(" Escribiste una cantidad incorrecta de argumentos\n\n");
			error_show(" Para enviar CAUGHT_POKEMON a BROKER debe ingresar los argumentos con el siguiente formato:\n"
			"./gameboy [PROCESO] CAUGHT_POKEMON [ID_MENSAJE_CORRELATIVO] [OK/FAIL]\n\n");
			exit(-1);
		}

		if(strcmp(argv[2],"GET_POKEMON")==0 && argc != 4){
			error_show(" Escribiste una cantidad incorrecta de argumentos\n\n");
			error_show(" Para enviar GET_POKEMON a BROKER debe ingresar los argumentos con el siguiente formato:\n"
			"./gameboy BROKER GET_POKEMON [POKEMON]\n\n");
			exit(-1);
		}
	}

	if(strcmp(argv[1],"TEAM")==0)
	{
		if(strcmp(argv[2],"APPEARED_POKEMON")==0 && argc != 6){
			error_show(" Escribiste una cantidad incorrecta de argumentos\n\n");
			error_show(" Para enviar APPEARED_POKEMON a TEAM debe ingresar los argumentos con el siguiente formato:\n"
			"./gameboy TEAM APPEARED_POKEMON [POKEMON] [POSX] [POSY]\n\n");
			exit(-1);
		}

		if(strcmp(argv[2],"APPEARED_POKEMON")!=0){
			error_show(" No podes mandar este mensaje a TEAM\n\n");
			exit(-1);
		}
	}

	if(strcmp(argv[1],"GAMECARD")==0)
	{
		if(strcmp(argv[2],"NEW_POKEMON")==0 && argc != 8){
			error_show(" Escribiste una cantidad incorrecta de argumentos\n\n");
			error_show(" Para enviar NEW_POKEMON a GAMECARD debe ingresar los argumentos con el siguiente formato:\n"
			"./gameboy GAMECARD NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD] [ID_MENSAJE]\n\n");
			exit(-1);
		}

		if(strcmp(argv[2],"GET_POKEMON")==0 && argc != 5){
			error_show(" Escribiste una cantidad incorrecta de argumentos\n\n");
			error_show(" Para enviar GET_POKEMON a GAMECARD debe ingresar los argumentos con el siguiente formato:\n"
			"./gameboy GAMECARD GET_POKEMON [POKEMON] [ID_MENSAJE]\n\n");
			exit(-1);
		}

		if(strcmp(argv[2],"CATCH_POKEMON")==0 && argc != 7){
			error_show(" Escribiste una cantidad incorrecta de argumentos\n\n");
			error_show(" Para enviar CATCH_POKEMON a GAMECARD debe ingresar los argumentos con el siguiente formato:\n"
			"./gameboy GAMECARD CATCH_POKEMON [POKEMON] [POSX] [POSY] [ID_MENSAJE]\n\n");
			exit(-1);
		}

		if(strcmp(argv[2],"NEW_POKEMON")!=0 &&
		   strcmp(argv[2],"GET_POKEMON")!=0 &&
		   strcmp(argv[2],"CATCH_POKEMON")!=0){
					error_show(" No podes mandar este mensaje a GAMECARD\n\n");
					exit(-1);
		}
	}
}

void obtener_codigos(char* cola_a_suscribirse, op_code* codigo_suscripcion, op_code* codigo_desuscripcion)
{
	if(strcmp(cola_a_suscribirse,"NEW_POKEMON")==0){
		*codigo_suscripcion = SUBSCRIBE_NEW_POKEMON;
		*codigo_desuscripcion = UNSUBSCRIBE_NEW_POKEMON;
	}
	if(strcmp(cola_a_suscribirse,"APPEARED_POKEMON")==0){
		*codigo_suscripcion = SUBSCRIBE_APPEARED_POKEMON;
		*codigo_desuscripcion = UNSUBSCRIBE_APPEARED_POKEMON;
	}
	if(strcmp(cola_a_suscribirse,"CATCH_POKEMON")==0){
		*codigo_suscripcion = SUBSCRIBE_CATCH_POKEMON;
		*codigo_desuscripcion = UNSUBSCRIBE_CATCH_POKEMON;
	}
	if(strcmp(cola_a_suscribirse,"CAUGHT_POKEMON")==0){
		*codigo_suscripcion = SUBSCRIBE_CAUGHT_POKEMON;
		*codigo_desuscripcion = UNSUBSCRIBE_CAUGHT_POKEMON;
	}
	if(strcmp(cola_a_suscribirse,"GET_POKEMON")==0){
		*codigo_suscripcion = SUBSCRIBE_GET_POKEMON;
		*codigo_desuscripcion = UNSUBSCRIBE_GET_POKEMON;
	}
	if(strcmp(cola_a_suscribirse,"LOCALIZED_POKEMON")==0){
		*codigo_suscripcion = SUBSCRIBE_LOCALIZED_POKEMON;
		*codigo_desuscripcion = UNSUBSCRIBE_LOCALIZED_POKEMON;
	}
}

void iniciar_hilo_para_desuscripcion(int tiempo_suscripcion, int conexion_con_broker, op_code codigo_desuscripcion)
{
	argumentos_contador_de_tiempo* arg = malloc(sizeof(argumentos_contador_de_tiempo));
	arg->tiempo_suscripcion = tiempo_suscripcion;
	arg->conexion_con_broker = conexion_con_broker;
	arg->codigo_desuscripcion = codigo_desuscripcion;

	pthread_t thread;
	if(0 != pthread_create(&thread, NULL, (void*) contador_de_tiempo, (void*)arg)){
		error_show(" No se pudo crear un hilo de gameboy\n\n");
		exit(-1);
	}
	pthread_detach(thread);
}

void* contador_de_tiempo(void* argumentos)
{
	argumentos_contador_de_tiempo* args = argumentos;

	int tiempo_suscripcion = args->tiempo_suscripcion;
	int conexion_con_broker = args->conexion_con_broker;
	op_code codigo_desuscripcion = args->codigo_desuscripcion;

	sleep(tiempo_suscripcion);

	printf("\nenviando mensaje para desuscripcion:\n");
	enviar_mensaje_de_suscripcion(conexion_con_broker, codigo_desuscripcion);

	shutdown(conexion_con_broker, SHUT_RDWR);

	return NULL;
}

int imprimir_mensaje_recibido(op_code codigo_operacion, int conexion_con_broker)
{
	void* mensaje = recibir_mensaje(conexion_con_broker);

	if(mensaje == NULL)return -1;

	enviar_ack(conexion_con_broker, codigo_operacion);
	//estos ACK estan bien? revisar TODO
	//no esta relacionado con el id_mensaje y el id_correlativo?

	switch (codigo_operacion)
	{
		case SUBSCRIBE_NEW_POKEMON:
			imprimir_new_pokemon(mensaje);
			break;

		case SUBSCRIBE_APPEARED_POKEMON:
			imprimir_appeared_pokemon(mensaje);
			break;

		case SUBSCRIBE_CATCH_POKEMON:
			imprimir_catch_pokemon(mensaje);
			break;

		case SUBSCRIBE_CAUGHT_POKEMON:
			imprimir_caught_pokemon(mensaje);
			break;

		case SUBSCRIBE_GET_POKEMON:
			imprimir_get_pokemon(mensaje);
			break;

		case SUBSCRIBE_LOCALIZED_POKEMON:
			imprimir_localized_pokemon(mensaje);
			break;

		default:
			break;
	}

	return 0;
}

void imprimir_new_pokemon(void* mensaje_a_imprimir)
{
	t_new_pokemon* mensaje = mensaje_a_imprimir;

	printf("\n		NEW POKEMON\n"
			"		nombre: %s\n"
			"		posicion x: %i\n"
			"		posicion y: %i\n"
			"		cantidad: %i\n",
	(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy, mensaje->cantidad);

	free(mensaje->nombre);
	free(mensaje);
}

void imprimir_appeared_pokemon(void* mensaje_a_imprimir)
{
	t_appeared_pokemon* mensaje = mensaje_a_imprimir;

	printf("\n		APPEARED POKEMON\n"
			"		nombre: %s\n"
			"		posicion x: %i\n"
			"		posicion y: %i\n",
	(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);

	free(mensaje->nombre);
	free(mensaje);
}

void imprimir_catch_pokemon(void* mensaje_a_imprimir)
{
	t_catch_pokemon* mensaje = mensaje_a_imprimir;

	printf("\n		CATCH POKEMON\n"
			"		nombre: %s\n"
			"		posicion x: %i\n"
			"		posicion y: %i\n",
	(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);

	free(mensaje->nombre);
	free(mensaje);
}

void imprimir_caught_pokemon(void* mensaje_a_imprimir)
{
	t_caught_pokemon* mensaje = mensaje_a_imprimir;

	printf("\n		CAUGHT POKEMON\n"
			"		resultado: %i\n",
	mensaje->resultado);

	free(mensaje);
}

void imprimir_get_pokemon(void* mensaje_a_imprimir)
{
	t_get_pokemon* mensaje = mensaje_a_imprimir;

	printf("\n		GET POKEMON\n"
			"		nombre: %s\n",
			(char*) mensaje->nombre);

	free(mensaje->nombre);
	free(mensaje);
}

void imprimir_localized_pokemon(void* mensaje_a_imprimir)
{
	t_localized_pokemon* mensaje = mensaje_a_imprimir;

	printf("\n		LOCALIZED POKEMON\n"
			"		nombre: %s\n",
			(char*) mensaje->nombre);

	for(int i = 0 ; i < mensaje->coordenadas->elements_count ; i++)
	{
		int posx = *((int*) list_get(mensaje->coordenadas, i));
		i++;
		int posy = *((int*) list_get(mensaje->coordenadas, i));

		printf( "		posicion x: %i\n"
				"		posicion y: %i\n\n",
				posx, posy);
	}
	list_destroy_and_destroy_elements(mensaje->coordenadas, free);
	free(mensaje->nombre);
	free(mensaje);
}

void despacharMensaje(int conexion, char *argv[]){

	if(strcmp(argv[2],"NEW_POKEMON")==0){
		enviarNew(conexion, argv);
	}
	if(strcmp(argv[2],"APPEARED_POKEMON")==0){
		enviarAppeared(conexion, argv);
	}
	if(strcmp(argv[2],"CATCH_POKEMON")==0){
		enviarCatch(conexion, argv);
	}
	if(strcmp(argv[2],"CAUGHT_POKEMON")==0){
		enviarCaught(conexion, argv);
	}
	if(strcmp(argv[2],"GET_POKEMON")==0){
		enviarGet(conexion, argv);
	}
}

void enviarNew(int conexion, char *argv[])
{
	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);
	int cantidad = atoi(argv[6]);
	int id_mensaje = 0;
	int id_correlativo = 0;

	if(strcmp(argv[1],"GAMECARD")==0)	//SE ENVIO EL MENSAJE PARA GAMECARD
		id_mensaje = atoi(argv[7]);

	enviar_new_pokemon(conexion, id_mensaje, id_correlativo, argv[3], posx, posy, cantidad);
}

void enviarAppeared(int conexion, char *argv[])
{
	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);
	int id_mensaje = 0;
	int id_correlativo = 0;

	if(strcmp(argv[1],"BROKER")==0)	 	//SE ENVIO EL MENSAJE PARA BROKER
		id_correlativo = atoi(argv[6]);

	enviar_appeared_pokemon(conexion, id_mensaje, id_correlativo, argv[3], posx, posy);
}

void enviarCatch(int conexion, char *argv[])
{
	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);
	int id_mensaje = 0;
	int id_correlativo = 0;

	if(strcmp(argv[1],"GAMECARD")==0)	//SE ENVIO EL MENSAJE PARA GAMECARD
		id_mensaje = atoi(argv[6]);

	enviar_catch_pokemon(conexion, id_mensaje, id_correlativo, argv[3], posx, posy);
}

void enviarCaught(int conexion, char *argv[])
{
	int id_mensaje = 0;
	int id_correlativo = atoi(argv[3]);
	int resultado = atoi(argv[4]);

	enviar_caught_pokemon(conexion, id_mensaje, id_correlativo, resultado);
}

void enviarGet(int conexion, char *argv[])
{
	int id_mensaje = 0;
	int id_correlativo = 0;

	if(strcmp(argv[1],"GAMECARD")==0)	//SE ENVIO EL MENSAJE PARA GAMECARD
		id_mensaje = atoi(argv[4]);

	enviar_get_pokemon(conexion, id_mensaje, id_correlativo, argv[3]);
}

void enviarLocalized(int conexion, int argc, char *argv[])
{
	//nunca se va a usar esta funcion desde gameboy
	//la dejo aca para que nos sirva para despues, en el gamecard creo que se usa
	//TODO
	//esta funcion no es llamada desde despacharMensaje() ni de verificarEntrada()
	//cuando la quiera sacar solo tengo que sacar esta funcion

	if(argc < 4){
		printf("\n");
		error_show(" Para GET_POKEMON debe ingresar los argumentos con el siguiente formato:\n"
						"./gameboy [PROCESO] LOCALIZED_POKEMON [PARES DE COORDENADAS]*\n\n");
						//y el id_correlativo?..
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
		//no hago free() porque eso lo hace list_destroy_and_destroy_elements()
		//eso estendi segun el codigo de list.c
	}

	int id_mensaje = 0;
	int id_correlativo = 0;

	enviar_localized_pokemon(conexion, id_mensaje, id_correlativo, argv[3], lista_coordenadas);
	list_destroy_and_destroy_elements(lista_coordenadas, free);

}

