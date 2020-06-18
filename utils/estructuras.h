#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/collections/list.h>

typedef enum
{
	NEW_POKEMON = 1,
	APPEARED_POKEMON = 2,
	CATCH_POKEMON = 3,
	CAUGHT_POKEMON = 4,
	GET_POKEMON = 5,
	LOCALIZED_POKEMON = 6,
	MENSAJE_A_COLA = 7,
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
	pthread_mutex_t mutex;
	t_coordenadas coordenadas_del_pokemon;
	char* nombre_pokemon;
	int rafagas;

} t_entrenador_tcb;



#endif /* ESTRUCTURAS_H_ */

