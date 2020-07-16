#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<readline/readline.h>
#include<netdb.h>

#include "estructuras.h"

void* serializar_paquete(t_paquete* paquete, int *bytes);

void serializar_identificacion(t_paquete** paquete, void* mensaje);
void serializar_mensaje(t_paquete** paquete, void* mensaje);
void serializar_new_pokemon(t_paquete** paquete, void* mensaje);
void serializar_appeared_pokemon(t_paquete** paquete, void* mensaje);
void serializar_catch_pokemon(t_paquete** paquete, void* mensaje);
void serializar_caught_pokemon(t_paquete** paquete, void* mensaje);
void serializar_get_pokemon(t_paquete** paquete, void* mensaje);
void serializar_localized_pokemon(t_paquete** paquete, void* mensaje);

int deserializar_identificacion(void* stream);
char* deserializar_mensaje(void* stream, int size);
t_new_pokemon* deserializar_new_pokemon(void *stream);
t_appeared_pokemon* deserializar_appeared_pokemon(void *stream);
t_catch_pokemon* deserializar_catch_pokemon(void *stream);
t_caught_pokemon* deserializar_caught_pokemon(void *stream);
t_get_pokemon* deserializar_get_pokemon(void *stream);
t_localized_pokemon* deserializar_localized_pokemon(void *stream);

void eliminar_paquete(t_paquete* paquete); //(TODO) REVISAR, nunca se define esta funcion

#endif /* SERIALIZACION_H_ */
