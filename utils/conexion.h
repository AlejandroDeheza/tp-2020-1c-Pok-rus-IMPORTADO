#ifndef TEAM_UTILS_H_
#define TEAM_UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/collections/list.h>
#include<readline/readline.h>

typedef enum
{
	TEAM = 1,
	NEW_POKEMON = 2,
	NEW_POKEMON_RESPONSE=3
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;


typedef struct
{
	int size;
	void* nombre;
	int posx;
	int posy;
	int cantidad;
} t_new_pokemon;

typedef struct
{
	int size;
	void* nombre;
	int cantidad_de_posiciones;
    t_list* coordenadas;
} t_localized_pokemon;

typedef struct
{
	int posx;
	int posy;
} t_coordenadas;

typedef struct
{
	int size;
	void* nombre;
} t_get_pokemon;



typedef struct
{
	int size;
	void* nombre;
	int posx;
	int posy;
} t_appaeared_pokemon;


typedef struct
{
	int size;
	void* nombre;
	int posx;
	int posy;
} t_catch_pokemon;


typedef struct
{
	int resultado;
} t_caught_pokemon;


int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
void new_pokemon(char* nombre, int posx, int posy, int cantidad, int socket_cliente);
char* recibir_mensaje(int socket_cliente);
void liberar_conexion(int socket_cliente);
t_log* iniciar_logger(char*, char*);


#endif /* TEAM_UTILS_H_ */
