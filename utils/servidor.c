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


/**************vv funciones anteriores vv*****************/
void* recibir_buffer(int socket_cliente, int* size)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void devolver_mensaje(void* payload, int size, int socket_cliente, op_code operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = operacion;
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

