#include "servidor.h"

int crear_socket_para_escuchar(char *ipServidor, char* puertoServidor){
	//esta funcion es equivalente a iniciar_servidor()
	//pero esta retorna el socket del servidor

	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ipServidor, puertoServidor, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next){

        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int aceptar_una_conexion(int socket_servidor){
	//esta funcion es equivalente a esperar_cliente()
	//pero esta retorna el socket del cliente
	//tampoco crea los hilos, así eso lo maneja cada proceso. Por ejemplo TEAM
	//así (el TEAM por ej) puede recibir el mensaje enviado y tratarlo desde el mismo proceso (TEAM)

	struct sockaddr_in dir_cliente;

	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente,(void*) &tam_direccion);

	return socket_cliente;
}


void* recibir_mensaje_desde_cliente(int socket_cliente){
	//esta funcion se puede llamar desde algun proceso (ej: TEAM)
	//para recibir el "stream" del t_buffer del t_paquete enviado

	void *stream;
	int size = 0;

	recv(socket_cliente, &size, sizeof(int), MSG_WAITALL);
	stream = malloc(size);
	recv(socket_cliente, stream, size, MSG_WAITALL);

	return stream;
}
	//despues de usar esta funcion, se puede usar un switch (como en process_request(), ver mas abajo)
	//para que segun el codigo de operacion, se llame a la funcion que deserializa el stream
	//(ej: deserializar_appeared_pokemon()) que esta en serializacion.c
	//para luego trabajar con el mensaje de la manera correspondiente segun el proceso




/**************vv funciones anteriores vv*****************/
void* recibir_mensaje_servidor(int socket_cliente, int* size)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void devolver_mensaje(void* payload, int size, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = 1;
	paquete->id_correlativo = 0;	//agregado para que no rompa
	paquete->id_mensaje = 0;		//agregado para que no rompa
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = size;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, payload, paquete->buffer->size);

	int bytes = 0;

	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

