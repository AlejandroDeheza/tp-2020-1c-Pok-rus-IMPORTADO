/*
 ============================================================================
 Name        : team.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "team.h"


int ID_MANUAL_DEL_PROCESO_TEAM = 0;
//--------------------------------------------------------------------------
// Constantes
//--------------------------------------------------------------------------
const int RAFAGAS_CATCH = 1;




int main(int argc, char *argv[]) {

	signal(SIGINT, &ejecutar_antes_de_terminar);

	verificar_e_interpretar_entrada(argc, argv);

	CONFIG = leer_config("../team.config");

	inicializarGlobales();

	LOGGER = generar_logger(CONFIG, "team");

	//---------------------------------------------------------------------------------------
	// Creacion de hilo planificador
	//---------------------------------------------------------------------------------------
	pthread_t planificador;
	pthread_create(&planificador,NULL,(void*)planificar, NULL);
	pthread_detach(planificador);

	//---------------------------------------------------------------------------------------
	// Obtiene los entrenadores del config, crea sus hilos y los carga en la lista de NEW
	//---------------------------------------------------------------------------------------
	obtenerEntrenadores();

	//---------------------------------------------------------------------------------------
	// Carga los objetivos globales del team y una lista identica de pokemones pendientes
	// Durante el proceso, se trabajará sobre la lista de pendientes
	//---------------------------------------------------------------------------------------
	cargarObjetivoGlobal();
	//---------------------------------------------------------------------------------------
	// Realizo conexion de prueba con el Broker para validar el estado previo a procesar
	//--------------------------------------------------------------------------------------
	conexion_inicial();

	//---------------------------------------------------------------------------------------
	// Escucho conexion con Gameboy para proceso default (en caso de ser necesario)
	//---------------------------------------------------------------------------------------
	pthread_t conexion_con_gameboy;
	pthread_create(&conexion_con_gameboy, NULL, (void*)escuchar_conexion, NULL);
	pthread_detach(conexion_con_gameboy);

	//---------------------------------------------------------------------------------------
	// Envia los mensajes GET al broker (1 por especie)
	//---------------------------------------------------------------------------------------
	enviar_get_pokemones_requeridos();

	//---------------------------------------------------------------------------------------
	// Suscripcion a las colas de mensajeria del broker
	//---------------------------------------------------------------------------------------
	suscribirse_a_colas();

	//---------------------------------------------------------------------------------------
	// Proceso hasta que se cumple el objetivo
	//---------------------------------------------------------------------------------------
	while(objetivo_pokemones_pendientes->elements_count > 0){

	}
	terminar_proceso();


	exit(0);

}

void ejecutar_antes_de_terminar(int numero_senial)
{
	log_info(LOGGER, "Se recibio la senial : %i  -- terminando programa", numero_senial);

	terminar_programa(0, LOGGER, CONFIG, NULL);	//TODO
	//TODO DEBERIA PASARLE UN MUTEX, NO NULL

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

	if(argc == 1) ID_MANUAL_DEL_PROCESO_TEAM = atoi(argv[1]);
}

//----------------------------------------------------------------------------------------------------------
// Crea las listas que se van a utilizar globalmente en el proceso
// Inicializa los semfotos mutex
//----------------------------------------------------------------------------------------------------------
void inicializarGlobales(){
	objetivo_global = list_create();
	entrenadores_new = list_create();
	entrenadores_ready = list_create();
	entrenadores_blocked_sin_espera = list_create();
	entrenadores_blocked_espera = list_create();
	entrenadores_exec = list_create();

	conexion_con_broker = NULO;
	conexion_con_gameboy = DESCONECTADO;

	pthread_mutex_init(&mutex_entrenadores_ready, NULL);
	//pthread_mutex_init(&mutex_hilos, NULL);
}

//----------------------------------------------------------------------------------------------------------
// Funcion para obtener los entrenadores desde el archivo de configuracion
// Por cada uno:
//    1. Se crea un t_entrenador
//    2. Se crea un tcb y un hilo en donde ejecutaŕa el mismo
//    3. Se asigna un semaforo (inicializado y bloqueado) a ese hilo para que el proceso no ejecute.
//    4. Se agrega tcb a la lista de entrenadores en estado new
//----------------------------------------------------------------------------------------------------------
void obtenerEntrenadores(){

	char* posicionesEntrenadores = asignar_string_property(CONFIG, "POSICIONES_ENTRENADORES");
	char* pokemonesEntrenadores = asignar_string_property(CONFIG, "POKEMON_ENTRENADORES");
	char* objetivosEntrenadores = asignar_string_property(CONFIG, "OBJETIVOS_ENTRENADORES");

	char** coodenadasEntrenador = formatearPropiedadDelConfig(posicionesEntrenadores);
	char** pokemonEntrenador = formatearPropiedadDelConfig(pokemonesEntrenadores);
	char** objetivoEntrenador = formatearPropiedadDelConfig(objetivosEntrenadores);

	int i = 0;
	while(coodenadasEntrenador[i]!= NULL){
		t_entrenador* entrenador = crearEntrenador(coodenadasEntrenador[i], pokemonEntrenador[i], objetivoEntrenador[i]);

		t_entrenador_tcb* tcb_entrenador = malloc(sizeof(t_entrenador_tcb));

		pthread_mutex_t mutex;
		pthread_mutex_init(&mutex, NULL);
		pthread_mutex_lock(&mutex);

		pthread_t thread;
		pthread_create(&thread,NULL,(void*)hilo_entrenador, tcb_entrenador);
		pthread_detach(thread);

		tcb_entrenador->id_hilo_entrenador = thread;
		tcb_entrenador->entrenador = entrenador;
		tcb_entrenador->mutex = mutex;
		tcb_entrenador->nombre_pokemon = NULL;
		tcb_entrenador->rafagas = 0;

		list_add(entrenadores_new, tcb_entrenador);

		i++;
	}

}

//----------------------------------------------------------------------------------------------------------
// Elimina un caracter de una cadena
//----------------------------------------------------------------------------------------------------------
void removeChar(char *str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

//----------------------------------------------------------------------------------------------------------
// Separa las propiedades del entrenador que vienen en listas
//----------------------------------------------------------------------------------------------------------
char** formatearPropiedadDelConfig(char* propiedad){
	removeChar(propiedad, '[');
	removeChar(propiedad, ']');

	return string_split(propiedad,",");
}

//----------------------------------------------------------------------------------------------------------
// Funcion para crear un entrenador con sus coordenadas, pokemones y objetivos
//----------------------------------------------------------------------------------------------------------
t_entrenador* crearEntrenador(char* coordenadas, char* pokemones, char* objetivos){
	t_entrenador* entrenador = malloc(sizeof(t_entrenador));

	char** xy = string_split(coordenadas,"|");

	entrenador->coordenadas.posx = atoi(xy[0]);
	entrenador->coordenadas.posy = atoi(xy[1]);

	t_list* lista_objetivos = armarLista(string_split(objetivos,"|"));
	t_list* lista_pokemones = armarLista(string_split(pokemones, "|"));

	entrenador->objetivo_entrenador = lista_objetivos;
	entrenador->pokemones_entrenador = lista_pokemones;

	return entrenador;
}

//----------------------------------------------------------------------------------------------------------
// A partir de una array de pokemones, devuelve una lista
//----------------------------------------------------------------------------------------------------------
t_list* armarLista(char** objetos){

	t_list* lista = list_create();

	int i = 0;
	while(objetos[i] != NULL){
		char* objeto = malloc(strlen(objetos[i]) + 1);
		memcpy(objeto, objetos[i], strlen(objetos[i]) + 1);
		list_add(lista, objeto);
		i++;
	}

	free(objetos);

	return lista;
}

//----------------------------------------------------------------------------------------------------------
// Funcion para la ejecucion de cada proceso entrenador
// Por cada ciclo, espera el semaforo del hilo. El planificador es el que realiza el signal
// Si no coincide la posicion del entrendaor con la del pokemon a capturar, se mueve.
//    - Primero de manera horizontal, luego vertical
//    - 1 movimiento por ciclo
// Si coincide, realiza el catch
//----------------------------------------------------------------------------------------------------------
void hilo_entrenador(t_entrenador_tcb* tcb_entrenador){

	while(1){

		pthread_mutex_lock(&(tcb_entrenador->mutex));

		if(!coincidenCoordenadas(tcb_entrenador->coordenadas_del_pokemon, tcb_entrenador->entrenador->coordenadas)){
			t_coordenadas coordenada_previa = tcb_entrenador->entrenador->coordenadas;
			mover_entrenador(&(tcb_entrenador->entrenador->coordenadas), tcb_entrenador->coordenadas_del_pokemon);
			t_coordenadas coordenada_nueva = tcb_entrenador->entrenador->coordenadas;
			log_info(LOGGER, "Muevo entrenador desde [%d, %d] hasta [%d, %d]", coordenada_previa.posx, coordenada_previa.posy, coordenada_nueva.posx, coordenada_nueva.posy);
		} else {
			int conexion = 0;
			conexion = conectarse_a_broker();
			if(conexion_con_broker == CONECTADO){
				realizar_catch(conexion, tcb_entrenador->nombre_pokemon, tcb_entrenador->coordenadas_del_pokemon);
			}
		}
	}
}

void realizar_catch(int conexion, char* pokemon, t_coordenadas coordenadas){
	log_info(LOGGER, "Envio mensaje CATCH al Broker");
	//enviar_catch_pokemon(conexion,0, 0, pokemon, coordenadas.posx, coordenadas.posy);
	// TODO recibir mensaje asociado y guardarlo
}

//----------------------------------------------------------------------------------------------------------
// Chequea si el pokemon y el entrenador estan en la misma posicion
//----------------------------------------------------------------------------------------------------------
bool coincidenCoordenadas(t_coordenadas coordenadas_entrenador, t_coordenadas coordenadas_pokemon){
	return (coordenadas_entrenador.posx == coordenadas_pokemon.posx) && (coordenadas_entrenador.posy == coordenadas_pokemon.posy);
}

//----------------------------------------------------------------------------------------------------------
// Evalua movimiento, primero coordenada x, luego y
//----------------------------------------------------------------------------------------------------------
void mover_entrenador(t_coordenadas* coordenadas_entrenador, t_coordenadas coordenadas_pokemon){
	if(coordenadas_entrenador->posx != coordenadas_pokemon.posx){
		mover_una_posicion(&(coordenadas_entrenador->posx), coordenadas_pokemon.posx);
	} else {
		mover_una_posicion(&(coordenadas_entrenador->posy), coordenadas_pokemon.posy);
	}
}

//----------------------------------------------------------------------------------------------------------
// Suma o resta 1 a la posicion para acercarse
//----------------------------------------------------------------------------------------------------------
void mover_una_posicion(int* coordenada_entrenador, int coordenada_pokemon){

	if(*coordenada_entrenador > coordenada_pokemon){
		*coordenada_entrenador -= 1;
	} else {
		*coordenada_entrenador += 1;
	}
}

//----------------------------------------------------------------------------------------------------------
// Finalizar programa
//----------------------------------------------------------------------------------------------------------
void terminar_proceso()
{
	if (LOGGER != NULL){
		log_destroy(LOGGER);
	}

	if (CONFIG != NULL) {
		config_destroy(CONFIG);
	}

	if (socket_appeared != 0) {
		close(socket_caught);
	}

	if (socket_localized != 0) {
		close(socket_localized);

	}

	if (socket_caught != 0) {
		close(socket_caught);
	}

	printf("Finalizo programa.\n");
}

//----------------------------------------------------------------------------------------------------------
// Devuelve el primer indice que coincida con el pokeom buscado
//----------------------------------------------------------------------------------------------------------
int buscar_indice_pokemon(char*pokemon, t_list* lista){

	t_link_element* pokemon_lista = lista->head;

	int index = 0;

	while(pokemon_lista != NULL){
		if(strcmp(pokemon, pokemon_lista->data) == 0){
			return index;
		}

		index ++;
		pokemon_lista = pokemon_lista->next;
	}
	return -1;

}

//----------------------------------------------------------------------------------------------------------
// Carga el objetivo global del proceso
// Recorre todos los entrenadores cargados y carga sus objetivos en la lista global
// Una vez finalizado, copia la lista a la de pokemones pendientes
// Elimina de la lista de pendientes los pokemon que ya tienen los entrenadores
//----------------------------------------------------------------------------------------------------------
void cargarObjetivoGlobal(){

	void cargo_desde_pokemon_entrenador(void* elemento){
		t_entrenador_tcb* tcb_entrenador = (t_entrenador_tcb*) elemento;
		t_link_element* pokemon_entrenador = tcb_entrenador->entrenador->pokemones_entrenador->head;

		while(pokemon_entrenador != NULL){
			char* pokemon = pokemon_entrenador->data;
			int index = buscar_indice_pokemon(pokemon, objetivo_pokemones_pendientes);

			if(index >= 0){
				list_remove(objetivo_pokemones_pendientes, index);
			}

			pokemon_entrenador = pokemon_entrenador->next;
		}
	}

	void cargo_desde_objetivo_entrenador(void* elemento){
		t_entrenador_tcb* tcb_entrenador = (t_entrenador_tcb*) elemento;
		list_add_all(objetivo_global, tcb_entrenador->entrenador->objetivo_entrenador);
	}

	list_iterate(entrenadores_new, cargo_desde_objetivo_entrenador);

	objetivo_pokemones_pendientes = list_duplicate(objetivo_global);

	list_iterate(entrenadores_new, cargo_desde_pokemon_entrenador);

}

//---------------------------------------------------------------------------------------------------------
// Al inicio del proceso intento una conexion con el Broker para validar el estado del mismo
// Actualiza la variable global conexion_con_broker
// Libero esa conexion
//---------------------------------------------------------------------------------------------------------
void conexion_inicial(){
	int conexion = 0;
	conexion = conectarse_a_broker();

	if(conexion_con_broker == CONECTADO) {
		log_info(LOGGER, "Estado de conexion inicial con Broker: CONECTADO");
	} else {
		log_info(LOGGER, "Estado de conexion inicial con Broker: DESCONECTADO");
	}

	close(conexion);
}

//----------------------------------------------------------------------------------------------------------
// Evalua si esa especie ya fue solicitada con un GET al broker
//----------------------------------------------------------------------------------------------------------
bool envie_pokemon(char* pokemon, t_list* get_enviados){

	t_link_element* especie = get_enviados->head;

	while(especie != NULL){
		if(strcmp(pokemon, especie->data) == 0){
			return true;
		}
		especie = especie->next;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------
// Itera la lista para enviar los GET requeridos al BROKER
// Por cada especie
//    1. Abre una conexion
//    2. Envia mensaje
//    3. Cierra esa conexion
//    4. Guarda la especie como enviada
// Luego libera la memoria solicitada
//----------------------------------------------------------------------------------------------------------
void enviar_get_pokemones_requeridos(){
	t_list* get_enviados;
	get_enviados = list_create();

	int solicitados = 0;

	void envio_un_get(void* pokemon){
		int conexion = 0;
		conexion = conectarse_a_broker();

		char* especie_pokemon = (char*) pokemon;

		if((conexion_con_broker == CONECTADO) && (!envie_pokemon(especie_pokemon, get_enviados))){
			log_info(LOGGER, "Envio GET para la especie %s", especie_pokemon);
			generar_y_enviar_get_pokemon(conexion, 0, 0, especie_pokemon);
			close(conexion);
			log_info(LOGGER, "Libero conexion");
			list_add(get_enviados, especie_pokemon);
			solicitados++;
		}
	}

	if(conexion_con_broker == CONECTADO){
		log_info(LOGGER, "Comienzo envío de mensajes GET");
		list_iterate(objetivo_global, envio_un_get);
	} else {
		log_info(LOGGER, "No se envían mensajes GET, se asume que no hay pokemones en el mapa");
	}

	list_destroy_and_destroy_elements(get_enviados, free);
}

//----------------------------------------------------------------------------------------------------------
// Crea los 3 hilos sobre los cuales se van a manejas las siguientes suscripciones al broker:
//    1. APPEARED_POKEMON
//    2. LOCALIZED_POKEMON
//    3. CAUGHT_POKEMON
// Cada conexion se maneja desde un hilo en particular
//----------------------------------------------------------------------------------------------------------
void suscribirse_a_colas(){

	pthread_mutex_t mutex_suscripciones;
	pthread_mutex_init(&mutex_suscripciones, NULL);

	//------------------------------------------------------------------------------------------------------
	// Funcion que ejecuta dentro del hilo
	// Realiza la conexion del socket, la suscripcion y queda a la espera de mensajes
	//------------------------------------------------------------------------------------------------------
	void hilo_suscripcion(void* arg_hilo_suscripcion){

		op_code queue_suscripcion = *((op_code*)arg_hilo_suscripcion);

  		op_code tipo_de_mensaje;
  		int conexion;
  		char* suscripcion;

  		switch(queue_suscripcion){
			case SUBSCRIBE_APPEARED_POKEMON:
				conexion = socket_appeared;
				tipo_de_mensaje = APPEARED_POKEMON;
				suscripcion = "APPEARED_POKEMON";
				break;
			case SUBSCRIBE_LOCALIZED_POKEMON:
				conexion = socket_localized;
				tipo_de_mensaje = LOCALIZED_POKEMON;
				suscripcion = "LOCALIZED_POKEMON";
				break;
			case SUBSCRIBE_CAUGHT_POKEMON:
				conexion = socket_caught;
				tipo_de_mensaje = CAUGHT_POKEMON;
				suscripcion = "CAUGHT_POKEMON";
				break;
			default:
				break;
		}

  		pthread_mutex_lock(&mutex_suscripciones);
  		conexion = conectarse_a_broker();

  		if(conexion < 0){
  			log_info(LOGGER, "Inicio de proceso de reintento de comunicación con el Broker");
  			reintentar_conexion_con_broker(conexion);
  		}

  		enviar_mensaje_de_suscripcion(conexion, queue_suscripcion, ID_MANUAL_DEL_PROCESO_TEAM, NULL);
  		log_info(LOGGER, "Suscripcion a cola de mensajes %s realizada", suscripcion);
  		pthread_mutex_unlock(&mutex_suscripciones);

  		while(conexion_con_broker == CONECTADO){
  			recibir_con_semaforo(conexion, mutex_suscripciones, tipo_de_mensaje);
  		}

	}

  	pthread_t thread_caught;
  	op_code codigo_cola_1 = SUBSCRIBE_CAUGHT_POKEMON;
	pthread_create(&thread_caught,NULL,(void*)hilo_suscripcion, (void*)&codigo_cola_1);
	pthread_detach(thread_caught);

	pthread_t thread_appeared;
  	op_code codigo_cola_2 = SUBSCRIBE_APPEARED_POKEMON;
	pthread_create(&thread_appeared,NULL,(void*)hilo_suscripcion, (void*)&codigo_cola_2);
	pthread_detach(thread_appeared);

	pthread_t thread_localized;
  	op_code codigo_cola_3 = SUBSCRIBE_LOCALIZED_POKEMON;
	pthread_create(&thread_localized, NULL ,(void*)hilo_suscripcion, (void*)&codigo_cola_3);
	pthread_detach(thread_localized);

}


//----------------------------------------------------------------------------------------------------------
// Abre conexion con el broker.
// Setea flag global para saber el estado de la conexion actual
// Se establece por parametro si debe iniciar el proceso de reintento  de conexion
//----------------------------------------------------------------------------------------------------------
int conectarse_a_broker()
{
	char* ip = NULL;
	char* puerto = NULL;

	int retorno_ip_puerto = leer_ip_y_puerto(&ip, &puerto, CONFIG, "BROKER");

	if(retorno_ip_puerto == -1)
		imprimir_error_y_terminar_programa_perzonalizado("No se encontro IP_BROKER o PUERTO_BROKER en el archivo de configuracion", NULL, NULL);

	int socket = iniciar_conexion_como_cliente(ip, puerto);

	if(socket > 0) {
		conexion_con_broker = CONECTADO;
		log_info(LOGGER, "Se realizo una conexion con BROKER");
	} else {
		conexion_con_broker = DESCONECTADO;
	}

	return socket;
}

//----------------------------------------------------------------------------------------------------------
// Proceso de reintento de comunicacion con Broker
// Obtiene la cantidad de segundos para volver a intentar desde el config
//----------------------------------------------------------------------------------------------------------
void reintentar_conexion_con_broker(int conexion){
	int tiempo_reconexion = -1;

	tiempo_reconexion = asignar_int_property(CONFIG, "TIEMPO_RECONEXION");

	if(tiempo_reconexion == -1){
		log_error(LOGGER, "No existe la propiedad TIEMPO_RECONEXION");
		exit(-1);
	}

	char* ip = NULL;
	char* puerto = NULL;

	int retorno_ip_puerto = leer_ip_y_puerto(&ip, &puerto, CONFIG, "BROKER");

	if(retorno_ip_puerto == -1)
		imprimir_error_y_terminar_programa_perzonalizado("No se encontro IP_BROKER o PUERTO_BROKER en el archivo de configuracion", NULL, NULL);

	conexion = iniciar_conexion_como_cliente(ip, puerto);

	while(conexion < 0){
		sleep(tiempo_reconexion);
		log_info(LOGGER, "Reintentando conexion con proceso BROKER");
		conexion = iniciar_conexion_como_cliente(ip, puerto);
	}

	log_info(LOGGER, "Se logro conexion con proceso BROKER");
}

//----------------------------------------------------------------------------------------------------------
// Recibe un mensaje en alguna de las suscripciones y lo atiende
//----------------------------------------------------------------------------------------------------------
void recibir_con_semaforo(int socket_cliente, pthread_mutex_t mutex, op_code tipo_mensaje){

	int id_correlativo = 0;		//EL TEAM SI USA EL ID CORRELATIVO. ARREGLAR TODO
	int id_mensaje = 0;

	op_code codigo_operacion_recibido = 0;

	pthread_mutex_lock(&mutex);
	void* response = recibir_mensaje_por_socket(&codigo_operacion_recibido, socket_cliente, &id_correlativo, &id_mensaje, NULL, NULL);
	//deberia pasarle una funcion pa finalizar y en mutex. TODO
	pthread_mutex_unlock(&mutex);

	if(response == NULL){
		pthread_exit(NULL);
	}else{
		switch(tipo_mensaje){
			case APPEARED_POKEMON:
				log_info(LOGGER, "Se recibio un mensaje del tipo %s", tipo_mensaje);
				t_appeared_pokemon* appeared_pokemon = (t_appeared_pokemon*) response;
				atender_solicitud_appeared(appeared_pokemon->nombre, appeared_pokemon->coordenadas);
				break;
			case LOCALIZED_POKEMON:
				// TODO
				break;
			case CAUGHT_POKEMON:
				// TODO
				break;
			default:
				break;
		}
	}
}

//----------------------------------------------------------------------------------------------------------
// Atiende la solicitud del broker
//   1. Evalua si es un pokemon requerido
//   2. Si lo es
//      2.1 Busca entrenador cercano (entrenadores en NEW o bloqueados sin hacer nada)
//      2.2 TODO Si no hay ninguno, lo deberia dejar encolado
//      2.3 Elimina la lista parcial creada
//----------------------------------------------------------------------------------------------------------
void atender_solicitud_appeared(char* pokemon, t_coordenadas coordenadas){
	bool es_pokemon_necesario(void* elemento){
		return strcmp(pokemon, elemento);
	}

	if(list_any_satisfy(objetivo_pokemones_pendientes, es_pokemon_necesario)){
		t_list* lista_disponibles;
		lista_disponibles = list_create();

		list_add_all(lista_disponibles, entrenadores_new);
		list_add_all(lista_disponibles, entrenadores_blocked_sin_espera);

		t_entrenador_tcb* tcb_entrenador = obtener_entrenador_mas_cercano(lista_disponibles, coordenadas);

		list_destroy(lista_disponibles);

		if(tcb_entrenador != NULL){
			bool es_el_tcb_buscado(t_entrenador_tcb* elemento){
				return elemento->id_hilo_entrenador == tcb_entrenador->id_hilo_entrenador;
			}

			tcb_entrenador->nombre_pokemon = pokemon;
			tcb_entrenador->coordenadas_del_pokemon = coordenadas;

			pthread_mutex_lock(&mutex_entrenadores_ready);
			list_add(entrenadores_ready, tcb_entrenador);
			pthread_mutex_unlock(&mutex_entrenadores_ready);

			list_remove_by_condition(entrenadores_new, (void*)es_el_tcb_buscado);
			list_remove_by_condition(entrenadores_blocked_sin_espera, (void*)es_el_tcb_buscado);
		}

	}
}

//----------------------------------------------------------------------------------------------------------
// Busca el entrenador mas cercano al pokemon que aparecio
// Si encuentra alguno, le asigna la cantidad de rafagas a procesar
//----------------------------------------------------------------------------------------------------------
t_entrenador_tcb* obtener_entrenador_mas_cercano(t_list* entrenadores, t_coordenadas coordenadas_pokemon){

	int distancia = INT_MAX;

	t_entrenador_tcb* tcb_entrenador = NULL;
	t_link_element* elemento = entrenadores->head;

	while(elemento != NULL) {
		t_entrenador_tcb* tcb_actual = (t_entrenador_tcb*) elemento->data;
		int distancia_actual = distanciaEntreCoordenadas(tcb_actual->entrenador->coordenadas, coordenadas_pokemon);
		if(distancia_actual < distancia){
			tcb_entrenador = tcb_actual;
			distancia = distancia_actual;
		}
		elemento = elemento->next;
	}

	if(tcb_entrenador != NULL){
		tcb_entrenador->rafagas = distancia + RAFAGAS_CATCH;
	}

	return tcb_entrenador;
}

//----------------------------------------------------------------------------------------------------------
// Calcula la distancia entre dos coordenadas
// Se calcula el modulo de las distancias parciales
//----------------------------------------------------------------------------------------------------------
int distanciaEntreCoordenadas(t_coordenadas coordenada_A, t_coordenadas coordenada_B){
	return (fabs(coordenada_B.posx - coordenada_A.posx)) + (fabs(coordenada_B.posy - coordenada_A.posy));
}


//----------------------------------------------------------------------------------------------------------
// Funcion para liberar un t_entrenador_tcb
// Elimina las listas, el nombre del pokemon, el entrenador y el tcb en si mismo
//----------------------------------------------------------------------------------------------------------
void liberar_tcb(t_entrenador_tcb* tcb_entrenador){
	free(tcb_entrenador->nombre_pokemon);
	list_destroy_and_destroy_elements(tcb_entrenador->entrenador->objetivo_entrenador, free);
	list_destroy_and_destroy_elements(tcb_entrenador->entrenador->pokemones_entrenador, free);
	free(tcb_entrenador->entrenador);
	free(tcb_entrenador);
}

//----------------------------------------------------------------------------------------------------------
// Escucho puerto e ip para recibir conexion del GAME-BOY
// Cuando se conecta, actualizo el estado en la variable global
//----------------------------------------------------------------------------------------------------------
void escuchar_conexion()
{
	char* ip = NULL;
	char* puerto = NULL;

	leer_ip_y_puerto(&ip, &puerto, CONFIG, "GAMEBOY");	//DEBERIA RODEAR DE UN MUTEX A ESTA LLAMADA. TODO

    socket_servidor_gameboy = crear_socket_para_escuchar(ip, puerto);
    log_info(LOGGER, "Comienzo a escuchar IP:%s y PUERTO:%s para recibir conexion de GAME-BOY", ip, puerto);

    socket_cliente_gameboy = aceptar_una_conexion(socket_servidor_gameboy);
    log_info(LOGGER, "Se recibio una conexion desde GAME-BOY, socket_cliente_gameboy: %d", socket_cliente_gameboy);
    conexion_con_gameboy = CONECTADO;
}
