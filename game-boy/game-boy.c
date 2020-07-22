#include "game-boy.h"

bool imprimir_con_printf = true; //si se pone false
//no se imprimen, con printf, los mensajes recibidos en modo suscriptor
//esto es util si solo usamos logs obligatorios
//ninguna funcion toca esta variable, solo nosotros tocamos esta variable "a mano";

int main(int argc, char *argv[]) {

	int conexion = 0;
	t_config* config = leer_config("../game-boy.config");
	t_log* logger = generar_logger(config, "game-boy");

	//verifico que lo ingresado por consola sea correcto
	verificar_Entrada(argc, argv);

	if(strcmp(argv[1],"SUSCRIPTOR")==0){	// lo de aca se ejecuta si estamos en modo SUSCRIPTOR

		conexion = iniciar_conexion_como_cliente("BROKER", config);

		if(conexion <= 0) imprimir_error_y_terminar_programa("Error de conexion con BROKER");

		char *orden_de_suscripcion = string_from_format("SUBSCRIBE_%s", argv[2]);
		log_info(logger, "Se realizo una conexion con BROKER, para enviar el mensaje %s", orden_de_suscripcion);
		free(orden_de_suscripcion);

		iniciar_modo_suscriptor(conexion, logger, argv[2], atoi(argv[3]));

	}else{	// esto se ejecuta si estamos en MODO NORMAL / MODO NO SUSCRIPTOR
		conexion = iniciar_conexion_como_cliente(argv[1], config);

		if(conexion <= 0) imprimir_error_y_terminar_programa("Error de conexion");

		log_info(logger, "Se realizo una conexion con %s, para enviar el mensaje %s", argv[1], argv[2]);

		//se interpretan los argumentos ingresados por consola y se envia el mensaje correspondiente
		int estado = despachar_Mensaje(conexion, argv);
		if(estado <= 0) imprimir_error_y_terminar_programa("Error al enviar mensaje al BROKER");

		int id_mensaje_enviado = esperar_id_mensaje_enviado();	//EL GAMEBOY NO HACE NADA CON ESTO
		//SE VA A QUEDAR BLOQUEADO SI EL BROKER SE CAE...PERO NO CREO QUE PASE JUSTO CUANDO LE ENVIAMOS UN MENSAJE
		if(id_mensaje_enviado <= 0) imprimir_error_y_terminar_programa("Error al recibir id_mensaje_enviado del BROKER");
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
	if(enviar_mensaje_de_suscripcion(conexion, codigo_suscripcion, 0) <= 0)
		imprimir_error_y_terminar_programa("No se pudo enviar mensaje de suscripcion a BROKER");

	//HACE FALTA RECIBIR ACK DEL MENSAJE DE SUSCRIPCION?? TODO

	log_info(logger, "Se realizo una suscripcion a la cola de mensajes %s", cola_a_suscribirse);

	//este hilo cierra el socket cuando se acabe el tiempo de suscripcion
	iniciar_hilo_para_desuscripcion(tiempo_suscripcion, conexion, codigo_desuscripcion);

	int id_correlativo = 0;		//EL GAMEBOY NO USA NINGUNO DE LOS 2 IDs
	int id_mensaje_recibido = 0;

	while(true)
	{
		//queda bloqueado hasta que el gameboy recibe un mensaje,
		void* mensaje = recibir_mensaje_como_cliente(conexion, &id_correlativo, &id_mensaje_recibido);

		//si se recibe el mensaje de error (que se genera si se acaba el tiempo de suscripcion), se sale de este while(true)
		if(mensaje == NULL)break;

		//si no hay error, se envia ACK al BROKER
		if(enviar_ack(conexion, id_mensaje_recibido) <= 0)
			imprimir_error_y_terminar_programa("No se pudo enviar ACK al BROKER del mensaje recibido ");
		//COMO EL ENUNCIADO NO ME PIDE QUE INTENTE RECONECTAR, TIRO ERROR

		//se imprime por pantalla el mensaje recibido (si imprimir_con_printf == true) y retorna un String con el mensaje a loguear.
		imprimir_mensaje_recibido(mensaje, codigo_suscripcion);	//COMENTAR PARA LAS PRUEBAS ?? TODO O DEBERIA DESCTIVAR LOS LOGS POR PANTALLA?

		char* mensaje_para_loguear = generar_mensaje_para_loggear(mensaje, codigo_suscripcion);

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
	if(0 != pthread_create(&thread, NULL, (void*) contador_de_tiempo, (void*)arg))
		imprimir_error_y_terminar_programa("No se pudo crear un hilo de gameboy");

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
	char* mensaje_para_loguear = NULL;

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

	char* mensaje_para_loguear = string_from_format("NEW_POKEMON %s %i %i %i",
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

	char* mensaje_para_loguear = string_from_format("APPEARED_POKEMON %s %i %i",
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

	char* mensaje_para_loguear = string_from_format("CATCH_POKEMON %s %i %i",
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

	char* mensaje_para_loguear = string_from_format("CAUGHT_POKEMON %i", (char*) mensaje->resultado);

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

	char* mensaje_para_loguear = string_from_format("GET_POKEMON %s", (char*) mensaje->nombre);

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

	char* mensaje_para_loguear = string_from_format("LOCALIZED_POKEMON %s %i", (char*) mensaje->nombre, (mensaje->coordenadas->elements_count)/2);

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

		string_append_with_format(&mensaje_para_loguear, " %i %i", posx, posy);
	}
	list_destroy_and_destroy_elements(mensaje->coordenadas, free);
	free(mensaje->nombre);
	free(mensaje);

	return mensaje_para_loguear;
}

int despachar_Mensaje(int conexion, char *argv[]){

	int estado = 0;

	if(strcmp(argv[2],"NEW_POKEMON")==0) estado = despachar_New(conexion, argv);

	if(strcmp(argv[2],"APPEARED_POKEMON")==0) estado = despachar_Appeared(conexion, argv);

	if(strcmp(argv[2],"CATCH_POKEMON")==0) estado = despachar_Catch(conexion, argv);

	if(strcmp(argv[2],"CAUGHT_POKEMON")==0) estado = despachar_Caught(conexion, argv);

	if(strcmp(argv[2],"GET_POKEMON")==0) estado = despachar_Get(conexion, argv);

	return estado;
}

int despachar_New(int conexion, char *argv[])
{
	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);
	int cantidad = atoi(argv[6]);
	int id_mensaje = 0;
	int id_correlativo = 0;

	if(strcmp(argv[1],"GAMECARD")==0) id_mensaje = atoi(argv[7]);	//SE ENVIO EL MENSAJE PARA GAMECARD

	return generar_y_enviar_new_pokemon(conexion, id_mensaje, id_correlativo, argv[3], posx, posy, cantidad);
}

int despachar_Appeared(int conexion, char *argv[])
{
	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);
	int id_mensaje = 0;
	int id_correlativo = 0;

	if(strcmp(argv[1],"BROKER")==0) id_correlativo = atoi(argv[6]);	//SE ENVIO EL MENSAJE PARA BROKER

	return generar_y_enviar_appeared_pokemon(conexion, id_mensaje, id_correlativo, argv[3], posx, posy);
}

int despachar_Catch(int conexion, char *argv[])
{
	int posx = atoi(argv[4]);
	int posy = atoi(argv[5]);
	int id_mensaje = 0;
	int id_correlativo = 0;

	if(strcmp(argv[1],"GAMECARD")==0) id_mensaje = atoi(argv[6]);	//SE ENVIO EL MENSAJE PARA GAMECARD

	return generar_y_enviar_catch_pokemon(conexion, id_mensaje, id_correlativo, argv[3], posx, posy);
}

int despachar_Caught(int conexion, char *argv[])
{
	int id_mensaje = 0;
	int id_correlativo = atoi(argv[3]);
	int resultado = 0;

	if(strcmp(argv[4],"OK")==0) resultado = 1;

	return generar_y_enviar_caught_pokemon(conexion, id_mensaje, id_correlativo, resultado);
}

int despachar_Get(int conexion, char *argv[])
{
	int id_mensaje = 0;
	int id_correlativo = 0;

	if(strcmp(argv[1],"GAMECARD")==0) id_mensaje = atoi(argv[4]); //SE ENVIO EL MENSAJE PARA GAMECARD

	return generar_y_enviar_get_pokemon(conexion, id_mensaje, id_correlativo, argv[3]);
}

int despachar_Localized(int conexion, int argc, char *argv[])
{
	//nunca se va a usar esta funcion desde gameboy
	//la dejo aca para que nos sirva para despues, en el gamecard creo que se usa
	//TODO
	//esta funcion no es llamada desde despacharMensaje() ni de verificarEntrada()
	//cuando la quiera sacar solo tengo que sacar esta funcion

	if(argc < 4)imprimir_error_y_terminar_programa("Para GET_POKEMON debe ingresar los argumentos con el siguiente formato:\n"
				"./gameboy [PROCESO] LOCALIZED_POKEMON [PARES DE COORDENADAS]*");
		//y el id_correlativo?..

	if(argc%2 == 1)	imprimir_error_y_terminar_programa("No ingresaste bien las coordenadas. Deben ser grupos de 2 numeros");

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

	int estado = generar_y_enviar_localized_pokemon(conexion, id_mensaje, id_correlativo, argv[3], lista_coordenadas);
	list_destroy_and_destroy_elements(lista_coordenadas, free);

	return estado;
}

