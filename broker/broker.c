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

int main(void)
{
	char* ip;
	char* puerto;

	t_config* config;

	config = leer_config("../broker.config");

	asignar_string_property(config, "IP_BROKER", &ip);
	asignar_string_property(config, "PUERTO_BROKER", &puerto);

	int socket_servidor = crear_socket_para_escuchar(ip, puerto);

    while(1){
    	pthread_t thread;

    	int socket_cliente = aceptar_una_conexion(socket_servidor);

    	pthread_create(&thread,NULL,(void*)tomar_parte_del_mensaje,&socket_cliente);
    	pthread_detach(thread);

    }


	return EXIT_SUCCESS;
}

void tomar_parte_del_mensaje(int* socket)
{	//esta funcion es equivalente a serve_client()
	//pero esta retorna el codigo de operacion del mensaje que se esta recibiendo
	// Las otras dos hacian exactamente lo mismo, pero con nombre de variables diferentes
	// Habria que ver de reutilizar codigo.
	int cod_op;
	if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	process_request(cod_op, *socket);
}

void process_request(int cod_op, int cliente_fd) {
	//int size;
	//void* msg;
		switch (cod_op) {
		/*case MENSAJE:
			msg = recibir_mensaje_servidor(cliente_fd, &size);
			devolver_mensaje(msg, size, cliente_fd);
			free(msg);
			break;*/
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
		case NEW_POKEMON:
			dar_aviso(cliente_fd,&suscribers_new_pokemon);
			break;
		case APPEARED_POKEMON:
			dar_aviso(cliente_fd,&suscribers_appeared_pokemon);
			break;
		case CATCH_POKEMON:
			dar_aviso(cliente_fd,&suscribers_catch_pokemon);
			break;
		case CAUGHT_POKEMON:
			dar_aviso(cliente_fd,&suscribers_caught_pokemon);
			break;
		case GET_POKEMON:
			dar_aviso(cliente_fd,&suscribers_get_pokemon);
			break;
		case LOCALIZED_POKEMON:
			dar_aviso(cliente_fd,&suscribers_localized_pokemon);
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
	list_add(lista, cliente_fd);
}

void avisarle_a_cada_suscriptor(int client, void* mensaje){

	printf("por mandar mensaje locura \n");
	send(client, mensaje, 33, 0);
}

//Avisarle a los que estan suscriptos que llego un mensaje de este estilo//
void dar_aviso(int cliente_fd, t_list *listaDeSuscriptores){

	int id_mensaje = 0;
	recv(cliente_fd, &id_mensaje, sizeof(int), 0);

	int id_correlativo = 0;
	recv(cliente_fd, &id_correlativo, sizeof(int), 0);

	void* mensaje = recibir_mensaje_desde_cliente(cliente_fd);

	t_appeared_pokemon* appeared_pokemon = deserializar_appeared_pokemon(mensaje);
	printf("nomres por docquier %s \n", (*appeared_pokemon).nombre);

	void avisarle(int client){
		return avisarle_a_cada_suscriptor(client, mensaje);
	}
   //Aca se me hace que es al pedo el deserializar, ya que no le interesa la data que tenga adentro, solo hace un pasamanos.
	list_iterate(listaDeSuscriptores, avisarle);
}







