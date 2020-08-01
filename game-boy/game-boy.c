#include "game-boy.h"

int main(int argc, char *argv[]) {

	signal(SIGINT, &ejecutar_antes_de_terminar);

	CONFIG = leer_config("../game-boy.config");
	LOGGER = generar_logger(CONFIG, "game-boy");
	ID_PROCESOS_TP = asignar_string_property(CONFIG, "ID_PROCESOS_TP");

	//verifico que lo ingresado por consola sea correcto
	verificar_Entrada(argc, argv);

	if(strcmp(argv[1],"SUSCRIPTOR")==0){	// lo de aca se ejecuta si estamos en modo SUSCRIPTOR

		char* ip = NULL;
		char* puerto = NULL;

		int retorno_ip_puerto = leer_ip_y_puerto(&ip, &puerto, CONFIG, "BROKER");

		if(retorno_ip_puerto == -1)
			imprimir_error_y_terminar_programa_perzonalizado("No se encontro IP_BROKER o PUERTO_BROKER en el archivo de configuracion", NULL, NULL);

		CONEXION = iniciar_conexion_como_cliente(ip, puerto);

		if(CONEXION == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error en socket() en crear_socket_como_cliente", NULL, NULL);

		if(CONEXION == 0) imprimir_error_y_terminar_programa_perzonalizado("Error de conexion con BROKER", NULL, NULL);

		log_info(LOGGER, "Se realizo una conexion con BROKER, para suscribirse a la cola de mensajes %s", argv[2]);

		int retorno_identificacion = enviar_identificacion_general(CONEXION, ID_PROCESOS_TP, NULL);

		if(retorno_identificacion == 0)
			imprimir_error_y_terminar_programa_perzonalizado("Error en enviar_identificacion_general()", finalizar_gameboy, NULL);

		if(retorno_identificacion == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_identificacion_general()", finalizar_gameboy, NULL);

		iniciar_modo_suscriptor(CONEXION, argv[2], atoi(argv[3]));

	}
	else
	{	// esto se ejecuta si estamos en MODO NORMAL / MODO NO SUSCRIPTOR
		char* ip = NULL;
		char* puerto = NULL;

		int retorno_ip_puerto = leer_ip_y_puerto(&ip, &puerto, CONFIG, argv[1]);

		if(retorno_ip_puerto == -1)
			imprimir_error_y_terminar_programa_perzonalizado("No se encontro IP_BROKER o PUERTO_BROKER en el archivo de configuracion", NULL, NULL);

		CONEXION = iniciar_conexion_como_cliente(ip, puerto);
		if(CONEXION == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error en socket() en crear_socket_como_cliente", NULL, NULL);

		if(CONEXION == 0) imprimir_error_y_terminar_programa_perzonalizado("Error de conexion", NULL, NULL);

		log_info(LOGGER, "Se realizo una conexion con %s, para enviar un mensaje %s", argv[1], argv[2]);

		int retorno_identificacion = enviar_identificacion_general(CONEXION, ID_PROCESOS_TP, NULL);

		if(retorno_identificacion == 0)
			imprimir_error_y_terminar_programa_perzonalizado("Error en enviar_identificacion_general()", finalizar_gameboy, NULL);

		if(retorno_identificacion == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_identificacion_general()", finalizar_gameboy, NULL);

		//se interpretan los argumentos ingresados por consola y se envia el mensaje correspondiente
		int estado = despachar_Mensaje(CONEXION, argv);
		if(estado == 0) imprimir_error_y_terminar_programa_perzonalizado("Error al enviar mensaje al BROKER", finalizar_gameboy, NULL);
		if(estado == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_mensaje_como_cliente()", finalizar_gameboy, NULL);

		if(strcmp(argv[1],"BROKER") == 0){
			int id_mensaje_enviado = esperar_id_mensaje_enviado(CONEXION);	//EL GAMEBOY NO HACE NADA CON ESTO
			//SE VA A QUEDAR BLOQUEADO SI EL BROKER SE CAE...PERO NO CREO QUE PASE JUSTO CUANDO LE ENVIAMOS UN MENSAJE
			if(id_mensaje_enviado == 0)
				imprimir_error_y_terminar_programa_perzonalizado("Error al recibir id_mensaje_enviado del BROKER", finalizar_gameboy, NULL);

			if(id_mensaje_enviado == -1)
				imprimir_error_y_terminar_programa_perzonalizado("Error al usar recv() en esperar_id_mensaje_enviado()", finalizar_gameboy, NULL);
		}
	}

	finalizar_gameboy();
	printf("\nEl Game-Boy finalizo correctamente.\n\n");
	return EXIT_SUCCESS;
}

void ejecutar_antes_de_terminar(int numero_senial)
{
	log_info(LOGGER, "Se recibio la senial : %i  -- terminando programa", numero_senial);

	finalizar_gameboy();

	exit(0);
}

void finalizar_gameboy()
{
	terminar_programa(CONEXION, LOGGER, CONFIG, NULL);
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

void iniciar_modo_suscriptor(int conexion, char* cola_a_suscribirse, int tiempo_suscripcion)
{
	op_code codigo_suscripcion = obtener_codigo_suscripcion(cola_a_suscribirse);

	ID_MANUAL_DEL_PROCESO = asignar_int_property(CONFIG, "ID_MANUAL_DEL_PROCESO");

	//envio mensaje a BROKER para suscribirme a una cola. COMO EN ID_MANUAL_DEL_PROCESO LE PONGO 0, EL BROKER ME DESUSCRIBE DE LA COLA DE MENSAJES
	//CUANDO SE DA CUENTA QUE EL SOCKET YA NO SIRVE
	int estado_envio = enviar_mensaje_de_suscripcion(conexion, codigo_suscripcion, ID_MANUAL_DEL_PROCESO, NULL);

	if(estado_envio == 0)
		imprimir_error_y_terminar_programa_perzonalizado("No se pudo enviar mensaje de suscripcion a BROKER", finalizar_gameboy, NULL);

	if(estado_envio == -1)
		imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_mensaje_de_suscripcion()", finalizar_gameboy, NULL);

	log_info(LOGGER, "Se realizo una suscripcion a la cola de mensajes %s", cola_a_suscribirse);

	//este hilo cierra el socket cuando se acabe el tiempo de suscripcion
	iniciar_hilo_para_desuscripcion(tiempo_suscripcion, conexion);

	int id_correlativo = 0;		//EL GAMEBOY NO USA NINGUNO DE LOS 2 IDs
	int id_mensaje_recibido = 0;

	op_code codigo_operacion_recibido = 0;

	while(true)
	{
		//queda bloqueado hasta que el gameboy recibe un mensaje,
		void* mensaje = recibir_mensaje_por_socket(&codigo_operacion_recibido, conexion, &id_correlativo, &id_mensaje_recibido, finalizar_gameboy, NULL);

		//si se recibe el mensaje de error (que se genera si se acaba el tiempo de suscripcion), se sale de este while(true)
		if(mensaje == NULL)break;

		//si no hay error, se envia ACK al BROKER
		int estado_ack = enviar_ack(conexion, id_mensaje_recibido);

		if(estado_ack == 0)
			imprimir_error_y_terminar_programa_perzonalizado("No se pudo enviar ACK al BROKER del mensaje recibido", finalizar_gameboy, NULL);
		//COMO EL ENUNCIADO NO ME PIDE QUE INTENTE RECONECTAR, TIRO ERROR

		if(estado_ack == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_ack()", finalizar_gameboy, NULL);

		//se imprime por pantalla el mensaje recibido
		imprimir_mensaje_recibido(mensaje, codigo_suscripcion);	//COMENTAR PARA LAS PRUEBAS ?? TODO O DEBERIA DESCTIVAR LOS LOGS POR PANTALLA?

		char* mensaje_para_loguear = generar_mensaje_para_loggear(mensaje, codigo_suscripcion);

		//se loguea el mensaje y vuelve a empezar el bucle
		log_info(LOGGER, "Se recibio el mensaje <<%s>> de la cola %s", mensaje_para_loguear, cola_a_suscribirse);

		free(mensaje_para_loguear);
	}
}

op_code obtener_codigo_suscripcion(char* cola_a_suscribirse)
{
	if(strcmp(cola_a_suscribirse,"NEW_POKEMON")==0) return SUBSCRIBE_NEW_POKEMON;

	if(strcmp(cola_a_suscribirse,"APPEARED_POKEMON")==0) return SUBSCRIBE_APPEARED_POKEMON;

	if(strcmp(cola_a_suscribirse,"CATCH_POKEMON")==0) return SUBSCRIBE_CATCH_POKEMON;

	if(strcmp(cola_a_suscribirse,"CAUGHT_POKEMON")==0) return SUBSCRIBE_CAUGHT_POKEMON;

	if(strcmp(cola_a_suscribirse,"GET_POKEMON")==0) return SUBSCRIBE_GET_POKEMON;

	return SUBSCRIBE_LOCALIZED_POKEMON;	//SI NO SE CUMPLE NINGUNO ANTERIOR, DEBERIA RETORNAR ESTO
}

void iniciar_hilo_para_desuscripcion(int tiempo_suscripcion, int conexion_con_broker)
{
	argumentos_contador_de_tiempo* arg = malloc(sizeof(argumentos_contador_de_tiempo));
	arg->tiempo_suscripcion = tiempo_suscripcion;
	arg->conexion_con_broker = conexion_con_broker;

	pthread_t thread;
	if(0 != pthread_create(&thread, NULL, (void*) contador_de_tiempo, (void*)arg))
		imprimir_error_y_terminar_programa_perzonalizado("No se pudo crear un hilo de gameboy", finalizar_gameboy, NULL);

	pthread_detach(thread);
}

void* contador_de_tiempo(void* argumentos)
{
	argumentos_contador_de_tiempo* args = argumentos;

	int tiempo_suscripcion = args->tiempo_suscripcion;
	int conexion_con_broker = args->conexion_con_broker;
	free(args);

	sleep(tiempo_suscripcion);

	//con esta funcion se corta la conexion, asi se corta el bucle de iniciar_modo_suscriptor()
	shutdown(conexion_con_broker, SHUT_RDWR);

	return NULL;
}

void imprimir_mensaje_recibido(void* mensaje, op_code codigo_operacion)
{
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
}

void imprimir_new_pokemon(void* mensaje_a_imprimir)
{
	t_new_pokemon* mensaje = mensaje_a_imprimir;

	printf("\n		NEW_POKEMON\n"
			"		nombre: %s\n"
			"		posicion x: %i\n"
			"		posicion y: %i\n"
			"		cantidad: %i\n",
	(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy, mensaje->cantidad);
}

void imprimir_appeared_pokemon(void* mensaje_a_imprimir)
{
	t_appeared_pokemon* mensaje = mensaje_a_imprimir;


	printf("\n		APPEARED_POKEMON\n"
			"		nombre: %s\n"
			"		posicion x: %i\n"
			"		posicion y: %i\n",
	(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);
}

void imprimir_catch_pokemon(void* mensaje_a_imprimir)
{
	t_catch_pokemon* mensaje = mensaje_a_imprimir;

	printf("\n		CATCH_POKEMON\n"
			"		nombre: %s\n"
			"		posicion x: %i\n"
			"		posicion y: %i\n",
	(char*) mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);
}

void imprimir_caught_pokemon(void* mensaje_a_imprimir)
{
	t_caught_pokemon* mensaje = mensaje_a_imprimir;

	printf("\n		CAUGHT_POKEMON\n"
			"		resultado: %i\n",
	mensaje->resultado);
}

void imprimir_get_pokemon(void* mensaje_a_imprimir)
{
	t_get_pokemon* mensaje = mensaje_a_imprimir;

	printf("\n		GET_POKEMON\n"
			"		nombre: %s\n",
			(char*) mensaje->nombre);
}

void imprimir_localized_pokemon(void* mensaje_a_imprimir)
{
	t_localized_pokemon* mensaje = mensaje_a_imprimir;

	printf("\n		LOCALIZED_POKEMON\n"
			"		nombre: %s\n",
			(char*) mensaje->nombre);

	for(int i = 0 ; i < mensaje->coordenadas->elements_count ; i++)
	{
		t_coordenadas* coordenadas = list_get(mensaje->coordenadas, i);

		int posx = coordenadas->posx;
		int posy = coordenadas->posy;

		printf( "		posicion x: %i\n"
				"		posicion y: %i\n\n",
				posx, posy);
	}
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

