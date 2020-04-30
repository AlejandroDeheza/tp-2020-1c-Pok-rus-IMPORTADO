#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<readline/readline.h>
#include<netdb.h>

#include "cliente.h"


void* serializar_paquete(t_paquete*, int*);
void eliminar_paquete(t_paquete* paquete);
