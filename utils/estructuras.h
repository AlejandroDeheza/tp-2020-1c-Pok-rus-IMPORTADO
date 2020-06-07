#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/collections/list.h>

typedef enum
{
	IDENTIFICACION = 1,
	NEW_POKEMON = 2,
	NEW_POKEMON_RESPONSE=3,
	MENSAJE = 4,
	APPEARED_POKEMON,
	CATCH_POKEMON,
	CAUGHT_POKEMON,
	GET_POKEMON,
	LOCALIZED_POKEMON
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

typedef struct
{
	int id_entrenador;
	t_coordenadas coordenadas;
	t_list* pokemones_entrenador;
	t_list* objetivo_entrenador;

} t_entrenador;

typedef enum
{
	NEW = 0,
	READY = 1,
	EXEC = 2,
	BLOCK = 3,
	EXIT = 4
} process_status;

typedef struct
{
	unsigned long int id_hilo_entrenador;
	t_entrenador* entrenador;
	process_status status;

} t_entrenador_tcb;

#endif /* ESTRUCTURAS_H_ */

