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
int enviar_mensaje_a_suscriptores(void* mensaje, int size_mensaje, int socket_cliente, op_code codigo_operacion, int id_mensaje, int id_correlativo)
{
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

	int bytes = 0;
	void* aEnviar = serializar_paquete(paquete, &bytes);
	printf("EnviarMensaje -> Paquete Serializado - Tamaño Total: %d Bytes.\n", bytes);

	int estado = send(socket_cliente, aEnviar, bytes, MSG_NOSIGNAL);
	//agrego el flag "MSG_NOSIGNAL" por lo que decian en este issue: https://github.com/sisoputnfrba/foro/issues/1707

	verificar_estado(estado);

	free(aEnviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return estado;
}

bool es_un_proceso_esperado(int socket_cliente, char* id_procesos_tp)
{
	int size = 0;

	if(recv(socket_cliente, &size, sizeof(int), MSG_WAITALL) <= 0) return false;
	void* cadena_recibida = malloc(size);
	if(recv(socket_cliente, cadena_recibida, size, MSG_WAITALL) <= 0) return false;

	if(strcmp((char*) cadena_recibida, id_procesos_tp) == 0)
	{
		free(cadena_recibida);
		return true;
	}

	free(cadena_recibida);
	return false;
}

/**************vv funciones anteriores vv*****************/


