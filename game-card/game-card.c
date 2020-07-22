#include "game-card.h"

int main(int argc, char *argv[]) {

	signal(SIGTERM, &executar_antes_de_terminar);
	signal(SIGINT, &executar_antes_de_terminar);

	CONFIG = leer_config("../game-card.config");
	LOGGER = generar_logger(CONFIG, "game-card");

	verificar_e_interpretar_entrada(argc, argv);

	iniciar_file_system();

	pthread_t thread_servidor = iniciar_hilo_servidor("ALGO", 0);	//TODO

	pthread_t thread_new_appeared = iniciar_hilo_de_mensajes("NEW_POKEMON", SUBSCRIBE_NEW_POKEMON);

	pthread_t thread_catch_caught = iniciar_hilo_de_mensajes("CATCH_POKEMON", SUBSCRIBE_CATCH_POKEMON);

	pthread_t thread_get_localized = iniciar_hilo_de_mensajes("GET_POKEMON", SUBSCRIBE_GET_POKEMON);

	pthread_join(thread_servidor, NULL);
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

void verificar_e_interpretar_entrada(int argc, char *argv[])
{
	if(argc > 1 ){
		error_show(" Ingresaste mas argumentos de los necesarios.\n"
				"Solo se puede ingresar un argumento, el ID_MANUAL_DEL_PROCESO, que es opcional.\n"
				"Si no se ingresa, se setea en 0\n\n");
		exit(-1);
	}

	if(argc == 1) ID_MANUAL_DEL_PROCESO = atoi(argv[1]);
}

pthread_t iniciar_hilo_de_mensajes(char* cola_a_suscribirse, op_code codigo_suscripcion)
{
	argumentos_de_hilo* arg = malloc(sizeof(argumentos_de_hilo));
	arg->stream = cola_a_suscribirse;
	arg->entero = codigo_suscripcion;

	pthread_t thread;
	if(0 != pthread_create(&thread, NULL, conectar_recibir_y_enviar_mensajes, (void*)arg))
		imprimir_error_y_terminar_programa("No se pudo crear hilo conectar_recibir_y_enviar_mensaje()");

	return thread;
}

void* conectar_recibir_y_enviar_mensajes(void* argumentos)
{
	argumentos_de_hilo* args = argumentos;
	char* cola_a_suscribirse = args->stream;
	op_code codigo_suscripcion = args->entero;

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

		int estado_envio = enviar_mensaje_de_suscripcion(conexion, codigo_suscripcion, ID_MANUAL_DEL_PROCESO);
		if(estado_envio <= 0) continue;
		// || estado_envio != (sizeof(op_code) + sizeof(int)) revisar si tenes tiempo, TODO
		//si se manda el mensaje incompleto, que deberia hacer? seria algo poco probable, no? capaz ni hace falta considerar esto
		//CREERIA QUE, SI PASA, DEBERIA TIRAR UN ERROR Y TERMINAR PROGRAMA... NO TENDRIA SENTIDO CONTINUAR

		//HACE FALTA RECIBIR ACK DEL MENSAJE DE SUSCRIPCION?? TODO

		log_info(LOGGER, "Se realizo una suscripcion a la cola de mensajes %s del BROKER", cola_a_suscribirse);

		while(true)
		{
			int id_correlativo = 0;
			int id_mensaje_recibido = 0; 	//EL GAMECARD SOLO USA ID MENSAJE

			//queda bloqueado hasta que el gameboy recibe un mensaje,
			void* mensaje = recibir_mensaje_como_cliente(conexion, &id_correlativo, &id_mensaje_recibido);

			//si se recibe el mensaje de error (que se genera si se CAE EL BROKER), se sale de este while(true) y reintenta la conexion
			if(mensaje == NULL) break;

			enviar_ack(conexion, id_mensaje_recibido); //NO VERIFICO EL ESTADO PORQUE NO IMPORTA, EL GAMECARD VA A REALIZAR SU TAREA ASIGNADA DE TODAS FORMAS
			//ADEMAS EL BROKER, COMO PERDIO TODOS LOS MENSAJES QUE TENIA EN MEMORIA, NO VA A REENVIAR NADA A GAMECARD

			char* mensaje_para_loguear = generar_mensaje_para_loggear(mensaje, codigo_suscripcion);
			log_info(LOGGER, "Se recibio el mensaje <<%s>> de la cola %s", mensaje_para_loguear, cola_a_suscribirse);
			free(mensaje_para_loguear);

			iniciar_hilo_para_tratar_y_responder_mensaje(id_mensaje_recibido, mensaje, codigo_suscripcion);
		}
	}

	return NULL;
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
					imprimir_error_y_terminar_programa("No se pudo crear hilo para atender_new_pokemon()");
				break;

			case SUBSCRIBE_CATCH_POKEMON:
				if(0 != pthread_create(&thread, NULL, atender_catch_pokemon, (void*)arg))
					imprimir_error_y_terminar_programa("No se pudo crear hilo para atender_catch_pokemon()");
				break;

			case SUBSCRIBE_GET_POKEMON:
				if(0 != pthread_create(&thread, NULL, atender_get_pokemon, (void*)arg))
					imprimir_error_y_terminar_programa("No se pudo crear hilo para atender_get_pokemon()");
				break;
			}

	pthread_detach(thread);
}

void* tratar_y_responder_mensaje(void* argumentos)
{
	//ESTE HILO SE VA A ENCARGAR DE REALIZAR LA TAREA ASIGNADA A LA RECEPCION DE ESE MENSAJE
	//LUEGO SE ENCARGARA DE ENVIARLO AL BROKER
	//DEBE ESPERAR A QUE EL BROKER LE MANDE ID MENSAJE...QUE ME SIRVE DE ACK...
	//SI TODO ESO VA BIEN, EL HILO FINALIZA
	//SI NO RECIBE RESPUESTA DEL BROKER, EL HILO VA REINTENTAR ENVIARLO HASTA QUE OBTENGA RESPUESTA DEL BROKER. O NO? porque hay que cambiar el socket...
	//por ahora, si no se puede enviar, terminas el hilo

	return NULL;
}

void* atender_new_pokemon(void* argumentos)
{
	argumentos_de_hilo* args = argumentos;
	int id_mensaje_recibido = args->entero;
	t_new_pokemon* mensaje = args->stream;

	if(verificar_si_existe(mensaje->nombre) == false) crear_archivo(mensaje->nombre);

	int tiempo_de_reintento_operacion = asignar_int_property(CONFIG, "TIEMPO_DE_REINTENTO_OPERACION");

	bool se_puede_abrir = verificar_si_se_puede_abrir(mensaje->nombre);

	while(se_puede_abrir == false)
	{
		sleep(tiempo_de_reintento_operacion);

		se_puede_abrir = verificar_si_se_puede_abrir(mensaje->nombre);
	}

	agregar_cantidad(mensaje->nombre);

	int tiempo_de_retardo_operacion = asignar_int_property(CONFIG, "TIEMPO_RETARDO_OPERACION");

	sleep(tiempo_de_retardo_operacion);

	liberar_archivo(mensaje->nombre);

	int conexion = iniciar_conexion_como_cliente("BROKER", CONFIG);

	int tiempo_de_reintento_conexion = asignar_int_property(CONFIG, "TIEMPO_DE_REINTENTO_CONEXION");

	while(conexion <= 0)
	{
		log_info(LOGGER, "La conexion con el BROKER para enviar appeared_pokemon fallo. Reintentando conexion en : %i segundos", tiempo_de_reintento_conexion);

		sleep(tiempo_de_reintento_conexion);

		conexion = iniciar_conexion_como_cliente("BROKER", CONFIG);
	}

	int estado = generar_y_enviar_appeared_pokemon(conexion, 0, id_mensaje_recibido, mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);
	/*
	Verificar si el Pokémon existe dentro de nuestro Filesystem. Para esto se deberá buscar dentro del directorio Pokemon si existe el archivo con el nombre de nuestro pokémon. En caso de no existir se deberá crear.
	Verificar si se puede abrir el archivo (si no hay otro proceso que lo esté abriendo). En caso que el archivo se encuentre abierto se deberá reintentar la operación luego de un tiempo definido en el archivo de configuración.
	Verificar si las posiciones ya existen dentro del archivo. En caso de existir, se deben agregar la cantidad pasada por parámetro a la actual. En caso de no existir se debe agregar al final del archivo una nueva línea indicando la cantidad de pokémon pasadas.
	Esperar la cantidad de segundos definidos por archivo de configuración
	Cerrar el archivo.
	Conectarse al Broker y enviar el mensaje a la Cola de Mensajes APPEARED_POKEMON con los los datos:
	ID del mensaje recibido.
	Pokemon.
	Posición del mapa.
	En caso que no se pueda realizar la conexión con el Broker se debe informar por logs y continuar la ejecución.
	*/

	return NULL;
}

void* atender_catch_pokemon(void* argumentos)
{
	argumentos_de_hilo* args = argumentos;
	int id_mensaje_recibido = args->entero;
	t_catch_pokemon* mensaje = args->stream;

	if(verificar_si_existe(mensaje->nombre) == false)
		log_info(LOGGER, "ERROR : Se recibio catch_pokemon. No existe pokemon solicitado en file system. Nombre del pokemon: %s", mensaje->nombre);

	int tiempo_de_reintento_operacion = asignar_int_property(CONFIG, "TIEMPO_DE_REINTENTO_OPERACION");

	bool se_puede_abrir = verificar_si_se_puede_abrir(mensaje->nombre);

	while(se_puede_abrir == false)
	{
		sleep(tiempo_de_reintento_operacion);

		se_puede_abrir = verificar_si_se_puede_abrir(mensaje->nombre);
	}

	if(verificar_si_existen_posiciones(mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy) == false)
		log_info(LOGGER, "ERROR : Se recibio catch_pokemon. No existe pokemon en ubicacion solicitada en file system. "
				"Nombre del pokemon: %s posx: %i posy %i", mensaje->nombre, mensaje->coordenadas.posx, mensaje->coordenadas.posy);

	int resultado_caught = evaluar_catch_y_reducir_cantidad(mensaje->nombre);

	int tiempo_de_retardo_operacion = asignar_int_property(CONFIG, "TIEMPO_RETARDO_OPERACION");

	sleep(tiempo_de_retardo_operacion);

	liberar_archivo(mensaje->nombre);

	int conexion = iniciar_conexion_como_cliente("BROKER", CONFIG);

	int tiempo_de_reintento_conexion = asignar_int_property(CONFIG, "TIEMPO_DE_REINTENTO_CONEXION");

	while(conexion <= 0)
	{
		log_info(LOGGER, "La conexion con el BROKER para enviar caught_pokemon fallo. Reintentando conexion en : %i segundos", tiempo_de_reintento_conexion);

		sleep(tiempo_de_reintento_conexion);

		conexion = iniciar_conexion_como_cliente("BROKER", CONFIG);
	}

	int estado = generar_y_enviar_caught_pokemon(conexion, 0, id_mensaje_recibido, resultado_caught);
	/*
	Este mensaje cumplirá la función de indicar si es posible capturar un Pokemon, y capturarlo en tal caso. Para esto se recibirán los siguientes parámetros:
	ID del mensaje recibido.
	Pokemon a atrapar.
	Posición del mapa.

	Verificar si el Pokémon existe dentro de nuestro Filesystem. Para esto se deberá buscar dentro del directorio Pokemon, si existe el archivo con el nombre de nuestro pokémon. En caso de no existir se deberá informar un error.
	Verificar si se puede abrir el archivo (si no hay otro proceso que lo esté abriendo). En caso que el archivo se encuentre abierto se deberá reintentar la operación luego de un tiempo definido en el archivo de configuración.
	Verificar si las posiciones ya existen dentro del archivo. En caso de no existir se debe informar un error.
	En caso que la cantidad del Pokémon sea “1”, se debe eliminar la línea. En caso contrario se debe decrementar la cantidad en uno.
	Esperar la cantidad de segundos definidos por archivo de configuración
	Cerrar el archivo.
	Conectarse al Broker y enviar el mensaje indicando el resultado correcto.
	Todo resultado, sea correcto o no, deberá realizarse conectandose al Broker y enviando un mensaje a la Cola de Mensajes CAUGHT_POKEMON indicando:
	ID del mensaje recibido originalmente.
	Resultado.
	En caso que no se pueda realizar la conexión con el Broker se debe informar por logs y continuar la ejecución.
	*/

	return NULL;
}

void* atender_get_pokemon(void* argumentos)
{
	argumentos_de_hilo* args = argumentos;
	int id_mensaje_recibido = args->entero;
	t_get_pokemon* mensaje = args->stream;

	/*
	Este mensaje cumplirá la función de obtener todas las posiciones y su cantidad de un Pokémon específico. Para esto recibirá:
	El identificador del mensaje recibido.
	Pokémon a devolver.
	Al recibir este mensaje se deberán realizar las siguientes operaciones:
	Verificar si el Pokémon existe dentro de nuestro Filesystem. Para esto se deberá buscar dentro del directorio Pokemon, si existe el archivo con el nombre de nuestro pokémon. En caso de no existir se deberá informar el mensaje sin posiciones ni cantidades.
	Verificar si se puede abrir el archivo (si no hay otro proceso que lo esté abriendo). En caso que el archivo se encuentre abierto se deberá reintentar la operación luego de un tiempo definido por configuración.
	Obtener todas las posiciones y cantidades de Pokemon requerido.
	Esperar la cantidad de segundos definidos por archivo de configuración
	Cerrar el archivo.
	Conectarse al Broker y enviar el mensaje con todas las posiciones y su cantidad.
	En caso que se encuentre por lo menos una posición para el Pokémon solicitado se deberá enviar un mensaje al Broker a la Cola de Mensajes LOCALIZED_POKEMON indicando:
	ID del mensaje recibido originalmente.
	El Pokémon solicitado.
	La lista de posiciones y la cantidad de posiciones X e Y de cada una de ellas en el mapa.
	En caso que no se pueda realizar la conexión con el Broker se debe informar por logs y continuar la ejecución.
	*/
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

