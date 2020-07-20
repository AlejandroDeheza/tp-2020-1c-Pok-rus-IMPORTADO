#include "game-card.h"

int main(void) {

	signal(SIGTERM, &executar_antes_de_terminar);
	signal(SIGINT, &executar_antes_de_terminar);

	CONFIG = leer_config("../game-card.config");
	LOGGER = generar_logger(CONFIG, "game-card");

	iniciar_file_system();

/*
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);

	int conex_appeared = 0;
	pthread_t thread_appeared;

	int conex_catch = 0;
	pthread_t thread_catch;

	int conex_get = 0;
	pthread_t thread_get;

	//log_info(logger, "Suscribiendo a las colas de mensajes");

	int op_code = 0;


	//void suscribir(t_config* config) {
	//	suscribirse_a_broker(config, "BROKER", op_code);
	//	log_info(logger, "Se realizo una suscripcion a la cola de mensajes ...");
	//}




	conex_catch = suscribirse_a_broker(CONFIG, SUBSCRIBE_CATCH_POKEMON);
	log_info(LOGGER, "Se realizo una suscripcion a la cola de mensajes CATCH_POKEMON");
	sleep(1);

	conex_appeared = suscribirse_a_broker(CONFIG, SUBSCRIBE_APPEARED_POKEMON);
	log_info(LOGGER, "Se realizo una suscripcion a la cola de mensajes APPEARED_POKEMON");
	sleep(1);

	conex_get = suscribirse_a_broker(CONFIG, SUBSCRIBE_GET_POKEMON);
	log_info(LOGGER, "Se realizo una suscripcion a la cola de mensajes GET_POKEMON");

	void* recibir_y_dar_ack(void* arg){

		int socket_cliente = *((int*)arg);

		pthread_mutex_lock(&mutex);
		void* response = recibir_mensaje_como_cliente(socket_cliente);
		pthread_mutex_unlock(&mutex);
		if(response == NULL){
			pthread_exit(NULL);
		}
		log_info(LOGGER, "recibio mensaje");
		enviar_ack(socket_cliente, op_code);
		log_info(LOGGER, "envio Ack");// creo que no hay logs obligatorios para gamecard
		return response;
	}

	while(1){

		pthread_create(&thread_appeared,NULL,(void*)recibir_y_dar_ack, (void*)&conex_appeared);
		pthread_detach(thread_appeared);

		pthread_create(&thread_get,NULL,(void*)recibir_y_dar_ack, (void*)&conex_get);
		pthread_detach(thread_get);

		pthread_create(&thread_catch,NULL,(void*)recibir_y_dar_ack, (void*)&conex_catch);
		pthread_detach(thread_catch);

	}

	terminar_programa(conex_appeared, LOGGER, CONFIG);
	terminar_programa(conex_catch, LOGGER, CONFIG);
	terminar_programa(conex_get, LOGGER, CONFIG);
	*/
    int retorno = msync(BITMAP->bitarray, BITMAP->size, MS_SYNC);

    if(retorno == -1) log_info(LOGGER, "[[ERROR]] Ocurrio un error al usar mysinc()");

	munmap(BITMAP->bitarray, BITMAP->size);

	terminar_programa(0, LOGGER, CONFIG);

	return EXIT_SUCCESS;
}

void executar_antes_de_terminar(int numero_senial)
{
	log_info(LOGGER, "Se recibio la senial : %i  -- terminando programa", numero_senial);

    int retorno = msync(BITMAP->bitarray, BITMAP->size, MS_SYNC);

    if(retorno == -1) log_info(LOGGER, "[[ERROR]] Ocurrio un error al usar mysinc()");

	munmap(BITMAP->bitarray, BITMAP->size);

	terminar_programa(0, LOGGER, CONFIG);

	exit(0);
}

void iniciar_file_system()
{
	char* punto_montaje_file_system = asignar_string_property(CONFIG, "PUNTO_MONTAJE_TALLGRASS");
	if(punto_montaje_file_system == NULL) imprimir_error_y_terminar_programa("Parece que PUNTO_MONTAJE_TALLGRASS no esta en game-card.config");

	generar_estructura_file_system_si_hace_falta(punto_montaje_file_system);

	t_config* metadata_metadata_bin = config_para_file_system(punto_montaje_file_system, "/Metadata/Metadata.bin");

	int cant_max_bloques = asignar_int_property(metadata_metadata_bin, "BLOCKS");

	char* path_metadata_bitmap_bin = string_from_format("%s/Metadata/Bitmap.bin", punto_montaje_file_system);

	BITMAP = mapear_bitmap_en_memoria(path_metadata_bitmap_bin, cant_max_bloques/8);
	free(path_metadata_bitmap_bin);

	//bitarray_c_bit(BITMAP, 56);
	//bitarray_set_bit(BITMAP, 83);

	printf("\n%i\n", bitarray_test_bit(BITMAP, 56));
	printf("\n%i\n", bitarray_test_bit(BITMAP, 83));
	printf("\n%i\n", bitarray_test_bit(BITMAP, 0));
	printf("\n%i\n", bitarray_test_bit(BITMAP, 3));

	config_destroy(metadata_metadata_bin);
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






int suscribirse_a_broker(t_config* config, op_code nombre_cola)
{
	int conexion = iniciar_conexion_como_cliente("BROKER", config);

	if(conexion <= 0)
	{
		imprimir_error_y_terminar_programa("Error de conexion en suscribirse_a_broker()");
	}

	enviar_mensaje_de_suscripcion(conexion, nombre_cola);

	return conexion;
};
