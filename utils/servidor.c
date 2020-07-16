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
	struct sockaddr_in dir_cliente;

	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente,(void*) &tam_direccion);

	return socket_cliente;
}

//Es bastante parecido a enviar_mensaje_como_cliente() de cliente.c, pero este no serializa, ya que el broker al enterarse de que recibio un mensaje,
// lo redistribuye y no pierde tiempo deserializando y serializando
void enviar_mensaje_a_suscriptores(void* mensaje, int size_mensaje, int socket_cliente, op_code codigo_operacion, int id_mensaje, int id_correlativo)
{
	int estado = 0;

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_operacion;
	paquete->id_correlativo = id_correlativo;	//este id lo puede settear el proceso que manda el mensaje
												//tambien lo puede dejar en 0 si no conoce el id
	paquete->id_mensaje = id_mensaje;	//EL ID_MENSAJE SIEMPRE LO SETEA EL BROKER
								//a menos que el gameboy envie un mensaje a gamecard
								//en ese caso lo seteamos nosotros desde consola
	paquete->buffer = malloc(sizeof(t_buffer));

	paquete->buffer->size = size_mensaje;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	printf("EnviarMensaje -> Mensaje Empaquetado: \"%s\".\n", (char*)paquete->buffer->stream);

	int bytes = 0;
	void* aEnviar = serializar_paquete(paquete, &bytes);
	printf("EnviarMensaje -> Paquete Serializado - Tamaño Total: %d Bytes.\n", bytes);

	estado = send(socket_cliente, aEnviar, bytes, MSG_NOSIGNAL); //soy Ale,
	//agrego el flag "MSG_NOSIGNAL" por lo que decian en este issue: https://github.com/sisoputnfrba/foro/issues/1707
	//lo marco como TODO para que esto resalte un poco y lo puedan mirar despues

	verificar_estado(estado);

	free(aEnviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
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

