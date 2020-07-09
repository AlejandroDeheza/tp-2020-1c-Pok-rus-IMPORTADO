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

pthread_mutex_t mutex;

int main(void)
{
	char* ip;
	char* puerto;
	pthread_mutex_init(&mutex, NULL);

	t_config* config;
	list_clean(&suscribers_new_pokemon);
	list_clean(&suscribers_appeared_pokemon);
	list_clean(&suscribers_catch_pokemon);
	list_clean(&suscribers_caught_pokemon);
	list_clean(&suscribers_get_pokemon);
	list_clean(&suscribers_localized_pokemon);

	config = leer_config("../broker.config");

	asignar_string_property(config, "IP_BROKER", &ip);
	asignar_string_property(config, "PUERTO_BROKER", &puerto);

	int socket_servidor = crear_socket_para_escuchar(ip, puerto);

    while(1){
    	pthread_t thread;

    	//pthread_mutex_lock(&mutex_conexion);
    	int socket_cliente = aceptar_una_conexion(socket_servidor);

    	pthread_create(&thread,NULL,(void*)leer_mensaje,&socket_cliente);
    	pthread_detach(thread);
    	//pthread_mutex_unlock(&mutex_conexion);

    }

    list_destroy(&suscribers_new_pokemon);
    list_destroy(&suscribers_appeared_pokemon);
    list_destroy(&suscribers_catch_pokemon);
    list_destroy(&suscribers_caught_pokemon);
    list_destroy(&suscribers_get_pokemon);
    list_destroy(&suscribers_localized_pokemon);

	return EXIT_SUCCESS;
}

void leer_mensaje(int* socket)
{
	int cod_op;
	if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}

void process_request(int cod_op, int cliente_fd) {
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
			esperar_ack(&suscribers_new_pokemon);
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
	list_add(lista, &cliente_fd);
}

void desuscribir(int cliente_fd, t_list *lista){
	printf("Desuscribiendo\n");
	list_remove(lista, cliente_fd);
}

void dar_aviso(int cliente_fd, t_list *listaDeSuscriptores, int op_code){
	//Aca hay que guardar en cache, y dar aviso por otro hilo
	printf("Dando aviso\n");

	int id_correlativo = 0;
	recv(cliente_fd, &id_correlativo, sizeof(int), 0);

	int id_mensaje = 0;
	recv(cliente_fd, &id_mensaje, sizeof(int), 0);

	int size_buffer = 0;
	void* mensaje = recibir_buffer(cliente_fd, &size_buffer);

    printf("op code %d", op_code);
	void avisarle(int client){
		enviar_mensaje_a_suscriptores(mensaje, size_buffer, client, op_code, id_mensaje, id_correlativo);
	}

	list_iterate(listaDeSuscriptores, (void *)avisarle);
}

void esperar_ack(t_list *listaDeSuscriptores)
{
	int cantidadDeAckRecibidos = 0;
	t_list *duplicada =  list_duplicate(listaDeSuscriptores);

	while(cantidadDeAckRecibidos < list_size(listaDeSuscriptores)){
	bool ack(int cliente){
		int cod_op;
		if(recv(cliente, &cod_op, sizeof(int), MSG_WAITALL) > -1) {
			printf("Codigo de operacion del ack %d", cod_op);
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






