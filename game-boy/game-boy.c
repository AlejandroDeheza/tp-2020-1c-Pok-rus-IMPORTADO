#include "game-boy.h"

bool imprimir_con_printf = true; //si se pone false
//no se imprimen, con printf, los mensajes recibidos en modo suscriptor
//esto es util si solo usamos logs obligatorios
//ninguna funcion toca esta variable, solo nosotros tocamos esta variable "a mano";

int main(int argc, char *argv[]) {

	int conexion = 0;
	t_log* logger = NULL;
	t_config* config = leer_config("../game-boy.config");
	iniciar_logger(&logger, config, "game-boy");

	//verifico que lo ingresado por consola sea correcto
	verificar_Entrada(argc, argv);

	if(strcmp(argv[1],"SUSCRIPTOR")==0){	// lo de aca se ejecuta si estamos en modo SUSCRIPTOR

		conexion = iniciar_conexion_como_cliente("BROKER", config);

		if(conexion <= 0){
			printf("\n");
			error_show(" Error de conexion\n\n");
			exit(-1);
		}

		char *orden_de_suscripcion = string_new();
		string_append_with_format(&orden_de_suscripcion, "SUBSCRIBE_%s", argv[2]);
		log_info(logger, "Se realizo una conexion con BROKER, para enviar el mensaje %s", orden_de_suscripcion);
		free(orden_de_suscripcion);

		iniciar_modo_suscriptor(conexion, logger, argv[2], atoi(argv[3]));

	}else{	// esto se ejecuta si estamos en MODO NORMAL / MODO NO SUSCRIPTOR
		conexion = iniciar_conexion_como_cliente(argv[1], config);

		if(conexion <= 0){
			printf("\n");
			error_show(" Error de conexion\n\n");
			exit(-1);
		}

		log_info(logger, "Se realizo una conexion con %s, para enviar el mensaje %s", argv[1], argv[2]);

		//se interpretan los argumentos ingresados por consola y se envia el mensaje correspondiente
		despachar_Mensaje(conexion, argv);
	}

	terminar_programa(conexion, logger, config);
	printf("\nEl Game-Boy finalizo correctamente.\n\n");
	return EXIT_SUCCESS;
}

void verificar_Entrada(int argc, char *argv[]){

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
			"./gameboy BROKER CAUGHT_POKEMON [ID_MENSAJE_CORRELATIVO] [OK/FAIL]\n\n");
			exit(-1);
		}

		if(strcmp(argv[2],"GET_POKEMON")==0 && argc != 4){
			error_show(" Escribiste una cantidad incorrecta de argumentos\n\n");
			error_show(" Para enviar GET_POKEMON a BROKER debe ingresar los argumentos con el siguiente formato:\n"
			"./gameboy BROKER GET_POKEMON [POKEMON]\n\n");
			exit(-1);
		}

		if(strcmp(argv[2],"CAUGHT_POKEMON")==0 && argc == 5 && strcmp(argv[4],"OK")!=0 && strcmp(argv[4],"FAIL")!=0){
			error_show(" El resultado solo puede ser OK o FAIL\n\n");
			error_show(" Para enviar CAUGHT_POKEMON a BROKER debe ingresar los argumentos con el siguiente formato:\n"
			"./gameboy BROKER CAUGHT_POKEMON [ID_MENSAJE_CORRELATIVO] [OK/FAIL]\n\n");
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

void iniciar_modo_suscriptor(int conexion, t_log* logger, char* cola_a_suscribirse, int tiempo_suscripcion)
{
	op_code codigo_suscripcion = 0;
	op_code codigo_desuscripcion = 0;
	obtener_codigos(cola_a_suscribirse, &codigo_suscripcion, &codigo_desuscripcion);

	//envio mensaje a BROKER para suscribirme a una cola
	enviar_mensaje_de_suscripcion(conexion, codigo_suscripcion);
	log_info(logger, "Se realizo una suscripcion a la cola de mensajes %s", cola_a_suscribirse);

	//este hilo cierra el socket cuando se acabe el tiempo de suscripcion
	iniciar_hilo_para_desuscripcion(tiempo_suscripcion, conexion, codigo_desuscripcion);

	while(true)
	{
		//queda bloqueado hasta que el gameboy recibe un mensaje,
		void* mensaje = recibir_mensaje_como_cliente(conexion);

		//si se recibe el mensaje de error (que se genera si se acaba el tiempo de suscripcion), se sale de este while(true)
		if(mensaje == NULL)break;

		//si no hay error, se envia ACK al BROKER
		enviar_ack(conexion, codigo_suscripcion);
		//estos ACK estan bien? revisar TODO
		//no deberia estar relacionado con el id_mensaje y el id_correlativo?

		//se imprime por pantalla el mensaje recibido (si imprimir_con_printf == true) y retorna un String con el mensaje a loguear.
		char* mensaje_para_loguear = imprimir_mensaje_recibido(mensaje, codigo_suscripcion);

		//se loguea el mensaje y vuelve a empezar el bucle
		log_info(logger, "Se recibio el mensaje <<%s>> de la cola %s", mensaje_para_loguear, cola_a_suscribirse);

		free(mensaje_para_loguear);
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

	//el hilo espera el tiempo de suscripcion
	sleep(tiempo_suscripcion);

	//se envia mensaje a BROKER para que este deje de enviarnos mensajes
	//hay que cambiar esto del lado del BROKER, no deberia ser necesario hacer esto porque el GAMEBOY podria cortarse abruptamente
	//TODO
	enviar_mensaje_de_suscripcion(conexion_con_broker, codigo_desuscripcion);

	//con esta funcion se corta la conexion, asi se corta el bucle de iniciar_modo_suscriptor()
	shutdown(conexion_con_broker, SHUT_RDWR);

	return NULL;
}

char* imprimir_mensaje_recibido(void* mensaje, op_code codigo_operacion)
{
	char* mensaje_para_loguear;

	switch (codigo_operacion)
	{
		case SUBSCRIBE_NEW_POKEMON:
			mensaje_para_loguear = imprimir_new_pokemon(mensaje);
			break;

		case SUBSCRIBE_APPEARED_POKEMON:
			mensaje_para_loguear = imprimir_appeared_pokemon(mensaje);
			break;

		case SUBSCRIBE_CATCH_POKEMON:
			mensaje_para_loguear = imprimir_catch_pokemon(mensaje);
			break;

		case SUBSCRIBE_CAUGHT_POKEMON:
			mensaje_para_loguear = imprimir_caught_pokemon(mensaje);
			break;

		case SUBSCRIBE_GET_POKEMON:
			mensaje_para_loguear = imprimir_get_pokemon(mensaje);
			break;

		case SUBSCRIBE_LOCALIZED_POKEMON:
			mensaje_para_loguear = imprimir_localized_pokemon(mensaje);
			break;

		default:
			break;
	}

	return mensaje_para_loguear;
}

char* imprimir_new_pokemon(void* mensaje_a_imprimir)
{
	t_new_pokemon* mensaje = mensaje_a_imprimir;

	if(imprimir_con_printf){

		printf("\n		NEW_POKEMON\n"
				"		nombre: %s\n"
				"		posicion x: %i\n"
				"		posicion y: %i\n"
				"		cantidad: %i\n",
		(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy, mensaje->cantidad);
	}

	char* mensaje_para_loguear = string_new();
	string_append_with_format(&mensaje_para_loguear, "NEW_POKEMON %s %i %i %i",
			(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy, mensaje->cantidad);

	free(mensaje->nombre);
	free(mensaje);

	return mensaje_para_loguear;
}

char* imprimir_appeared_pokemon(void* mensaje_a_imprimir)
{
	t_appeared_pokemon* mensaje = mensaje_a_imprimir;

	if(imprimir_con_printf){

		printf("\n		APPEARED_POKEMON\n"
				"		nombre: %s\n"
				"		posicion x: %i\n"
				"		posicion y: %i\n",
		(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);
	}

	char* mensaje_para_loguear = string_new();
	string_append_with_format(&mensaje_para_loguear, "APPEARED_POKEMON %s %i %i",
			(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);

	free(mensaje->nombre);
	free(mensaje);

	return mensaje_para_loguear;
}

char* imprimir_catch_pokemon(void* mensaje_a_imprimir)
{
	t_catch_pokemon* mensaje = mensaje_a_imprimir;

	if(imprimir_con_printf){

		printf("\n		CATCH_POKEMON\n"
				"		nombre: %s\n"
				"		posicion x: %i\n"
				"		posicion y: %i\n",
		(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);
	}

	char* mensaje_para_loguear = string_new();
	string_append_with_format(&mensaje_para_loguear, "CATCH_POKEMON %s %i %i",
			(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);

	free(mensaje->nombre);
	free(mensaje);

	return mensaje_para_loguear;
}

char* imprimir_caught_pokemon(void* mensaje_a_imprimir)
{
	t_caught_pokemon* mensaje = mensaje_a_imprimir;

	if(imprimir_con_printf){

		printf("\n		CAUGHT_POKEMON\n"
				"		resultado: %i\n",
		mensaje->resultado);
	}

	char* mensaje_para_loguear = string_new();
	string_append_with_format(&mensaje_para_loguear, "CAUGHT_POKEMON %i",
			(char*) mensaje->resultado);

	free(mensaje);

	return mensaje_para_loguear;
}

char* imprimir_get_pokemon(void* mensaje_a_imprimir)
{
	t_get_pokemon* mensaje = mensaje_a_imprimir;

	if(imprimir_con_printf){

		printf("\n		GET_POKEMON\n"
				"		nombre: %s\n",
				(char*) mensaje->nombre);
	}

	char* mensaje_para_loguear = string_new();
	string_append_with_format(&mensaje_para_loguear, "GET_POKEMON %s",
			(char*) mensaje->nombre);

	free(mensaje->nombre);
	free(mensaje);

	return mensaje_para_loguear;
}

char* imprimir_localized_pokemon(void* mensaje_a_imprimir)
{
	t_localized_pokemon* mensaje = mensaje_a_imprimir;

	if(imprimir_con_printf){

		printf("\n		LOCALIZED_POKEMON\n"
				"		nombre: %s\n",
				(char*) mensaje->nombre);
	}

	char* mensaje_para_loguear = string_new();
	string_append_with_format(&mensaje_para_loguear, "LOCALIZED_POKEMON %s %i",
			(char*) mensaje->nombre, (mensaje->coordenadas->elements_count)/2);

	for(int i = 0 ; i < mensaje->coordenadas->elements_count ; i++)
	{
		int posx = *((int*) list_get(mensaje->coordenadas, i));
		i++;
		int posy = *((int*) list_get(mensaje->coordenadas, i));

		if(imprimir_con_printf){

			printf( "		posicion x: %i\n"
					"		posicion y: %i\n\n",
					posx, posy);
		}

		string_append_with_format(&mensaje_para_loguear, " %i %i",
				posx, posy);
	}
	list_destroy_and_destroy_elements(mensaje->coordenadas, free);
	free(mensaje->nombre);
	free(mensaje);

	return mensaje_para_loguear;
}

void despachar_Mensaje(int conexion, char *argv[]){

	if(strcmp(argv[2],"NEW_POKEMON")==0){
		despachar_New(conexion, argv);
	}
	if(strcmp(argv[2],"APPEARED_POKEMON")==0){
		despachar_Appeared(conexion, argv);
	}
	if(strcmp(argv[2],"CATCH_POKEMON")==0){
		despachar_Catch(conexion, argv);
	}
	if(strcmp(argv[2],"CAUGHT_POKEMON")==0){
		despachar_Caught(conexion, argv);
	}
	if(strcmp(argv[2],"GET_POKEMON")==0){
		despachar_Get(conexion, argv);
	}
}

void despachar_New(int conexion, char *argv[])
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

void despachar_Appeared(int conexion, char *argv[])
{
	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);
	int id_mensaje = 0;
	int id_correlativo = 0;

	if(strcmp(argv[1],"BROKER")==0)	 	//SE ENVIO EL MENSAJE PARA BROKER
		id_correlativo = atoi(argv[6]);

	enviar_appeared_pokemon(conexion, id_mensaje, id_correlativo, argv[3], posx, posy);
}

void despachar_Catch(int conexion, char *argv[])
{
	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);
	int id_mensaje = 0;
	int id_correlativo = 0;

	if(strcmp(argv[1],"GAMECARD")==0)	//SE ENVIO EL MENSAJE PARA GAMECARD
		id_mensaje = atoi(argv[6]);

	enviar_catch_pokemon(conexion, id_mensaje, id_correlativo, argv[3], posx, posy);
}

void despachar_Caught(int conexion, char *argv[])
{
	int id_mensaje = 0;
	int id_correlativo = atoi(argv[3]);
	int resultado = 0;

	if(strcmp(argv[4],"OK")==0)
	{
		resultado = 1;
	}

	enviar_caught_pokemon(conexion, id_mensaje, id_correlativo, resultado);
}

void despachar_Get(int conexion, char *argv[])
{
	int id_mensaje = 0;
	int id_correlativo = 0;

	if(strcmp(argv[1],"GAMECARD")==0)	//SE ENVIO EL MENSAJE PARA GAMECARD
		id_mensaje = atoi(argv[4]);

	enviar_get_pokemon(conexion, id_mensaje, id_correlativo, argv[3]);
}

void despachar_Localized(int conexion, int argc, char *argv[])
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

