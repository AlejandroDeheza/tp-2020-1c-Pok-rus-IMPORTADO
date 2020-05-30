#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/collections/list.h>

typedef enum
{
	IDENTIFICACION = 1,
	NEW_POKEMON,
	NEW_POKEMON_RESPONSE,
	APPEARED_POKEMON,
	CATCH_POKEMON,
	CAUGHT_POKEMON,
	GET_POKEMON,
	LOCALIZED_POKEMON,
	MENSAJE
}op_code;


typedef enum
{
	TEAM = 1,
	GAMEBOY = 2,
	GAMECARD = 3,
	BROKER = 4
} proyecto;

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

#endif /* ESTRUCTURAS_H_ */

