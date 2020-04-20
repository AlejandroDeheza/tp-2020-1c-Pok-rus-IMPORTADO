#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<readline/readline.h>
#include<netdb.h>
#include "conexion.h"


typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


void* serializar_paquete(t_paquete*, int*);
void eliminar_paquete(t_paquete* paquete);
