#include<stdio.h>
#include<stdlib.h>
#include<commons/collections/list.h>

typedef enum
{
	IDENTIFICACION = 1,
	NEW_POKEMON = 2,
	NEW_POKEMON_RESPONSE=3,
	MENSAJE = 4
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
	t_coordenadas posicion;
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
	int size;
	void* nombre;
} t_get_pokemon;



typedef struct
{
	int size;
	void* nombre;
	t_coordenadas coordenadas;
} t_appaeared_pokemon;


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
