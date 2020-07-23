#include "broker.h"

t_queue queue_new_pokemon;
t_list suscribers_new_pokemon;
t_queue queue_appeared_pokemon;
t_list suscribers_appeared_pokemon;
t_queue queue_catch_pokemon;
t_list suscribers_catch_pokemon;
t_queue queue_caught_pokemon;
t_list suscribers_caught_pokemon;
t_queue queue_get_pokemon;
t_list suscribers_get_pokemon;
t_queue queue_localized_pokemon;
t_list suscribers_localized_pokemon;

pthread_mutex_t mutex_para_desuscribir;

int socket_a_desuscribir = 0;

int main(void)
{
	pthread_mutex_init(&mutex_para_desuscribir, NULL);

	t_config* config;
	list_clean(&suscribers_new_pokemon);
	list_clean(&suscribers_appeared_pokemon);
	list_clean(&suscribers_catch_pokemon);
	list_clean(&suscribers_caught_pokemon);
	list_clean(&suscribers_get_pokemon);
	list_clean(&suscribers_localized_pokemon);

	config = leer_config("../broker.config");

	char* ip = asignar_string_property(config, "IP_BROKER");
	char* puerto = asignar_string_property(config, "PUERTO_BROKER");
	int socket_servidor = crear_socket_para_escuchar(ip, puerto);

    while(1)
    {
    	pthread_t thread;

    	//pthread_mutex_lock(&mutex_conexion);
	   	argumentos_leer_mensajes* arg = malloc(sizeof(argumentos_leer_mensajes));
    	arg->socket = aceptar_una_conexion(socket_servidor);

    	pthread_create(&thread,NULL,(void*)leer_mensajes, (void*)arg);
    	pthread_detach(thread);
    	//pthread_mutex_unlock(&mutex_conexion);
    }

    list_destroy(&suscribers_new_pokemon);
    list_destroy(&suscribers_appeared_pokemon);
    list_destroy(&suscribers_catch_pokemon);
    list_destroy(&suscribers_caught_pokemon);
    list_destroy(&suscribers_get_pokemon);
    list_destroy(&suscribers_localized_pokemon);

    terminar_programa(socket_servidor, NULL, config);

	return EXIT_SUCCESS;
}

void leer_mensajes(void* argumentos)
{
	argumentos_leer_mensajes* arg = argumentos;
	int socket = arg->socket;

	while(1){
   	    int cod_op;

   	    int valor_retorno = recv(socket, &cod_op, sizeof(int), MSG_WAITALL);

   	    if(valor_retorno == 0)break; //si se finaliza el envio de mensajes, se sale de este while(1)

   	    if(valor_retorno < 0)
   	    {
   	    	cod_op = valor_retorno;
   	    }

		thread_process_request(cod_op, socket);
	}
}

void thread_process_request(int cod_op, int cliente_fd)
{
	switch (cod_op) {
		case SUBSCRIBE_NEW_POKEMON:
			suscribir(cliente_fd, &suscribers_new_pokemon);
			break;
		case SUBSCRIBE_APPEARED_POKEMON:
			suscribir(cliente_fd, &suscribers_appeared_pokemon);
			break;
		case SUBSCRIBE_CATCH_POKEMON:
			suscribir(cliente_fd, &suscribers_catch_pokemon);
			break;
		case SUBSCRIBE_CAUGHT_POKEMON:
			suscribir(cliente_fd, &suscribers_caught_pokemon);
			break;
		case SUBSCRIBE_GET_POKEMON:
			suscribir(cliente_fd, &suscribers_get_pokemon);
			break;
		case SUBSCRIBE_LOCALIZED_POKEMON:
			suscribir(cliente_fd, &suscribers_localized_pokemon);
			break;
		case UNSUBSCRIBE_NEW_POKEMON:
			desuscribir(cliente_fd, &suscribers_new_pokemon);
			break;
		case UNSUBSCRIBE_APPEARED_POKEMON:
			desuscribir(cliente_fd, &suscribers_appeared_pokemon);
			break;
		case UNSUBSCRIBE_CATCH_POKEMON:
			desuscribir(cliente_fd, &suscribers_catch_pokemon);
			break;
		case UNSUBSCRIBE_CAUGHT_POKEMON:
			desuscribir(cliente_fd, &suscribers_caught_pokemon);
			break;
		case UNSUBSCRIBE_GET_POKEMON:
			desuscribir(cliente_fd, &suscribers_get_pokemon);
			break;
		case UNSUBSCRIBE_LOCALIZED_POKEMON:
			desuscribir(cliente_fd, &suscribers_localized_pokemon);
			break;
		case NEW_POKEMON:
			dar_aviso(cliente_fd,&suscribers_new_pokemon, NEW_POKEMON);
			esperar_ack(&suscribers_new_pokemon);	//DEBERIAS USAR EL SOCKET COMO IDENTIFICARDOR DEL PROCESO, ALMENOS SIRVE PARA ESPERAR EL ACK.. TODO
			break;
		case APPEARED_POKEMON:
			dar_aviso(cliente_fd,&suscribers_appeared_pokemon, APPEARED_POKEMON);
			esperar_ack(&suscribers_appeared_pokemon);
			break;
		case CATCH_POKEMON:
			dar_aviso(cliente_fd,&suscribers_catch_pokemon, CATCH_POKEMON);
			esperar_ack(&suscribers_catch_pokemon);
			break;
		case CAUGHT_POKEMON:
			dar_aviso(cliente_fd,&suscribers_caught_pokemon, CAUGHT_POKEMON);
			esperar_ack(&suscribers_caught_pokemon);
			break;
		case GET_POKEMON:
			dar_aviso(cliente_fd,&suscribers_get_pokemon, GET_POKEMON);
			esperar_ack(&suscribers_get_pokemon);
			break;
		case LOCALIZED_POKEMON:
			dar_aviso(cliente_fd,&suscribers_localized_pokemon, LOCALIZED_POKEMON);
			esperar_ack(&suscribers_localized_pokemon);
			break;
		case 0:
			//este caso se deberia tratar. si es 0, es porque recv() en serve_client() retorna 0
			//eso significa que hubo una desconexion o que simplemente termino la conexion...
			//si hubo una desconexion, se debe intentar reconectar segun el proceso
			//TODO
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}
}


void suscribir(int cliente_fd, t_list *lista){
	printf("Suscribiendo\n");
	int* socket = malloc(sizeof(int));
	*socket = cliente_fd;
	list_add(lista, socket);	//EL SOCKET SOLO SE USA PARA ATENDER LOS MENSAJES EN SISTUACIONES NORMALES
	//SI BROKER U OTRO PROCESO SE CAE, HAY QUE ACTUALIZAR SOCKET CUANDO TODO ESTE ESTABLE

	int id_manual_del_proceso = 0;	// ESTO TAMBIEN SE DEBERIA GUARDAR EN LA LISTA TODO
	//SI ES == 0, ENTONCES TENDRIA UN COMPORTAMIENTO POR DEFAULT --> NO LE ENVIA NADA SI EL PROCESO SE CAE Y SE RECONECTA, POR EJ TODO
	recv(cliente_fd, &id_manual_del_proceso, sizeof(int), 0);


	//HACE FALTA ENVIAR ACK DEL MENSAJE DE SUSCRIPCION?? TODO
}

void desuscribir(int cliente_fd, t_list *lista){
	printf("Desuscribiendo\n");
	pthread_mutex_lock(&mutex_para_desuscribir);
	socket_a_desuscribir = cliente_fd;
	int* puntero = list_remove_by_condition(lista, es_igual_a_socket_a_desuscribir);
	free(puntero);
	pthread_mutex_unlock(&mutex_para_desuscribir);
}

bool es_igual_a_socket_a_desuscribir(void* elemento_de_lista)
{
	int un_socket_de_lista = *((int*)elemento_de_lista);
	return socket_a_desuscribir == un_socket_de_lista;
}

void dar_aviso(int cliente_fd, t_list *listaDeSuscriptores, int op_code){
	//Aca hay que guardar en cache, y dar aviso por otro hilo
	printf("Dando aviso\n");

	int id_correlativo = 0;
	recv(cliente_fd, &id_correlativo, sizeof(int), 0);

	int id_mensaje = 0;	//EL BROKER DEBERIA GENERAR UN ID Y GUARDARLO EN ALGUNA PARTE. TAMBIEN DEBERIA MANDAR ESE ID AL EMISOR DEL MENSAJE TODO
	recv(cliente_fd, &id_mensaje, sizeof(int), 0);

	int size_buffer = 0;
	void* mensaje = recibir_buffer(cliente_fd, &size_buffer);

    printf("op code %d\n", op_code);
	void avisarle(int client){
		enviar_mensaje_a_suscriptores(mensaje, size_buffer, client, op_code, id_mensaje, id_correlativo);	//revisar valor retorno TODO
	}

	list_iterate(listaDeSuscriptores, (void *)avisarle);
}

void esperar_ack(t_list *listaDeSuscriptores)
{
	int cantidadDeAckRecibidos = 0;
	t_list *duplicada =  list_duplicate(listaDeSuscriptores);

	while(cantidadDeAckRecibidos < list_size(listaDeSuscriptores)){

	bool ack(void* arg){
		int id_mensaje_del_ack;
		int cliente = *((int*)arg);
		if(recv(cliente, &id_mensaje_del_ack, sizeof(int), MSG_WAITALL) > -1) {
			printf("Codigo de mensaje del ack %d", id_mensaje_del_ack);	// AL RECIBIR ACK DEBERIA HACER OTRA COSA... TODO
			cantidadDeAckRecibidos =+ 1;
			return 0;
		} else {
			return 1;
		}
	}

		list_remove_by_condition(duplicada, ack);
	}
	pthread_exit(NULL);
}
