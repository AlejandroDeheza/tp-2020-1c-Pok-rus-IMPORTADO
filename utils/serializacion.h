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


typedef struct
{
	op_code codigo_operacion;
	t_new_pokemon* buffer;
} t_mensaje_new_pokemon;


void* serializar_paquete(t_paquete*, int*);
void* serializar_new_pokemon(t_mensaje_new_pokemon*, int*);
void eliminar_paquete(t_paquete* paquete);
