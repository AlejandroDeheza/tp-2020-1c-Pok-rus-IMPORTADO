#include "game-card.h"

int main(int argc, char *argv[]) {

	signal(SIGINT, &ejecutar_antes_de_terminar);



	verificar_e_interpretar_entrada(argc, argv);

	iniciar_variables_globales_gamecard();

	iniciar_file_system();

	establecer_comunicaciones_gamecard();



	finalizar_gamecard();

	return EXIT_SUCCESS;
}

void ejecutar_antes_de_terminar(int numero_senial)
{
	pthread_mutex_lock(MUTEX_LOGGER);
	log_info(LOGGER, "Se recibio la senial : %i  -- terminando programa", numero_senial);
	pthread_mutex_unlock(MUTEX_LOGGER);

	finalizar_gamecard();

	exit(0);
}

void finalizar_gamecard()
{
	pthread_mutex_lock(MUTEX_BITMAP);
	pthread_mutex_lock(MUTEX_LOGGER);
    if(msync(BITMAP->bitarray, BITMAP->size, MS_SYNC) == -1) log_error(LOGGER, "Ocurrio un error al usar mysinc()");

    if(munmap(BITMAP->bitarray, BITMAP->size) == -1) log_error(LOGGER, "Ocurrio un error al usar munmap()");
	pthread_mutex_unlock(MUTEX_LOGGER);

	bitarray_destroy(BITMAP);
	pthread_mutex_unlock(MUTEX_BITMAP);

	pthread_mutex_lock(MUTEX_METADATA_METADATA_BIN);
	config_destroy(METADATA_METADATA_BIN);
	pthread_mutex_unlock(MUTEX_METADATA_METADATA_BIN);

	pthread_mutex_lock(MUTEX_LOGGER);
	pthread_mutex_lock(MUTEX_CONFIG);
	terminar_programa(0, LOGGER, CONFIG, NULL);
	pthread_mutex_unlock(MUTEX_CONFIG);
	pthread_mutex_unlock(MUTEX_LOGGER);

	pthread_mutex_lock(MUTEX_DICCIONARIO);
	dictionary_destroy_and_destroy_elements(DICCIONARIO_CON_MUTEX, eliminador_de_mutex_en_diccionario);
	pthread_mutex_unlock(MUTEX_DICCIONARIO);

	pthread_mutex_destroy(MUTEX_CONFIG);
	pthread_mutex_destroy(MUTEX_LOGGER);
	pthread_mutex_destroy(MUTEX_METADATA_METADATA_BIN);
	pthread_mutex_destroy(MUTEX_BITMAP);
	pthread_mutex_destroy(MUTEX_ID_PROCESOS_TP);
	pthread_mutex_destroy(MUTEX_ID_MANUAL_DEL_PROCESO);
	pthread_mutex_destroy(MUTEX_DICCIONARIO);
	pthread_mutex_destroy(MUTEX_CONSULTA_POKEMON);
}

void eliminador_de_mutex_en_diccionario(void* argumento)
{
	pthread_mutex_t* mutex = argumento;
	pthread_mutex_destroy(mutex);
}

void verificar_e_interpretar_entrada(int argc, char *argv[])
{
	if(argc > 2 ){
		error_show(" Ingresaste mas argumentos de los necesarios.\n"
				"Solo se puede ingresar un argumento, el ID_MANUAL_DEL_PROCESO, que es opcional.\n"
				"Si no se ingresa, se setea en 0\n\n");
		exit(-1);
	}

	if(argc == 2) ID_MANUAL_DEL_PROCESO = atoi(argv[1]);
}

void iniciar_variables_globales_gamecard()
{
	CONFIG = leer_config("../game-card.config");
	LOGGER = generar_logger(CONFIG, "game-card");
	ID_PROCESOS_TP = asignar_string_property(CONFIG, "ID_PROCESOS_TP");
	DICCIONARIO_CON_MUTEX = dictionary_create();

	MUTEX_CONFIG = malloc(sizeof(pthread_mutex_t));
	MUTEX_LOGGER = malloc(sizeof(pthread_mutex_t));
	MUTEX_METADATA_METADATA_BIN = malloc(sizeof(pthread_mutex_t));
	MUTEX_BITMAP = malloc(sizeof(pthread_mutex_t));
	MUTEX_ID_PROCESOS_TP = malloc(sizeof(pthread_mutex_t));
	MUTEX_ID_MANUAL_DEL_PROCESO = malloc(sizeof(pthread_mutex_t));
	MUTEX_DICCIONARIO = malloc(sizeof(pthread_mutex_t));
	MUTEX_CONSULTA_POKEMON = malloc(sizeof(pthread_mutex_t));

	pthread_mutex_init(MUTEX_CONFIG, NULL);
	pthread_mutex_init(MUTEX_LOGGER, NULL);
	pthread_mutex_init(MUTEX_METADATA_METADATA_BIN, NULL);
	pthread_mutex_init(MUTEX_BITMAP, NULL);
	pthread_mutex_init(MUTEX_ID_PROCESOS_TP, NULL);
	pthread_mutex_init(MUTEX_ID_MANUAL_DEL_PROCESO, NULL);
	pthread_mutex_init(MUTEX_DICCIONARIO, NULL);
	pthread_mutex_init(MUTEX_CONSULTA_POKEMON, NULL);
}

void iniciar_file_system()
{
	char* punto_montaje_file_system = asignar_string_property(CONFIG, "PUNTO_MONTAJE_TALLGRASS");
	if(punto_montaje_file_system == NULL) imprimir_error_y_terminar_programa("Parece que PUNTO_MONTAJE_TALLGRASS no esta en game-card.config");

	generar_estructura_file_system_si_hace_falta(punto_montaje_file_system);

	METADATA_METADATA_BIN = config_para_file_system(punto_montaje_file_system, "/Metadata/Metadata.bin");

	int cant_max_bloques = asignar_int_property(METADATA_METADATA_BIN, "BLOCKS");

	char* path_metadata_bitmap_bin = string_from_format("%s/Metadata/Bitmap.bin", punto_montaje_file_system);

	BITMAP = mapear_bitmap_en_memoria(path_metadata_bitmap_bin, cant_max_bloques/8);

	log_info(LOGGER, "Se realizo con exito el mapeado en memoria de : %s", path_metadata_bitmap_bin);
	free(path_metadata_bitmap_bin);

	log_info(LOGGER, "El File System se inicio correctamente");
}

void generar_estructura_file_system_si_hace_falta(char* punto_montaje_file_system)
{
	/* SE CREA CARPETA DE PUNTO DE MONTAJE */
	crear_carpeta_si_no_existe(punto_montaje_file_system);

	/* SE CREA CARPETA METADATA */
	char* path_metadata = string_from_format("%s/Metadata", punto_montaje_file_system);
	crear_carpeta_si_no_existe(path_metadata);

	/* SE CREA ARCHIVO METADATA/METADATA.BIN */
	string_append_with_format(&path_metadata, "/Metadata.bin");

	char* block_size = asignar_string_property(CONFIG, "DEFAULT_BLOCK_SIZE");
	char* blocks = asignar_string_property(CONFIG, "DEFAULT_BLOCKS");
	char* magic_number = asignar_string_property(CONFIG, "DEFAULT_MAGIC_NUMBER");
	char* datos_metadata_metadata_bin = string_from_format("BLOCK_SIZE=%s\nBLOCKS=%s\nMAGIC_NUMBER=%s", block_size, blocks, magic_number);

	escribir_y_cerrar_archivo_si_no_existe(path_metadata, datos_metadata_metadata_bin, strlen(datos_metadata_metadata_bin));
	free(datos_metadata_metadata_bin);
	free(path_metadata);

	/* SE CREA CARPETA BLOCKS */
	char* path_blocks = string_from_format("%s/Blocks", punto_montaje_file_system);
	crear_carpeta_si_no_existe(path_blocks);
	free(path_blocks);

	/* SE CREA ARCHIVO METADATA/BITMAP.BIN ---- ADEMAS ---- SE CREAN ARCHIVOS BLOCKS/NUMERO.BIN */
	t_config* config_metadata_metadata_bin = config_para_file_system(punto_montaje_file_system, "/Metadata/Metadata.bin");
	int cantidad_bloques = asignar_int_property(config_metadata_metadata_bin, "BLOCKS");
	config_destroy(config_metadata_metadata_bin);
	int cantidad_bytes = cantidad_bloques/ 8;

	char* path_metadata_bitmap_bin = string_from_format("%s/Metadata/Bitmap.bin", punto_montaje_file_system);
	struct stat datos_bitmap_bin;
	t_bitarray* bitarray = NULL;

	if(stat(path_metadata_bitmap_bin, &datos_bitmap_bin) == 0)
	{	// ESTO SE EJECUTA SI EL ARCHIVO EN path_metadata_bitmap_bin EXISTE
		//N0 IMPORTA SI EL BITMAP.BIN TIENE DATOS ADENTRO O ESTA VACIO
		bitarray = mapear_bitmap_en_memoria(path_metadata_bitmap_bin, cantidad_bytes);
		generar_bloques_bin_que_hagan_falta(punto_montaje_file_system, cantidad_bloques, bitarray);

	    if(msync(bitarray->bitarray, bitarray->size, MS_SYNC) == -1) imprimir_error_y_terminar_programa("Ocurrio un error al usar mysinc() en generar_estructura_file_system_si_hace_falta");

		munmap(bitarray->bitarray, bitarray->size);

	}else{
		char* bytes = calloc(cantidad_bytes, sizeof(char));
		bitarray = bitarray_create_with_mode(bytes, cantidad_bytes, LSB_FIRST);
		generar_bloques_bin_que_hagan_falta(punto_montaje_file_system, cantidad_bloques, bitarray);
		sobrescribir_y_cerrar_archivo(path_metadata_bitmap_bin, bitarray->bitarray, cantidad_bytes);
		free(bytes);
	}
	free(path_metadata_bitmap_bin);
	bitarray_destroy(bitarray);

	/* SE CREA CARPETA FILES */
	char* path_files = string_from_format("%s/Files", punto_montaje_file_system);
	crear_carpeta_si_no_existe(path_files);

	/* SE CREA ARCHIVO FILES/METADATA.BIN */
	char* path_files_metadata_bin = string_from_format("%s/Metadata.bin", path_files);
	escribir_y_cerrar_archivo_si_no_existe(path_files_metadata_bin, "DIRECTORY=Y", strlen("DIRECTORY=Y"));
	free(path_files_metadata_bin);

	/* SE GENERAN MUTEX EN EL DICCIONARIO */
	DIR *dir;
	struct dirent *ent;
	dir = opendir (path_files);
	free(path_files);

	if (dir == NULL) imprimir_error_y_terminar_programa("No se pudo abrir un directorio en generar_estructura_file_system_si_hace_falta()");

	while ((ent = readdir (dir)) != NULL)
	{
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, "Metadata.bin") == 0)
	    {
			continue;
	    }

		char* metadata_bin_archivo_pokemon = string_from_format("%s/Files/%s/Metadata.bin", punto_montaje_file_system, ent->d_name);
		struct stat datos_metadata_bin_archivo_pokemon;

		if(stat(metadata_bin_archivo_pokemon, &datos_metadata_bin_archivo_pokemon) == -1)
		{
			continue;
		}
		else
		{
			if(datos_metadata_bin_archivo_pokemon.st_size != 0)
			{
				pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
				pthread_mutex_init(mutex, NULL);
				dictionary_put(DICCIONARIO_CON_MUTEX, metadata_bin_archivo_pokemon, mutex);
			}
		}

		free(metadata_bin_archivo_pokemon);
	}

	closedir(dir);
}

t_bitarray* mapear_bitmap_en_memoria(char* archivo, size_t size_memoria_a_mapear){

	int fd = open(archivo, O_RDWR);
	if (fd == -1)imprimir_error_y_terminar_programa("No se pudo abrir el archivo en mapear_bitmap_en_memoria()");

	if (ftruncate(fd, size_memoria_a_mapear) == -1)imprimir_error_y_terminar_programa("No se pudo truncar el archivo en mapear_bitmap_en_memoria()");

	void* memoria_mapeada = mmap(NULL, size_memoria_a_mapear, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (close(fd) == -1)imprimir_error_y_terminar_programa("No se pudo cerrar el archivo en mapear_bitmap_en_memoria()");

	if (memoria_mapeada == MAP_FAILED) imprimir_error_y_terminar_programa("No se pudo realizar el mapeo en memoria del BITMAP");

	return bitarray_create_with_mode((char*) memoria_mapeada, size_memoria_a_mapear, LSB_FIRST);
}

void generar_bloques_bin_que_hagan_falta(char* punto_montaje_file_system, int cantidad_bloques_del_fs, t_bitarray* bitarray)
{
	for(int i = 1; i < (cantidad_bloques_del_fs + 1); i++){

		char* un_path_bloque = string_from_format("%s/Blocks/%i.bin", punto_montaje_file_system, i);

		struct stat datos_bloque_bin;

		if(stat(un_path_bloque, &datos_bloque_bin) == -1){
			sobrescribir_y_cerrar_archivo(un_path_bloque, "", 0);
		}else{

			if(datos_bloque_bin.st_size != 0) bitarray_set_bit(bitarray, i - 1);
		}
		free(un_path_bloque);
	}
}

t_config* config_para_file_system(char* punto_montaje_file_system, char* path_archivo)
{
	char* archivo = string_from_format("%s%s", punto_montaje_file_system, path_archivo);
	t_config* config = leer_config(archivo);
	free(archivo);

	return config;
}

void crear_carpeta_si_no_existe(char* path_carpeta_con_nombre)
{
	DIR* carpeta = opendir(path_carpeta_con_nombre);

	if(carpeta == NULL)
	{
		mkdir(path_carpeta_con_nombre, 0777);
	}
	else
	{
		closedir(carpeta);
	}
}

void escribir_y_cerrar_archivo_si_no_existe(char* path_archivo_con_nombre, char* datos_a_grabar, int tamanio_datos_a_grabar)
{
	FILE* archivo = fopen(path_archivo_con_nombre, "rb");

	if(archivo == NULL)
	{
		sobrescribir_y_cerrar_archivo(path_archivo_con_nombre, datos_a_grabar, tamanio_datos_a_grabar);
	}
	else
	{
		fclose(archivo);
	}
}

void sobrescribir_y_cerrar_archivo(char* path_archivo_con_nombre, char* datos_a_grabar, int tamanio_datos_a_grabar)
{
	FILE* archivo = fopen(path_archivo_con_nombre, "wb");

	if(archivo == NULL)
	{
		char* mensaje = string_from_format("No se pudo crear archivo %s", path_archivo_con_nombre);
		imprimir_error_y_terminar_programa_perzonalizado(mensaje, finalizar_gamecard, MUTEX_LOGGER);
		free(mensaje); //ESTO NUNCA SE EJECUTA...
	}

	//fprintf(metadata_bin, "%s", datos_metadata_bin);
	fwrite(datos_a_grabar, tamanio_datos_a_grabar, 1, archivo);
	fflush(archivo);

	if (fclose(archivo) != 0)
		imprimir_error_y_terminar_programa_perzonalizado("No se pudo cerrar el archivo en sobrescribir_y_cerrar_archivo()",
				finalizar_gamecard, MUTEX_LOGGER);
}

void establecer_comunicaciones_gamecard()
{
	pthread_t thread_servidor = iniciar_hilo_para_recibir_conexiones();

	pthread_t thread_new_appeared = iniciar_hilo_para_comunicarse_con_broker("NEW_POKEMON", SUBSCRIBE_NEW_POKEMON);

	pthread_t thread_catch_caught = iniciar_hilo_para_comunicarse_con_broker("CATCH_POKEMON", SUBSCRIBE_CATCH_POKEMON);

	pthread_t thread_get_localized = iniciar_hilo_para_comunicarse_con_broker("GET_POKEMON", SUBSCRIBE_GET_POKEMON);

	pthread_join(thread_servidor, NULL);
	pthread_join(thread_new_appeared, NULL);
	pthread_join(thread_catch_caught, NULL);
	pthread_join(thread_get_localized, NULL);
}

pthread_t iniciar_hilo_para_recibir_conexiones()
{
	pthread_t thread;
	if(0 != pthread_create(&thread, NULL, recibir_conexiones, NULL))
		imprimir_error_y_terminar_programa_perzonalizado("No se pudo crear hilo recibir_conexiones()", finalizar_gamecard, MUTEX_LOGGER);

	return thread;
}

void* recibir_conexiones()
{
	pthread_mutex_lock(MUTEX_CONFIG);
	char* ip = asignar_string_property(CONFIG, "IP_GAMECARD");
	char* puerto = asignar_string_property(CONFIG, "PUERTO_GAMECARD");
	pthread_mutex_unlock(MUTEX_CONFIG);

	int socket_servidor = crear_socket_para_escuchar(ip, puerto);

    while(true)
    {
    	int socket_cliente = aceptar_una_conexion(socket_servidor);

    	int id_correlativo = 0;
		int id_mensaje_recibido = 0; 	//EL GAMECARD SOLO USA ID MENSAJE
		op_code codigo_operacion_recibido = 0;

		//IDENTIFICA Y RECHAZA PROCESOS QUE NO ESTA ESPERANDO
		if(es_un_proceso_esperado(socket_cliente, ID_PROCESOS_TP, MUTEX_ID_PROCESOS_TP, finalizar_gamecard, MUTEX_LOGGER) == false) continue;

		//queda bloqueado hasta que el gamecard recibe un mensaje del gameboy
		void* mensaje = recibir_mensaje_por_socket(&codigo_operacion_recibido, socket_cliente, &id_correlativo, &id_mensaje_recibido,
				finalizar_gamecard, MUTEX_LOGGER);

		//si se recibe el mensaje de error (que se genera si se CAE EL GAMEBOY O SI HAY UN ERROR), ESPERA OTRA CONEXION
		if(mensaje == NULL) continue;

		char* mensaje_para_loguear = generar_mensaje_para_loggear(mensaje, codigo_operacion_recibido);

		pthread_mutex_lock(MUTEX_LOGGER);
		log_info(LOGGER, "Se recibio el mensaje <<  %s  >> del GAMEBOY", mensaje_para_loguear);
		pthread_mutex_unlock(MUTEX_LOGGER);

		free(mensaje_para_loguear);

		if(close(socket_cliente) == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al cerrar socket en recibir_conexiones()", finalizar_gamecard, MUTEX_LOGGER);

		iniciar_hilo_para_tratar_y_responder_mensaje(id_mensaje_recibido, mensaje, codigo_operacion_recibido);
    }

	return NULL;
}

pthread_t iniciar_hilo_para_comunicarse_con_broker(char* cola_a_suscribirse, op_code codigo_suscripcion)
{
	argumentos_de_hilo* arg = malloc(sizeof(argumentos_de_hilo));
	arg->stream = cola_a_suscribirse;
	arg->entero = codigo_suscripcion;

	pthread_t thread;
	if(0 != pthread_create(&thread, NULL, conectar_recibir_y_enviar_mensajes, (void*)arg))
		imprimir_error_y_terminar_programa_perzonalizado("No se pudo crear hilo conectar_recibir_y_enviar_mensajes()", finalizar_gamecard, MUTEX_LOGGER);

	return thread;
}

void* conectar_recibir_y_enviar_mensajes(void* argumentos)
{
	argumentos_de_hilo* args = argumentos;
	char* cola_a_suscribirse = args->stream;
	op_code codigo_suscripcion = args->entero;
	free(args);

	while(true)
	{
		char* mensaje_de_logueo_al_reintentar_conexion = string_from_format("La conexion con el BROKER para suscribirse a cola %s fallo.", cola_a_suscribirse);
		int conexion = conectar_a_broker_y_reintentar_si_hace_falta(mensaje_de_logueo_al_reintentar_conexion);
		free(mensaje_de_logueo_al_reintentar_conexion);

		int estado_envio = enviar_mensaje_de_suscripcion(conexion, codigo_suscripcion, ID_MANUAL_DEL_PROCESO, MUTEX_ID_MANUAL_DEL_PROCESO);
		if(estado_envio == 0) continue;
		if(estado_envio == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_mensaje_de_suscripcion()", finalizar_gamecard, MUTEX_LOGGER);

		pthread_mutex_lock(MUTEX_LOGGER);
		log_info(LOGGER, "Se realizo una suscripcion a la cola de mensajes %s del BROKER", cola_a_suscribirse);
		pthread_mutex_unlock(MUTEX_LOGGER);

		while(true)
		{
			int id_correlativo = 0;
			int id_mensaje_recibido = 0; 	//EL GAMECARD SOLO USA ID MENSAJE

			op_code codigo_operacion_recibido = 0;

			//queda bloqueado hasta que el gameboy recibe un mensaje,
			void* mensaje = recibir_mensaje_por_socket(&codigo_operacion_recibido, conexion, &id_correlativo, &id_mensaje_recibido,
					finalizar_gamecard, MUTEX_LOGGER);

			//si se recibe el mensaje de error (que se genera si se CAE EL BROKER), se sale de este while(true) y reintenta la conexion
			if(mensaje == NULL) break;

			int estado_ack = enviar_ack(conexion, id_mensaje_recibido); //NO VERIFICO EL ESTADO PORQUE NO IMPORTA, EL GAMECARD VA A REALIZAR SU TAREA ASIGNADA DE TODAS FORMAS
			//ADEMAS EL BROKER, COMO PERDIO TODOS LOS MENSAJES QUE TENIA EN MEMORIA, NO VA A REENVIAR NADA A GAMECARD

			if(estado_ack == -1)
				imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_ack()", finalizar_gamecard, MUTEX_LOGGER);

			char* mensaje_para_loguear = generar_mensaje_para_loggear(mensaje, codigo_suscripcion);

			pthread_mutex_lock(MUTEX_LOGGER);
			log_info(LOGGER, "Se recibio el mensaje <<  %s  >> de la cola %s", mensaje_para_loguear, cola_a_suscribirse);
			pthread_mutex_unlock(MUTEX_LOGGER);

			free(mensaje_para_loguear);

			iniciar_hilo_para_tratar_y_responder_mensaje(id_mensaje_recibido, mensaje, codigo_suscripcion);
		}
	}

	return NULL;
}

int conectar_a_broker_y_reintentar_si_hace_falta(char* mensaje_de_logueo_al_reintentar_conexion)
{
	char* ip = NULL;
	char* puerto = NULL;

	pthread_mutex_lock(MUTEX_CONFIG);
	int retorno_ip_puerto = leer_ip_y_puerto(&ip, &puerto, CONFIG, "BROKER");
	pthread_mutex_unlock(MUTEX_CONFIG);

	if(retorno_ip_puerto == -1)
		imprimir_error_y_terminar_programa_perzonalizado("No se encontro IP_BROKER o PUERTO_BROKER en el archivo de configuracion",
				finalizar_gamecard, MUTEX_CONFIG);

	int conexion = iniciar_conexion_como_cliente(ip, puerto);

	if(conexion == -1)
		imprimir_error_y_terminar_programa_perzonalizado("Error en socket() en crear_socket_como_cliente", finalizar_gamecard, MUTEX_LOGGER);

	pthread_mutex_lock(MUTEX_CONFIG);
	int tiempo_de_reintento_conexion = asignar_int_property(CONFIG, "TIEMPO_DE_REINTENTO_CONEXION");
	pthread_mutex_unlock(MUTEX_CONFIG);

	while(conexion <= 0)
	{
		pthread_mutex_lock(MUTEX_LOGGER);
		log_info(LOGGER, "%s Reintentando conexion en : %i segundos", mensaje_de_logueo_al_reintentar_conexion, tiempo_de_reintento_conexion);
		pthread_mutex_unlock(MUTEX_LOGGER);

		sleep(tiempo_de_reintento_conexion);

		conexion = iniciar_conexion_como_cliente(ip, puerto);

		if(conexion == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error en socket() en crear_socket_como_cliente", finalizar_gamecard, MUTEX_LOGGER);

		if(conexion > 0)
		{
			int retorno_identificacion = enviar_identificacion_general(conexion, ID_PROCESOS_TP, MUTEX_ID_PROCESOS_TP);

			if(retorno_identificacion == 0) conexion = -1;
			//SI FALLA LA IDENTIFICACIOJN, ENTONCES SE INTENTA RECONECTAR DEVUELTA

			if(retorno_identificacion == -1)
				imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_identificacion_general()",
						finalizar_gamecard, MUTEX_LOGGER);
		}
	}

	pthread_mutex_lock(MUTEX_LOGGER);
	log_info(LOGGER, "Conexion con BROKER exitosa");
	pthread_mutex_unlock(MUTEX_LOGGER);

	return conexion;
}

void verificar_estado_del_envio_y_cerrar_conexion(char* tipo_mensaje, int estado, int conexion)
{
	if(estado <= 0){

		pthread_mutex_lock(MUTEX_LOGGER);
		log_error(LOGGER, "No se pudo enviar %s al BROKER. Continuando operacion del GAME CARD", tipo_mensaje);
		pthread_mutex_unlock(MUTEX_LOGGER);

	}
	else
	{
		int id_mensaje_enviado = esperar_id_mensaje_enviado(conexion);	//EL GAMECARD NO HACE NADA CON EL ID, SOLO TIENE QUE ESPERARLO

		if(id_mensaje_enviado == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar recv() en esperar_id_mensaje_enviado()", finalizar_gamecard, MUTEX_LOGGER);

		pthread_mutex_lock(MUTEX_LOGGER);
		log_error(LOGGER, "Mensaje %s enviado al BROKER correctamente", tipo_mensaje);
		pthread_mutex_unlock(MUTEX_LOGGER);
	}

	close(conexion);

	pthread_exit(NULL);
}

void iniciar_hilo_para_tratar_y_responder_mensaje(int id_mensaje_recibido, void* mensaje, op_code codigo_suscripcion)
{
	argumentos_de_hilo* arg = malloc(sizeof(argumentos_de_hilo));
	arg->entero = id_mensaje_recibido;
	arg->stream = mensaje;

	pthread_t thread;

	switch (codigo_suscripcion) {
			case SUBSCRIBE_NEW_POKEMON:
				if(0 != pthread_create(&thread, NULL, atender_new_pokemon, (void*)arg))
					imprimir_error_y_terminar_programa_perzonalizado("No se pudo crear hilo para atender_new_pokemon()", finalizar_gamecard, MUTEX_LOGGER);
				break;

			case SUBSCRIBE_CATCH_POKEMON:
				if(0 != pthread_create(&thread, NULL, atender_catch_pokemon, (void*)arg))
					imprimir_error_y_terminar_programa_perzonalizado("No se pudo crear hilo para atender_catch_pokemon()", finalizar_gamecard, MUTEX_LOGGER);
				break;

			case SUBSCRIBE_GET_POKEMON:
				if(0 != pthread_create(&thread, NULL, atender_get_pokemon, (void*)arg))
					imprimir_error_y_terminar_programa_perzonalizado("No se pudo crear hilo para atender_get_pokemon()", finalizar_gamecard, MUTEX_LOGGER);
				break;
			case NEW_POKEMON:
				if(0 != pthread_create(&thread, NULL, atender_new_pokemon, (void*)arg))
					imprimir_error_y_terminar_programa_perzonalizado("No se pudo crear hilo para atender_new_pokemon()", finalizar_gamecard, MUTEX_LOGGER);
				break;

			case CATCH_POKEMON:
				if(0 != pthread_create(&thread, NULL, atender_catch_pokemon, (void*)arg))
					imprimir_error_y_terminar_programa_perzonalizado("No se pudo crear hilo para atender_catch_pokemon()", finalizar_gamecard, MUTEX_LOGGER);
				break;

			case GET_POKEMON:
				if(0 != pthread_create(&thread, NULL, atender_get_pokemon, (void*)arg))
					imprimir_error_y_terminar_programa_perzonalizado("No se pudo crear hilo para atender_get_pokemon()", finalizar_gamecard, MUTEX_LOGGER);
				break;
			default:
				break;
			}

	pthread_detach(thread);
}

void* atender_new_pokemon(void* argumentos)
{
	argumentos_de_hilo* args = argumentos;
	int id_mensaje_recibido = args->entero;
	t_new_pokemon* mensaje = args->stream;
	free(args);

	pthread_mutex_lock(MUTEX_CONSULTA_POKEMON);
	if(verificar_si_existe_archivo_pokemon(mensaje->nombre) == false) crear_archivo_pokemon(mensaje->nombre);
	pthread_mutex_unlock(MUTEX_CONSULTA_POKEMON);

	pedir_archivo_pokemon(mensaje->nombre);

    char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades = generar_array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades((char*) mensaje->nombre);
    char* posicion_buscada_en_string = string_from_format("%i-%i", mensaje->coordenadas.posx, mensaje->coordenadas.posy);

	pthread_mutex_lock(MUTEX_LOGGER);
    printf("posicion_buscada_en_string : %s\n", posicion_buscada_en_string);
    fflush(stdout);
	pthread_mutex_unlock(MUTEX_LOGGER);

	int retorno_agregar = agregar_cantidad_en_archivo_pokemon(mensaje, array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades, posicion_buscada_en_string);

	pthread_mutex_lock(MUTEX_LOGGER);
    printf("retorno_agregar : %i\n", retorno_agregar);
    fflush(stdout);
	pthread_mutex_unlock(MUTEX_LOGGER);

	retener_un_rato_y_liberar_archivo_pokemon(mensaje->nombre);

	char* ip = NULL;
	char* puerto = NULL;

	pthread_mutex_lock(MUTEX_CONFIG);
	int retorno_ip_puerto = leer_ip_y_puerto(&ip, &puerto, CONFIG, "BROKER");
	pthread_mutex_unlock(MUTEX_CONFIG);

	if(retorno_ip_puerto == -1)
		imprimir_error_y_terminar_programa_perzonalizado("No se encontro IP_BROKER o PUERTO_BROKER en el archivo de configuracion", finalizar_gamecard, MUTEX_LOGGER);

	int conexion = iniciar_conexion_como_cliente(ip, puerto);
	if(conexion == -1)
		imprimir_error_y_terminar_programa_perzonalizado("Error en socket() en crear_socket_como_cliente", finalizar_gamecard, MUTEX_LOGGER);

	int retorno_identificacion = 0;

	if( conexion > 0)
	{
		retorno_identificacion = enviar_identificacion_general(conexion, ID_PROCESOS_TP, MUTEX_ID_PROCESOS_TP);

		if(retorno_identificacion == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_identificacion_general()", finalizar_gamecard, MUTEX_LOGGER);
	}

	int estado = 0;

	if((retorno_agregar != -1) && (retorno_identificacion != 0))
	{
		estado = generar_y_enviar_appeared_pokemon(conexion, 0, id_mensaje_recibido, mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);

		if(estado == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_mensaje_como_cliente()", finalizar_gamecard, MUTEX_LOGGER);
	}


    int i = 0;
    while(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i] != NULL)
    {
    	free(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i]);
    	i++;
    }
    free(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades);
    free(posicion_buscada_en_string);

	free(mensaje->nombre);
	free(mensaje);

	verificar_estado_del_envio_y_cerrar_conexion("APPEARED_POKEMON", estado, conexion);

	return NULL;
}

void* atender_catch_pokemon(void* argumentos)
{
	argumentos_de_hilo* args = argumentos;
	int id_mensaje_recibido = args->entero;
	t_catch_pokemon* mensaje = args->stream;
	free(args);

	if(verificar_si_existe_archivo_pokemon(mensaje->nombre) == false)
	{
		pthread_mutex_lock(MUTEX_LOGGER);
		log_error(LOGGER, "Se recibio CATCH_POKEMON. No existe pokemon solicitado en file system. Nombre del pokemon: %s", mensaje->nombre);
		pthread_mutex_unlock(MUTEX_LOGGER);

		conectar_enviar_verificar_caught(id_mensaje_recibido, 0);
	}

	pedir_archivo_pokemon(mensaje->nombre);

    char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades = generar_array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades((char*) mensaje->nombre);
    char* posicion_buscada_en_string = string_from_format("%i-%i", mensaje->coordenadas.posx, mensaje->coordenadas.posy);
    int indice_de_busqueda = buscar_posicion_en_archivo_pokemon(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades, posicion_buscada_en_string);

	if(indice_de_busqueda == -1)
	{
		pthread_mutex_lock(MUTEX_LOGGER);
		log_error(LOGGER, "Se recibio CATCH_POKEMON. No existe pokemon en ubicacion solicitada en file system. "
				"Nombre del pokemon: %s posx: %i posy %i", mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);
		pthread_mutex_unlock(MUTEX_LOGGER);

		retener_conectar_librerar_recursos_caught(mensaje, id_mensaje_recibido, 0, posicion_buscada_en_string,
				array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades);
	}

	reducir_cantidad_en_archivo_pokemon(mensaje->nombre, indice_de_busqueda, array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades, posicion_buscada_en_string);

	retener_conectar_librerar_recursos_caught(mensaje, id_mensaje_recibido, 1, posicion_buscada_en_string,
			array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades);

	return NULL;
}

void* atender_get_pokemon(void* argumentos)
{
	argumentos_de_hilo* args = argumentos;
	int id_mensaje_recibido = args->entero;
	t_get_pokemon* mensaje = args->stream;
	free(args);

	if(verificar_si_existe_archivo_pokemon(mensaje->nombre) == false)
	{
		pthread_mutex_lock(MUTEX_LOGGER);
		log_error(LOGGER, "Se recibio GET_POKEMON. No existe pokemon solicitado en file system. "
				"Se decide no enviar LOCALIZED_POKEMON y continuar con la ejecucion normal de Game-Card. Nombre del pokemon: %s", mensaje->nombre);
		pthread_mutex_unlock(MUTEX_LOGGER);

		pthread_exit(NULL);
	}

	pedir_archivo_pokemon(mensaje->nombre);

	t_list* lista_posiciones = obtener_todas_las_posiciones_de_archivo_pokemon(mensaje->nombre);
	//segun el anexo 2, no tengo que indicar la cantidad de pokemones en cada posicion

	retener_un_rato_y_liberar_archivo_pokemon(mensaje->nombre);

	char* ip = NULL;
	char* puerto = NULL;

	pthread_mutex_lock(MUTEX_CONFIG);
	int retorno_ip_puerto = leer_ip_y_puerto(&ip, &puerto, CONFIG, "BROKER");
	pthread_mutex_unlock(MUTEX_CONFIG);

	if(retorno_ip_puerto == -1)
		imprimir_error_y_terminar_programa_perzonalizado("No se encontro IP_BROKER o PUERTO_BROKER en el archivo de configuracion", finalizar_gamecard, MUTEX_LOGGER);

	int conexion = iniciar_conexion_como_cliente(ip, puerto);
	if(conexion == -1)
		imprimir_error_y_terminar_programa_perzonalizado("Error en socket() en crear_socket_como_cliente", finalizar_gamecard, MUTEX_LOGGER);

	int retorno_identificacion = 0;

	if(conexion > 0)
	{
		retorno_identificacion = enviar_identificacion_general(conexion, ID_PROCESOS_TP, MUTEX_ID_PROCESOS_TP);

		if(retorno_identificacion == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_identificacion_general()", finalizar_gamecard, MUTEX_LOGGER);
	}

	int estado = 0;

	if((retorno_identificacion != 0) && (lista_posiciones->elements_count != 0))
	{
		estado = generar_y_enviar_localized_pokemon(conexion, 0, id_mensaje_recibido, mensaje->nombre, lista_posiciones);

		if(estado == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_mensaje_como_cliente()", finalizar_gamecard, MUTEX_LOGGER);
	}

	list_destroy_and_destroy_elements(lista_posiciones, free);
	free(mensaje->nombre);
	free(mensaje);

	verificar_estado_del_envio_y_cerrar_conexion("LOCALIZED_POKEMON", estado, conexion);

	return NULL;
}

void retener_conectar_librerar_recursos_caught(t_catch_pokemon* mensaje, int id_mensaje_recibido, int resultado_caught, char* posicion_buscada_en_string,
		char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades)
{
	retener_un_rato_y_liberar_archivo_pokemon(mensaje->nombre);

    int i = 0;
    while(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i] != NULL)
    {
    	free(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i]);
    	i++;
    }
    free(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades);
    free(posicion_buscada_en_string);

	free(mensaje->nombre);
	free(mensaje);

	conectar_enviar_verificar_caught(id_mensaje_recibido, resultado_caught);
}

void conectar_enviar_verificar_caught(int id_mensaje_recibido, int resultado_caught)
{
	char* ip = NULL;
	char* puerto = NULL;

	pthread_mutex_lock(MUTEX_CONFIG);
	int retorno_ip_puerto = leer_ip_y_puerto(&ip, &puerto, CONFIG, "BROKER");
	pthread_mutex_unlock(MUTEX_CONFIG);

	if(retorno_ip_puerto == -1)
		imprimir_error_y_terminar_programa_perzonalizado("No se encontro IP_BROKER o PUERTO_BROKER en el archivo de configuracion",
				finalizar_gamecard, MUTEX_LOGGER);

	int conexion = iniciar_conexion_como_cliente(ip, puerto);
	if(conexion == -1)
		imprimir_error_y_terminar_programa_perzonalizado("Error en socket() en crear_socket_como_cliente", finalizar_gamecard, MUTEX_LOGGER);

	int retorno_identificacion = 0;

	if(conexion > 0)
	{
		retorno_identificacion = enviar_identificacion_general(conexion, ID_PROCESOS_TP, MUTEX_ID_PROCESOS_TP);

		if(retorno_identificacion == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_identificacion_general()", finalizar_gamecard, MUTEX_LOGGER);
	}

	int estado = 0;

	if(retorno_identificacion != 0)
	{
		estado = generar_y_enviar_caught_pokemon(conexion, 0, id_mensaje_recibido, resultado_caught);

		if(estado == -1)
			imprimir_error_y_terminar_programa_perzonalizado("Error al usar send() en enviar_mensaje_como_cliente()", finalizar_gamecard, MUTEX_LOGGER);
	}

	verificar_estado_del_envio_y_cerrar_conexion("CAUGHT_POKEMON", estado, conexion);
}

bool verificar_si_existe_archivo_pokemon(char* nombre_pokemon)
{
	pthread_mutex_lock(MUTEX_CONFIG);
	char* punto_montaje_file_system = asignar_string_property(CONFIG, "PUNTO_MONTAJE_TALLGRASS");
	pthread_mutex_unlock(MUTEX_CONFIG);

	char* path_carpeta_con_nombre = string_from_format("%s/Files/%s", punto_montaje_file_system, nombre_pokemon);
	DIR* carpeta = opendir(path_carpeta_con_nombre);

	if(carpeta == NULL)
	{
		free(path_carpeta_con_nombre);
		return false;
	}

	closedir(carpeta);

	char* path_archivo_con_nombre = string_from_format("%s/Metadata.bin", path_carpeta_con_nombre);
	free(path_carpeta_con_nombre);

	struct stat datos_archivo;

	if(stat(path_archivo_con_nombre, &datos_archivo) == -1)
	{
		free(path_archivo_con_nombre);
		return false;
	}

	free(path_archivo_con_nombre);

	if(datos_archivo.st_size == 0) return false;

	return true;	//DEBERIA MIRAR SI EL CONTENIDO ADENTRO DEL ARCHIVO ME SIRVE... TODO . por ahora no lo mires...no creo que me den archivos asi
}

void crear_archivo_pokemon(char* nombre_pokemon)
{
	pthread_mutex_lock(MUTEX_CONFIG);
	char* punto_montaje_file_system = asignar_string_property(CONFIG, "PUNTO_MONTAJE_TALLGRASS");
	pthread_mutex_unlock(MUTEX_CONFIG);

	char* path_carpeta_con_nombre = string_from_format("%s/Files/%s", punto_montaje_file_system, nombre_pokemon);
	crear_carpeta_si_no_existe(path_carpeta_con_nombre);

	char* path_archivo_con_nombre = string_from_format("%s/Metadata.bin", path_carpeta_con_nombre);

	pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);

	pthread_mutex_lock(MUTEX_DICCIONARIO);
	dictionary_put(DICCIONARIO_CON_MUTEX, path_archivo_con_nombre, mutex);
	pthread_mutex_unlock(MUTEX_DICCIONARIO);

	sobrescribir_y_cerrar_archivo(path_archivo_con_nombre, "DIRECTORY=N\nSIZE=0\nBLOCKS=[]\nOPEN=N", strlen("DIRECTORY=N\nSIZE=0\nBLOCKS=[0]\nOPEN=N"));
	//DEBERIA PONER BLOCK=[0] ? O DEJARLO ASI? REVISAR TODO

	pthread_mutex_lock(MUTEX_LOGGER);
	log_info(LOGGER, "Nuevo archivo pokemon creado : %s", path_carpeta_con_nombre);
	pthread_mutex_unlock(MUTEX_LOGGER);

	free(path_carpeta_con_nombre);
	free(path_archivo_con_nombre);
}

void pedir_archivo_pokemon(char* nombre_pokemon)
{
	pthread_mutex_lock(MUTEX_CONFIG);
	int tiempo_de_reintento_operacion = asignar_int_property(CONFIG, "TIEMPO_DE_REINTENTO_OPERACION");
	pthread_mutex_unlock(MUTEX_CONFIG);

	bool se_pudo_abrir = intentar_abrir_archivo_pokemon(nombre_pokemon);

	while(se_pudo_abrir == false)
	{
		sleep(tiempo_de_reintento_operacion);

		se_pudo_abrir = intentar_abrir_archivo_pokemon(nombre_pokemon);
	}
}

bool intentar_abrir_archivo_pokemon(char* nombre_pokemon)
{
	pthread_mutex_lock(MUTEX_CONFIG);
	char* punto_montaje_file_system = asignar_string_property(CONFIG, "PUNTO_MONTAJE_TALLGRASS");
	pthread_mutex_unlock(MUTEX_CONFIG);

	char* path_archivo_pokemon_metadata_bin = string_from_format("%s/Files/%s/Metadata.bin", punto_montaje_file_system, nombre_pokemon);

	pthread_mutex_lock(MUTEX_DICCIONARIO);
	pthread_mutex_t* mutex_archivo_pokemon = dictionary_get(DICCIONARIO_CON_MUTEX, path_archivo_pokemon_metadata_bin);
	pthread_mutex_unlock(MUTEX_DICCIONARIO);

	if(mutex_archivo_pokemon == NULL)
		imprimir_error_y_terminar_programa_perzonalizado("No se encontro un MUTEX en el DICCIONARIO", finalizar_gamecard, MUTEX_LOGGER);

	pthread_mutex_lock(mutex_archivo_pokemon);
	t_config* archivo_pokemon_metadata_bin = config_create(path_archivo_pokemon_metadata_bin);

	if(archivo_pokemon_metadata_bin == NULL)
	{
		char* mensaje_error = string_from_format("No se pudo abrir archivo %s en intentar_abrir_archivo_pokemon()", path_archivo_pokemon_metadata_bin);
		imprimir_error_y_terminar_programa_perzonalizado(mensaje_error, finalizar_gamecard, MUTEX_LOGGER);
		free(mensaje_error); //ESTO NUNCA SE EJECUTA...
	}
	free(path_archivo_pokemon_metadata_bin);

	char* estado_apertura = asignar_string_property(archivo_pokemon_metadata_bin, "OPEN");

	if(strcmp(estado_apertura,"N") == 0)
	{
		config_set_value(archivo_pokemon_metadata_bin, "OPEN", "Y");
		config_save(archivo_pokemon_metadata_bin);
		config_destroy(archivo_pokemon_metadata_bin);
		pthread_mutex_unlock(mutex_archivo_pokemon);

		return true;
	}

	config_destroy(archivo_pokemon_metadata_bin);
	pthread_mutex_unlock(mutex_archivo_pokemon);

	return false;
}

void retener_un_rato_y_liberar_archivo_pokemon(char* nombre_pokemon)
{
	pthread_mutex_lock(MUTEX_CONFIG);
	int tiempo_de_retardo_operacion = asignar_int_property(CONFIG, "TIEMPO_RETARDO_OPERACION");
	pthread_mutex_unlock(MUTEX_CONFIG);

	sleep(tiempo_de_retardo_operacion);

	liberar_archivo_pokemon(nombre_pokemon);
}

void liberar_archivo_pokemon(char* nombre_pokemon)
{
	pthread_mutex_lock(MUTEX_CONFIG);
	char* punto_montaje_file_system = asignar_string_property(CONFIG, "PUNTO_MONTAJE_TALLGRASS");
	pthread_mutex_unlock(MUTEX_CONFIG);
	char* path_archivo_pokemon_metadata_bin = string_from_format("%s/Files/%s/Metadata.bin", punto_montaje_file_system, nombre_pokemon);

	pthread_mutex_lock(MUTEX_DICCIONARIO);
	pthread_mutex_t* mutex_archivo_pokemon = dictionary_get(DICCIONARIO_CON_MUTEX, path_archivo_pokemon_metadata_bin);
	pthread_mutex_unlock(MUTEX_DICCIONARIO);

	if(mutex_archivo_pokemon == NULL)
		imprimir_error_y_terminar_programa_perzonalizado("No se encontro un MUTEX en el DICCIONARIO", finalizar_gamecard, MUTEX_LOGGER);

	pthread_mutex_lock(mutex_archivo_pokemon);
	t_config* archivo_pokemon_metadata_bin = config_create(path_archivo_pokemon_metadata_bin);

	if(archivo_pokemon_metadata_bin == NULL)
	{
		char* mensaje_error = string_from_format("No se pudo abrir archivo %s en liberar_archivo_pokemon()", path_archivo_pokemon_metadata_bin);
		imprimir_error_y_terminar_programa_perzonalizado(mensaje_error, finalizar_gamecard, MUTEX_LOGGER);
		free(mensaje_error); //ESTO NUNCA SE EJECUTA...
	}
	free(path_archivo_pokemon_metadata_bin);

	config_set_value(archivo_pokemon_metadata_bin, "OPEN", "N");
	config_save(archivo_pokemon_metadata_bin);
	config_destroy(archivo_pokemon_metadata_bin);

	pthread_mutex_unlock(mutex_archivo_pokemon);
}

int buscar_posicion_en_archivo_pokemon(char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades, char* posicion_buscada_en_string)
{
    //**** BUSQUEDA DE LAS POSICIONES ****//	--->   SI NO SE ENCONTRO LA POSICION BUSCADA, DEVUELVE -1. SI LO ENCUENTRA, DEVUELVE EL INDICE DEL ARRAY EN DONDE SE ENCUENTRA
    int i = 0;

    int indice_de_busqueda = -1;

    while(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i] != NULL)
    {
        if(string_starts_with(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i], posicion_buscada_en_string) == true)
        {
        	indice_de_busqueda = i;
        	break;
        }

        i++;
    }

    return indice_de_busqueda;
}

t_list* obtener_todas_las_posiciones_de_archivo_pokemon(char* nombre_pokemon)
{
    //**** BUSQUEDA DE LAS POSICIONES ****//

    char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades = generar_array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades(nombre_pokemon);

    t_list* lista_coordenadas = list_create();

    int i = 0;

    while(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i] != NULL)
    {
        char** key_y_value = string_split(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i], "=");

        char** pos_x_pos_y = string_split(key_y_value[0], "-");

        t_coordenadas* coordenadas = malloc(sizeof(t_coordenadas));

        coordenadas->posx = atoi(pos_x_pos_y[0]);
        coordenadas->posy = atoi(pos_x_pos_y[1]);

        list_add(lista_coordenadas, coordenadas);

        i++;

        free(key_y_value[0]);
        free(key_y_value[1]);
        free(key_y_value);

        free(pos_x_pos_y);
    }

    i = 0;
    while(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i] != NULL)
    {
    	free(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i]);
    	i++;
    }
    free(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades);

    return lista_coordenadas;
}

char** generar_array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades(char* nombre_pokemon)
{
	pthread_mutex_lock(MUTEX_CONFIG);
	char* punto_montaje_file_system = asignar_string_property(CONFIG, "PUNTO_MONTAJE_TALLGRASS");
	pthread_mutex_unlock(MUTEX_CONFIG);

	char* path_archivo_pokemon_metadata_bin = string_from_format("%s/Files/%s/Metadata.bin", punto_montaje_file_system, nombre_pokemon);

	pthread_mutex_lock(MUTEX_DICCIONARIO);
	pthread_mutex_t* mutex_archivo_pokemon = dictionary_get(DICCIONARIO_CON_MUTEX, path_archivo_pokemon_metadata_bin);
	pthread_mutex_unlock(MUTEX_DICCIONARIO);

	if(mutex_archivo_pokemon == NULL)
		imprimir_error_y_terminar_programa_perzonalizado("No se encontro un MUTEX en el DICCIONARIO", finalizar_gamecard, MUTEX_LOGGER);

	pthread_mutex_lock(mutex_archivo_pokemon);
	t_config* archivo_pokemon_metadata_bin = config_create(path_archivo_pokemon_metadata_bin);

	if(archivo_pokemon_metadata_bin == NULL)
	{
		char* mensaje_error = string_from_format("No se pudo abrir archivo %s en "
				"generar_array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades()", path_archivo_pokemon_metadata_bin);
		imprimir_error_y_terminar_programa_perzonalizado(mensaje_error, finalizar_gamecard, MUTEX_LOGGER);
		free(mensaje_error); //ESTO NUNCA SE EJECUTA...
	}
	free(path_archivo_pokemon_metadata_bin);

	char* bloques_del_archivo_pokemon = asignar_string_property(archivo_pokemon_metadata_bin, "BLOCKS");
	pthread_mutex_unlock(mutex_archivo_pokemon);

	pthread_mutex_lock(MUTEX_LOGGER);
	printf("bloques: %s\n", bloques_del_archivo_pokemon);
	fflush(stdout);
	pthread_mutex_unlock(MUTEX_LOGGER);

	char** array_bloques_del_archivo_pokemon = string_get_string_as_array(bloques_del_archivo_pokemon);	//REVISAR QUE PASA CON LISTA VACIA TODO

	pthread_mutex_lock(MUTEX_METADATA_METADATA_BIN);
	int tamanio_bloques_del_file_system = asignar_int_property(METADATA_METADATA_BIN, "BLOCK_SIZE");
	pthread_mutex_unlock(MUTEX_METADATA_METADATA_BIN);


	//**** METO TODA LA INFO DEL ARCHIVO POKEMON (QUE ESTABA SEPARADA EN BLOQUES) EN UN STRING ****//

	char* todo_el_archivo_pokemon_en_un_string = string_new();

    int i = 0;
    while (array_bloques_del_archivo_pokemon[i] != NULL)
    {
    	char* string_de_un_bloque_del_archivo_pokemon = array_bloques_del_archivo_pokemon[i];

    	char* path_un_bloque_bin = string_from_format("%s/Blocks/%i.bin", punto_montaje_file_system, atoi(string_de_un_bloque_del_archivo_pokemon));

    	FILE* archivo_bloque_bin = fopen(path_un_bloque_bin, "rb");
    	free(path_un_bloque_bin);

    	char* contenido_de_un_bloque = malloc(tamanio_bloques_del_file_system);

    	fread(contenido_de_un_bloque, tamanio_bloques_del_file_system, 1, archivo_bloque_bin);
    	fflush(archivo_bloque_bin);

    	string_append_with_format(&todo_el_archivo_pokemon_en_un_string, "%s", contenido_de_un_bloque);
    	free(contenido_de_un_bloque);

    	fclose(archivo_bloque_bin);

	    i++;
    }

	pthread_mutex_lock(MUTEX_LOGGER);
    printf("todo_el_archivo_pokemon_en_un_string : <%s>\n", todo_el_archivo_pokemon_en_un_string);
    fflush(stdout);
	pthread_mutex_unlock(MUTEX_LOGGER);

    char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades = string_split(todo_el_archivo_pokemon_en_un_string, "\n");
    free(todo_el_archivo_pokemon_en_un_string);

	pthread_mutex_lock(MUTEX_LOGGER);
    printf("array 1 : <%s>\n", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[0]);
   // printf("array 2 : <%s>\n", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[1]);
    fflush(stdout);
	pthread_mutex_unlock(MUTEX_LOGGER);

    i = 0;
    while(array_bloques_del_archivo_pokemon[i] != NULL)
    {
    	free(array_bloques_del_archivo_pokemon[i]);
    	i++;
    }
    free(array_bloques_del_archivo_pokemon);

	pthread_mutex_lock(mutex_archivo_pokemon);
    config_destroy(archivo_pokemon_metadata_bin);
	pthread_mutex_unlock(mutex_archivo_pokemon);

    return array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades;
}

int agregar_cantidad_en_archivo_pokemon(t_new_pokemon* mensaje, char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades, char* posicion_buscada_en_string)
{
    int indice_de_busqueda = buscar_posicion_en_archivo_pokemon(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades, posicion_buscada_en_string);

    //**** SE AGREGA CANTIDAD EN EL nuevo_archivo_pokemon_en_un_string  ****//
	pthread_mutex_lock(MUTEX_LOGGER);
    printf("indice_de_busqueda : %i\n", indice_de_busqueda);
    fflush(stdout);
	pthread_mutex_unlock(MUTEX_LOGGER);

    char* nuevo_archivo_pokemon_en_un_string = string_new();

    int i = 1;

    if(indice_de_busqueda != -1)
    {	//  SI ESTO SE EJECUTA, REPRESENTA UN --->  agregar_cantidad_en_linea();
    	char* linea_de_posicion_encontrada = array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[indice_de_busqueda];
        char** key_y_value = string_split(linea_de_posicion_encontrada, "=");
        char* cantidad_anterior_de_pokemones = key_y_value[1];
        int cantidad_nueva_de_pokemones = atoi(cantidad_anterior_de_pokemones) + mensaje->cantidad;

        array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[indice_de_busqueda] = string_from_format("%s=%i", posicion_buscada_en_string, cantidad_nueva_de_pokemones);

        free(linea_de_posicion_encontrada);
        free(key_y_value[0]);
        free(key_y_value[1]);
        free(key_y_value);

    	string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "%s", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[0]);

        while(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i] != NULL)
        {
        	string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "\n%s", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i]);

        	i++;
        }
    }
    else
    {	//  SI ESTO SE EJECUTA, REPRESENTA UN --->   agregar_cantidad_al_final();
        char* posicion_para_agregar_al_final = string_from_format("%s=%i", posicion_buscada_en_string, mensaje->cantidad);

        if(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[0] != NULL)
        {
        	string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "%s", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[0]);


        	while(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i] != NULL)
        	{
        		string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "\n%s", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i]);

        		i++;
        	}

            string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "\n%s", posicion_para_agregar_al_final);
        }
        else
        {// ESTO SE EJECUTA SI EL ARCHIVO POKEMON NO TENIA NINGUNA POSICION
        	string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "%s", posicion_para_agregar_al_final);
        }
        free(posicion_para_agregar_al_final);
    }

	pthread_mutex_lock(MUTEX_LOGGER);
    printf("nuevo_archivo_pokemon_en_un_string : %s\n", nuevo_archivo_pokemon_en_un_string);
    fflush(stdout);
	pthread_mutex_unlock(MUTEX_LOGGER);
    //**** SE INTERPRETA SI HAY QUE AGREGAR UN BLOQUE MAS  ****//

	pthread_mutex_lock(MUTEX_CONFIG);
	char* punto_montaje_file_system = asignar_string_property(CONFIG, "PUNTO_MONTAJE_TALLGRASS");
	pthread_mutex_unlock(MUTEX_CONFIG);

	char* path_archivo_pokemon_metadata_bin = string_from_format("%s/Files/%s/Metadata.bin", punto_montaje_file_system, (char*) mensaje->nombre);

	pthread_mutex_lock(MUTEX_DICCIONARIO);
	pthread_mutex_t* mutex_archivo_pokemon = dictionary_get(DICCIONARIO_CON_MUTEX, path_archivo_pokemon_metadata_bin);
	pthread_mutex_unlock(MUTEX_DICCIONARIO);

	if(mutex_archivo_pokemon == NULL)
		imprimir_error_y_terminar_programa_perzonalizado("No se encontro un MUTEX en el DICCIONARIO", finalizar_gamecard, MUTEX_LOGGER);

	pthread_mutex_lock(mutex_archivo_pokemon);
	t_config* archivo_pokemon_metadata_bin = config_create(path_archivo_pokemon_metadata_bin);

	if(archivo_pokemon_metadata_bin == NULL)
	{
		char* mensaje_error = string_from_format("No se pudo abrir archivo %s en agregar_cantidad_en_archivo_pokemon()", path_archivo_pokemon_metadata_bin);
		imprimir_error_y_terminar_programa_perzonalizado(mensaje_error, finalizar_gamecard, MUTEX_LOGGER);
		free(mensaje_error); //ESTO NUNCA SE EJECUTA...
	}
	free(path_archivo_pokemon_metadata_bin);

	int tamanio_anterior_archivo_pokemon = asignar_int_property(archivo_pokemon_metadata_bin, "SIZE");
    char* nuevo_value_SIZE_archivo_pokemon = string_from_format("%i", strlen(nuevo_archivo_pokemon_en_un_string));
    config_set_value(archivo_pokemon_metadata_bin, "SIZE", nuevo_value_SIZE_archivo_pokemon);

	pthread_mutex_lock(MUTEX_METADATA_METADATA_BIN);
	int tamanio_bloques_del_file_system = asignar_int_property(METADATA_METADATA_BIN, "BLOCK_SIZE");
	pthread_mutex_unlock(MUTEX_METADATA_METADATA_BIN);

    int primer_es_division_con_resto = 0;
    if((tamanio_anterior_archivo_pokemon % tamanio_bloques_del_file_system) != 0) primer_es_division_con_resto = 1;
    int cantidad_de_bloques_anterior = (tamanio_anterior_archivo_pokemon / tamanio_bloques_del_file_system) + primer_es_division_con_resto;

    int segundo_es_division_con_resto = 0;
    if((atoi(nuevo_value_SIZE_archivo_pokemon) % tamanio_bloques_del_file_system) != 0) segundo_es_division_con_resto = 1;
    int cantidad_de_bloques_necesaria = (atoi(nuevo_value_SIZE_archivo_pokemon) / tamanio_bloques_del_file_system) + segundo_es_division_con_resto;

	char* bloques_del_archivo_pokemon = asignar_string_property(archivo_pokemon_metadata_bin, "BLOCKS");
	char** array_bloques_del_archivo_pokemon = string_get_string_as_array(bloques_del_archivo_pokemon);	//REVISAR QUE PASA CON LISTA VACIA TODO
	pthread_mutex_unlock(mutex_archivo_pokemon);

	pthread_mutex_lock(MUTEX_LOGGER);
	printf("size nuevo : %i\n", atoi(nuevo_value_SIZE_archivo_pokemon));
	fflush(stdout);
	printf("segundo resto: %i\n", (atoi(nuevo_value_SIZE_archivo_pokemon) % tamanio_bloques_del_file_system));
	fflush(stdout);

	printf("cantidad_de_bloques_anterior : %i\n", cantidad_de_bloques_anterior);
	fflush(stdout);
	printf("cantidad_de_bloques_necesaria : %i\n", cantidad_de_bloques_necesaria);
	fflush(stdout);

	printf("nuevo array 0: %s\n", array_bloques_del_archivo_pokemon[0]);
	fflush(stdout);
	printf("nuevo array 1: %s\n", array_bloques_del_archivo_pokemon[1]);
	fflush(stdout);

	printf("cantidad_de_bloques_necesaria : %i\n", cantidad_de_bloques_necesaria);
	fflush(stdout);
	pthread_mutex_unlock(MUTEX_LOGGER);

    int j = 0;
    i = 0;

    int valor_retorno_funcion = 0;

    if(cantidad_de_bloques_anterior == cantidad_de_bloques_necesaria)
    {	//ESTO SE EJECUTA SI ME ALCANZA CON LOS BLOQUES QUE TENGO
    	while(j < atoi(nuevo_value_SIZE_archivo_pokemon))
    	{
    		char* cadena_a_grabar = string_substring(nuevo_archivo_pokemon_en_un_string, j, tamanio_bloques_del_file_system);

           	char* un_bloque_string_nuevo = array_bloques_del_archivo_pokemon[i];

           	char* path_bloque_bin_nuevo = string_from_format("%s/Blocks/%i.bin", punto_montaje_file_system, atoi(un_bloque_string_nuevo));

           	FILE* archivo_bloque_bin_nuevo = fopen(path_bloque_bin_nuevo, "wb");

           	if(strlen(cadena_a_grabar) >= tamanio_bloques_del_file_system)
           	{
               	fwrite(cadena_a_grabar, tamanio_bloques_del_file_system, 1, archivo_bloque_bin_nuevo);
           	}
           	else
           	{
               	fwrite(cadena_a_grabar, strlen(cadena_a_grabar), 1, archivo_bloque_bin_nuevo);
           	}
        	fflush(archivo_bloque_bin_nuevo);

           	fclose(archivo_bloque_bin_nuevo);

            i++;
            j = j + tamanio_bloques_del_file_system;

            free(cadena_a_grabar);
            free(path_bloque_bin_nuevo);
    	}
    }
    else
    {	//ESTO SE EJECUTA SI TENGO QUE PEDIR UN BLOQUE MAS
       	int archivo_bloque_actual = 0;

		int se_encontro_bit = -1;

       	while(archivo_bloque_actual < cantidad_de_bloques_necesaria)
       	{

           	char* cadena_a_grabar = string_substring(nuevo_archivo_pokemon_en_un_string, j, tamanio_bloques_del_file_system);

       		if(archivo_bloque_actual == (cantidad_de_bloques_necesaria - 1))
       		{	//ESTO SE EJECUTA SI ESTOY EN EL ULTIMO BLOQUE A GRABAR

       		    //**** SE BUSCA EL PRIMER BLOQUE DISPONIBLE  ****//
       			pthread_mutex_lock(MUTEX_METADATA_METADATA_BIN);
       			int cantidad_bloques_en_file_system = asignar_int_property(METADATA_METADATA_BIN, "BLOCKS");
       			pthread_mutex_unlock(MUTEX_METADATA_METADATA_BIN);

       			int bit;

   				pthread_mutex_lock(MUTEX_BITMAP);
       			for(bit = 0; bit < cantidad_bloques_en_file_system; bit++)
       			{
           			if(bitarray_test_bit(BITMAP, bit) == false)
           			{
           				se_encontro_bit = bit;
           				break;
           			}
       			}
       			pthread_mutex_unlock(MUTEX_BITMAP);

       			if(se_encontro_bit == -1)
       			{
       				imprimir_error_y_terminar_programa_perzonalizado("NO SE ENCONTRARON MAS BLOQUES LIBRES EN EL FILE SYSTEM. TERMINANDO PROGRAMA",
       						finalizar_gamecard, MUTEX_LOGGER);
                    free(cadena_a_grabar);
                    valor_retorno_funcion = -1;
                    break;
       			}

       			pthread_mutex_lock(MUTEX_BITMAP);
   				bitarray_set_bit(BITMAP, se_encontro_bit);
   				pthread_mutex_unlock(MUTEX_BITMAP);

   				char* string_bloques_sin_corchetes = string_substring(bloques_del_archivo_pokemon, 1, strlen(bloques_del_archivo_pokemon) - 2);
   				char* nuevo_value_blocks;

   				pthread_mutex_lock(MUTEX_LOGGER);
   				printf("string_bloques_sin_corchetes : %s\n", string_bloques_sin_corchetes);
   				fflush(stdout);
   				pthread_mutex_unlock(MUTEX_LOGGER);

   				if(strcmp(string_bloques_sin_corchetes, "") != 0)
   				{
   	   				nuevo_value_blocks = string_from_format("[%s,%i]", string_bloques_sin_corchetes, se_encontro_bit + 1);
   				}
   				else
   				{
   					nuevo_value_blocks = string_from_format("[%i]", se_encontro_bit + 1);
   				}
   				free(string_bloques_sin_corchetes);

   				pthread_mutex_lock(mutex_archivo_pokemon);
   		        config_set_value(archivo_pokemon_metadata_bin, "BLOCKS", nuevo_value_blocks);
   		        pthread_mutex_unlock(mutex_archivo_pokemon);

   		        free(nuevo_value_blocks);

               	char* path_bloque_con_nombre_nuevo = string_from_format("%s/Blocks/%i.bin", punto_montaje_file_system, se_encontro_bit + 1);
               	FILE* archivo_bloque_nuevo = fopen(path_bloque_con_nombre_nuevo, "wb");
               	free(path_bloque_con_nombre_nuevo);

               	fwrite(cadena_a_grabar, tamanio_bloques_del_file_system, 1, archivo_bloque_nuevo);
            	fflush(archivo_bloque_nuevo);
               	fclose(archivo_bloque_nuevo);

                free(cadena_a_grabar);
       			break;
       		}

       		int un_bloque_nuevo;

       		if(array_bloques_del_archivo_pokemon[archivo_bloque_actual] != NULL)
       		{
               	un_bloque_nuevo = atoi(array_bloques_del_archivo_pokemon[archivo_bloque_actual]);
       		}
       		else
       		{
       			un_bloque_nuevo = se_encontro_bit + 1;
       		}

           	char* path_bloque_con_nombre_nuevo = string_from_format("%s/Blocks/%i.bin", punto_montaje_file_system, un_bloque_nuevo);

           	FILE* archivo_bloque_nuevo = fopen(path_bloque_con_nombre_nuevo, "wb");
           	free(path_bloque_con_nombre_nuevo);

           	if(strlen(cadena_a_grabar) >= tamanio_bloques_del_file_system)
           	{
               	fwrite(cadena_a_grabar, tamanio_bloques_del_file_system, 1, archivo_bloque_nuevo);
           	}
           	else
           	{
               	fwrite(cadena_a_grabar, strlen(cadena_a_grabar), 1, archivo_bloque_nuevo);
           	}
        	fflush(archivo_bloque_nuevo);
           	fclose(archivo_bloque_nuevo);

           	archivo_bloque_actual++;
            j = j + tamanio_bloques_del_file_system;

            free(cadena_a_grabar);
       	}
    }

    i = 0;
    while(array_bloques_del_archivo_pokemon[i] != NULL)
    {
    	free(array_bloques_del_archivo_pokemon[i]);
    	i++;
    }
    free(array_bloques_del_archivo_pokemon);

    free(nuevo_archivo_pokemon_en_un_string);
    free(nuevo_value_SIZE_archivo_pokemon);

	pthread_mutex_lock(mutex_archivo_pokemon);
    config_save(archivo_pokemon_metadata_bin);
	config_destroy(archivo_pokemon_metadata_bin);
	pthread_mutex_unlock(mutex_archivo_pokemon);

	return valor_retorno_funcion;
}

void reducir_cantidad_en_archivo_pokemon(char* nombre_pokemon, int indice_de_busqueda, char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades, char* posicion_buscada_en_string)
{
    //**** SE REDUCE EN 1 LA CANTIDAD EN EL nuevo_archivo_pokemon_en_un_string  ****//

    char* nuevo_archivo_pokemon_en_un_string = string_new();

    int i = 1;

   	char* linea_de_posicion_encontrada = array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[indice_de_busqueda];
    char** key_y_value = string_split(linea_de_posicion_encontrada, "=");
    char* cantidad_anterior_de_pokemones = key_y_value[1];

    if(atoi(cantidad_anterior_de_pokemones) == 1)
    {	//ESTO SE EJECUTA SI HAY QUE BORRAR UNA LINEA

    	if(indice_de_busqueda != 0)
    	{
    		string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "%s", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[0]);

            while(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i] != NULL)
            {
            	if(indice_de_busqueda != i)
            		string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "\n%s", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i]);

               	i++;
            }
    	}
    	else
    	{
    		i = 2;

    		string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "%s", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[1]);

            while(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i] != NULL)
            {
               	string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "\n%s", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i]);
               	i++;
            }
    	}
    }
    else
    {	//ESTO SE EJECUTA SI SOLAMENTE HAY QUE REDUCIR LA CANTIDAD EN 1
        int cantidad_nueva_de_pokemones = atoi(cantidad_anterior_de_pokemones) - 1;

        free(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[indice_de_busqueda]);

        array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[indice_de_busqueda] = string_from_format("%s=%i", posicion_buscada_en_string, cantidad_nueva_de_pokemones);

      	string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "%s", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[0]);

        while(array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i] != NULL)
        {
           	string_append_with_format(&nuevo_archivo_pokemon_en_un_string, "\n%s", array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades[i]);
           	i++;
        }
    }
    free(key_y_value[0]);
    free(key_y_value[1]);
    free(key_y_value);


    //**** SE INTERPRETA SI HAY QUE ELIMINAR EL ULTIMO BLOQUE  ****//

	pthread_mutex_lock(MUTEX_CONFIG);
	char* punto_montaje_file_system = asignar_string_property(CONFIG, "PUNTO_MONTAJE_TALLGRASS");
	pthread_mutex_unlock(MUTEX_CONFIG);

	char* path_archivo_pokemon_metadata_bin = string_from_format("%s/Files/%s/Metadata.bin", punto_montaje_file_system, nombre_pokemon);

	pthread_mutex_lock(MUTEX_DICCIONARIO);
	pthread_mutex_t* mutex_archivo_pokemon = dictionary_get(DICCIONARIO_CON_MUTEX, path_archivo_pokemon_metadata_bin);
	pthread_mutex_unlock(MUTEX_DICCIONARIO);

	if(mutex_archivo_pokemon == NULL)
		imprimir_error_y_terminar_programa_perzonalizado("No se encontro un MUTEX en el DICCIONARIO", finalizar_gamecard, MUTEX_LOGGER);

	pthread_mutex_lock(mutex_archivo_pokemon);
	t_config* archivo_pokemon_metadata_bin = config_create(path_archivo_pokemon_metadata_bin);

	if(archivo_pokemon_metadata_bin == NULL)
	{
		char* mensaje_error = string_from_format("No se pudo abrir archivo %s en reducir_cantidad_en_archivo_pokemon()", path_archivo_pokemon_metadata_bin);
		imprimir_error_y_terminar_programa_perzonalizado(mensaje_error, finalizar_gamecard, MUTEX_LOGGER);
		free(mensaje_error); //ESTO NUNCA SE EJECUTA...
	}
	free(path_archivo_pokemon_metadata_bin);

	int tamanio_anterior_archivo_pokemon = asignar_int_property(archivo_pokemon_metadata_bin, "SIZE");
    char* nuevo_value_SIZE_archivo_pokemon = string_from_format("%i", strlen(nuevo_archivo_pokemon_en_un_string));
    config_set_value(archivo_pokemon_metadata_bin, "SIZE", nuevo_value_SIZE_archivo_pokemon);

	char* bloques_del_archivo_pokemon = asignar_string_property(archivo_pokemon_metadata_bin, "BLOCKS");
	char** array_bloques_del_archivo_pokemon = string_get_string_as_array(bloques_del_archivo_pokemon);
	pthread_mutex_unlock(mutex_archivo_pokemon);

	pthread_mutex_lock(MUTEX_METADATA_METADATA_BIN);
	int tamanio_bloques_del_file_system = asignar_int_property(METADATA_METADATA_BIN, "BLOCK_SIZE");
	pthread_mutex_unlock(MUTEX_METADATA_METADATA_BIN);

    int j = 0;
    i = 0;

    while(j < atoi(nuevo_value_SIZE_archivo_pokemon))
    {
    	char* cadena_a_grabar = string_substring(nuevo_archivo_pokemon_en_un_string, j, tamanio_bloques_del_file_system);

    	char* path_bloque_bin_nuevo = string_from_format("%s/Blocks/%i.bin", punto_montaje_file_system, atoi(array_bloques_del_archivo_pokemon[i]));

        FILE* archivo_bloque_bin_nuevo = fopen(path_bloque_bin_nuevo, "wb");
        free(path_bloque_bin_nuevo);

       	fwrite(cadena_a_grabar, tamanio_bloques_del_file_system, 1, archivo_bloque_bin_nuevo);
    	fflush(archivo_bloque_bin_nuevo);
        free(cadena_a_grabar);

       	fclose(archivo_bloque_bin_nuevo);

        i++;
        j = j + tamanio_bloques_del_file_system;
    }

    int primer_es_division_con_resto = 0;
    if((tamanio_anterior_archivo_pokemon % tamanio_bloques_del_file_system) != 0) primer_es_division_con_resto = 1;
    int cantidad_de_bloques_anterior = (tamanio_anterior_archivo_pokemon / tamanio_bloques_del_file_system) + primer_es_division_con_resto;

    int segundo_es_division_con_resto = 0;
    if((atoi(nuevo_value_SIZE_archivo_pokemon) % tamanio_bloques_del_file_system) != 0) segundo_es_division_con_resto = 1;
    int cantidad_de_bloques_necesaria = (atoi(nuevo_value_SIZE_archivo_pokemon) / tamanio_bloques_del_file_system) + segundo_es_division_con_resto;

    if(cantidad_de_bloques_anterior != cantidad_de_bloques_necesaria)
    {	//ESTO SE EJECUTA SI TENGO QUE LIBERAR EL ULTIMO BLOQUE

      	char* bloque_que_se_elimina_en_string = array_bloques_del_archivo_pokemon[cantidad_de_bloques_necesaria];
    	pthread_mutex_lock(MUTEX_BITMAP);
    	bitarray_clean_bit(BITMAP, atoi(bloque_que_se_elimina_en_string) - 1);
    	pthread_mutex_unlock(MUTEX_BITMAP);

    	char* nuevo_value_blocks_sin_corchetes = string_new();

    	if(cantidad_de_bloques_necesaria != 0)
    	{
        	string_append_with_format(&nuevo_value_blocks_sin_corchetes, "%s", array_bloques_del_archivo_pokemon[0]);

        	for(int i = 1; i < cantidad_de_bloques_necesaria; i++)
        	{
        		string_append_with_format(&nuevo_value_blocks_sin_corchetes, ",%s", array_bloques_del_archivo_pokemon[i]);
        	}
    	}

    	char* nuevo_value_blocks = string_from_format("[%s]", nuevo_value_blocks_sin_corchetes);
    	free(nuevo_value_blocks_sin_corchetes);

    	pthread_mutex_lock(mutex_archivo_pokemon);
    	config_set_value(archivo_pokemon_metadata_bin, "BLOCKS", nuevo_value_blocks);
    	pthread_mutex_unlock(mutex_archivo_pokemon);

    	free(nuevo_value_blocks);
    }

    i = 0;
    while(array_bloques_del_archivo_pokemon[i] != NULL)
    {
    	free(array_bloques_del_archivo_pokemon[i]);
    	i++;
    }
    free(array_bloques_del_archivo_pokemon);

    free(nuevo_archivo_pokemon_en_un_string);
    free(nuevo_value_SIZE_archivo_pokemon);

	pthread_mutex_lock(mutex_archivo_pokemon);
    config_save(archivo_pokemon_metadata_bin);
	config_destroy(archivo_pokemon_metadata_bin);
	pthread_mutex_unlock(mutex_archivo_pokemon);
}
