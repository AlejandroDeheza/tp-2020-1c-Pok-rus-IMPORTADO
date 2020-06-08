#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/collections/list.h>

typedef enum
{
	IDENTIFICACION = 1,
	NEW_POKEMON = 11,
	APPEARED_POKEMON = 12,
	CATCH_POKEMON = 13,
	CAUGHT_POKEMON = 14,
	GET_POKEMON = 15,
	LOCALIZED_POKEMON = 16,
	MENSAJE = 2,
	NEW_POKEMON_RESPONSE = 3,
	SUBSCRIBE_NEW_POKEMON = 21,
	SUBSCRIBE_APPEARED_POKEMON = 22,
	SUBSCRIBE_CATCH_POKEMON = 23,
	SUBSCRIBE_CAUGHT_POKEMON = 24,
	SUBSCRIBE_GET_POKEMON = 25,
	SUBSCRIBE_LOCALIZED_POKEMON = 26
}op_code;


typedef enum
{
	TEAM = 10,
	GAMEBOY = 11,
	GAMECARD = 12,
	BROKER = 13
} proyecto;

//mensjae de suscripcion -> TEAM(process_code) SUBSCRIBE_NEW_POKEMON(subscribe_code)    -> asociar el socket a la lista

// NEW_POKEMON PIKACHU ASDADADAD

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	int id_correlativo;	// puede que este seteado en 0 por defecto para indicar que no se usa
	int id_mensaje;		// puede que este seteado en 0 por defecto para indicar que no se usa
	t_buffer* buffer;
} t_paquete;

typedef struct
{
	int posx;
	int posy;
} t_coordenadas;

typedef struct
{
	int size;
	void* nombre;
	t_coordenadas coordenadas;
	int cantidad;
} t_new_pokemon;

typedef struct
{
	int size;
	void* nombre;
    t_list* coordenadas;
} t_localized_pokemon;


typedef struct
{
	int size;
	void* nombre;
} t_get_pokemon;



typedef struct
{
	int size;
	void* nombre;
	t_coordenadas coordenadas;
} t_appeared_pokemon;


typedef struct
{
	int size;
	void* nombre;
	t_coordenadas coordenadas;
} t_catch_pokemon;


typedef struct
{
	int resultado;
} t_caught_pokemon;

typedef struct
{
	int id_mensaje;
	op_code code;
	t_list* suscriptores;
	t_list* ack_suscriptores;
} t_cache;


#endif /* ESTRUCTURAS_H_ */

