#include "game-card.h"

int main(void) {

	signal(SIGTERM, &executar_antes_de_terminar);
	signal(SIGINT, &executar_antes_de_terminar);

	CONFIG = leer_config("../game-card.config");
	LOGGER = generar_logger(CONFIG, "game-card");

	iniciar_file_system();

	pthread_t thread_new_appeared = iniciar_hilo_de_mensajes("NEW_POKEMON", SUBSCRIBE_NEW_POKEMON);

	pthread_t thread_catch_caught = iniciar_hilo_de_mensajes("CATCH_POKEMON", SUBSCRIBE_CATCH_POKEMON);

	pthread_t thread_get_localized = iniciar_hilo_de_mensajes("GET_POKEMON", SUBSCRIBE_GET_POKEMON);

	pthread_join(thread_new_appeared, NULL);
	pthread_join(thread_catch_caught, NULL);
	pthread_join(thread_get_localized, NULL);






	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);






	config_destroy(METADATA_METADATA_BIN);

    int retorno = msync(BITMAP->bitarray, BITMAP->size, MS_SYNC);

    if(retorno == -1) log_info(LOGGER, "[[ERROR]] Ocurrio un error al usar mysinc()");

	munmap(BITMAP->bitarray, BITMAP->size);

	terminar_programa(0, LOGGER, CONFIG);

	return EXIT_SUCCESS;
}

void executar_antes_de_terminar(int numero_senial)
{
	config_destroy(METADATA_METADATA_BIN);

	log_info(LOGGER, "Se recibio la senial : %i  -- terminando programa", numero_senial);

    int retorno = msync(BITMAP->bitarray, BITMAP->size, MS_SYNC);

    if(retorno == -1) log_info(LOGGER, "[[ERROR]] Ocurrio un error al usar mysinc()");

	munmap(BITMAP->bitarray, BITMAP->size);

	terminar_programa(0, LOGGER, CONFIG);

	exit(0);
}

pthread_t iniciar_hilo_de_mensajes(char* cola_a_suscribirse, op_code codigo_suscripcion)
{
	argumentos_de_hilo* arg = malloc(sizeof(argumentos_de_hilo));
	arg->mensaje = cola_a_suscribirse;
	arg->codigo_operacion = codigo_suscripcion;

	pthread_t thread;
	if(0 != pthread_create(&thread, NULL, conectar_recibir_y_enviar_mensajes, (void*)arg))
		imprimir_error_y_terminar_programa("No se pudo crear hilo conectar_recibir_y_enviar_mensaje()");

	return thread;
}

void* conectar_recibir_y_enviar_mensajes(void* argumentos)
{
	argumentos_de_hilo* args = argumentos;
	char* cola_a_suscribirse = args->mensaje;
	op_code codigo_suscripcion = args->codigo_operacion;

	int tiempo_de_reintento_conexion = asignar_int_property(CONFIG, "TIEMPO_DE_REINTENTO_CONEXION");

	while(true)
	{
		int conexion = iniciar_conexion_como_cliente("BROKER", CONFIG);

		while(conexion <= 0)
		{
			log_info(LOGGER, "La conexion con el BROKER para suscribirse a cola %s fallo. Reintentando conexion en : %i segundos", cola_a_suscribirse, tiempo_de_reintento_conexion);

			sleep(tiempo_de_reintento_conexion);

			conexion = iniciar_conexion_como_cliente("BROKER", CONFIG);
		}

		log_info(LOGGER, "Se realizo una conexion con BROKER, para suscribirse a cola %s", cola_a_suscribirse);

		if(enviar_mensaje_de_suscripcion(conexion, codigo_suscripcion) <= 0) continue;

		log_info(LOGGER, "Se realizo una suscripcion a la cola de mensajes %s del BROKER", cola_a_suscribirse);

		while(true)
		{
			//queda bloqueado hasta que el gameboy recibe un mensaje,
			void* mensaje = recibir_mensaje_como_cliente(conexion);	//GUARDA QUE NO TE DA LOS IDS DE MENSAJE TODO

			//si se recibe el mensaje de error (que se genera si se CAE EL BROKER), se sale de este while(true) y reintenta la conexion
			if(mensaje == NULL) break;

			//si no hay error, se envia ACK al BROKER
			if(enviar_ack(conexion, codigo_suscripcion) <= 0) break;
			//deberias intentar hacer la tarea asignada y reintentar enviar ACK (LUEGO DE VOLVER A CONECTAR...) TODO

			//estos ACK estan bien? revisar TODO
			//no deberia estar relacionado con el id_mensaje y el id_correlativo?

			//retorna un String con el mensaje a loguear.
			char* mensaje_para_loguear = generar_mensaje_para_loggear(mensaje, codigo_suscripcion);

			//se loguea el mensaje y vuelve a empezar el bucle
			log_info(LOGGER, "Se recibio el mensaje <<%s>> de la cola %s", mensaje_para_loguear, cola_a_suscribirse);

			free(mensaje_para_loguear);

			iniciar_hilo_para_tratar_y_responder_mensaje(mensaje, codigo_suscripcion);
		}
	}

	return NULL;
}

void iniciar_hilo_para_tratar_y_responder_mensaje(void* mensaje, op_code codigo_operacion)
{
	argumentos_de_hilo* arg = malloc(sizeof(argumentos_de_hilo));
	arg->mensaje = mensaje;
	arg->codigo_operacion = codigo_operacion;

	pthread_t thread;
	if(0 != pthread_create(&thread, NULL, tratar_y_responder_mensaje, (void*)arg))
		imprimir_error_y_terminar_programa("No se pudo crear hilo tratar_y_responder_mensaje()");

	pthread_detach(thread);
}

void* tratar_y_responder_mensaje(void* argumentos)
{
	//ESTE HILO SE VA A ENCARGAR DE REALIZAR LA TAREA ASIGNADA A LA RECEPCION DE ESE MENSAJE
	//LUEGO SE ENCARGARA DE ENVIARLO AL BROKER
	//DEBE ESPERAR A QUE EL BROKER LE RESPONDA Y DEBE GUARDAR LOS DATOS QUE EL BROKER LE BRINDE
	//SI TODO ESO VA BIEN, EL HILO FINALIZA
	//SI NO RECIBE RESPUESTA DEL BROKER, EL HILO VA REINTENTAR ENVIARLO HASTA QUE OBTENGA RESPUESTA DEL BROKER

	return NULL;
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
	free(path_metadata_bitmap_bin);
}

t_config* config_para_file_system(char* punto_montaje_file_system, char* path_archivo)
{
	char* archivo = string_from_format("%s%s", punto_montaje_file_system, path_archivo);
	t_config* config = leer_config(archivo);
	free(archivo);

	return config;
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

	    int retorno = msync(bitarray->bitarray, bitarray->size, MS_SYNC);
	    if(retorno == -1) imprimir_error_y_terminar_programa("Ocurrio un error al usar mysinc()");

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
	string_append_with_format(&path_files, "/Metadata.bin");
	escribir_y_cerrar_archivo_si_no_existe(path_files, "DIRECTORY=Y", strlen("DIRECTORY=Y"));
	free(path_files);
}

void crear_carpeta_si_no_existe(char* path_carpeta_con_nombre)
{
	DIR* carpeta = opendir(path_carpeta_con_nombre);

	if(carpeta == NULL)
	{
		mkdir(path_carpeta_con_nombre, 0777);
		log_info(LOGGER, "Creando carpeta : %s", path_carpeta_con_nombre);
	}else
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
		log_info(LOGGER, "Creando archivo : %s  --  Bytes escritos : %i", path_archivo_con_nombre, tamanio_datos_a_grabar);

	}else{
		fclose(archivo);
	}
}

void sobrescribir_y_cerrar_archivo(char* path_archivo_con_nombre, char* datos_a_grabar, int tamanio_datos_a_grabar)
{
	FILE* archivo = fopen(path_archivo_con_nombre, "wb");

	if(archivo == NULL)
	{
		char* mensaje = string_from_format("No se pudo crear archivo %s", path_archivo_con_nombre);
		imprimir_error_y_terminar_programa(mensaje);
		free(mensaje); //ESTO NUNCA SE EJECUTA...
	}

	//fprintf(metadata_bin, "%s", datos_metadata_bin);
	fwrite(datos_a_grabar, tamanio_datos_a_grabar, 1, archivo);

	fclose(archivo);
}

t_bitarray* mapear_bitmap_en_memoria(char* archivo, size_t size_memoria_a_mapear){

	int fd = open(archivo, O_RDWR);
	if (fd == -1)imprimir_error_y_terminar_programa("No se pudo abrir el archivo en mapear_bitmap_en_memoria()");

	ftruncate(fd, size_memoria_a_mapear);

	void* memoria_mapeada = mmap(NULL, size_memoria_a_mapear, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	close(fd);

	if (memoria_mapeada == MAP_FAILED) imprimir_error_y_terminar_programa("No se pudo realizar el mapeo en memoria del BITMAP");

	log_info(LOGGER, "Se realizo con exito el mapeado en memoria de : %s", archivo);

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

