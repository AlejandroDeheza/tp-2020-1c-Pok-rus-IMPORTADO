#ifndef GAMECARD_H_
#define GAMECARD_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/bitarray.h>
#include<commons/process.h>
#include<readline/readline.h>
#include<semaphore.h>
#include<pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <cliente.h>
#include "../utils/config.h"
#include "servidor.h"

t_dictionary* DICCIONARIO_HILOS;
bool HAY_QUE_TERMINAR;
int SOCKET_SERVIDOR;

pthread_mutex_t* MUTEX_CONFIG;
pthread_mutex_t* MUTEX_LOGGER;
pthread_mutex_t* MUTEX_METADATA_METADATA_BIN;
pthread_mutex_t* MUTEX_BITMAP;
pthread_mutex_t* MUTEX_ID_PROCESOS_TP;
pthread_mutex_t* MUTEX_ID_MANUAL_DEL_PROCESO;

pthread_mutex_t* MUTEX_DICCIONARIO;
pthread_mutex_t* MUTEX_CONSULTA_POKEMON;

pthread_mutex_t* MUTEX_DICCIONARIO_HILOS;
pthread_mutex_t* MUTEX_HAY_QUE_TERMINAR;
pthread_mutex_t* MUTEX_SOCKET_SERVIDOR;

t_dictionary* DICCIONARIO_CON_MUTEX;


t_config* CONFIG = NULL;
t_log* LOGGER = NULL;

t_bitarray* BITMAP = NULL;
t_config* METADATA_METADATA_BIN = NULL;

int ID_MANUAL_DEL_PROCESO = 0;
char* ID_PROCESOS_TP;

typedef struct
{
	int entero;
	void* stream;
} argumentos_de_hilo;


void ejecutar_antes_de_terminar(int numero_senial);
void finalizar_gamecard();

void verificar_e_interpretar_entrada(int argc, char *argv[]);
void iniciar_variables_globales_gamecard();
void esperar_a_todos_los_hilos();
void establecer_comunicaciones_gamecard();


///**** FUNCIONES PARA ARCHIVOS FILE SYSTEM *****///
void iniciar_file_system();
void generar_estructura_file_system_si_hace_falta(char* punto_montaje_file_system);
t_bitarray* mapear_bitmap_en_memoria(char* archivo, size_t size_memoria_a_mapear);
void generar_bloques_bin_que_hagan_falta(char* punto_montaje_file_system, int cantidad_bloques_del_fs, t_bitarray* bitarray);
t_config* config_para_file_system(char* punto_montaje_file_system, char* path_archivo);

void crear_carpeta_si_no_existe(char* path_carpeta_con_nombre);
void escribir_y_cerrar_archivo_si_no_existe(char* path_archivo_con_nombre, char* datos_a_grabar, int tamanio_datos_a_grabar);
void sobrescribir_y_cerrar_archivo(char* path_archivo_con_nombre, char* datos_a_grabar, int tamanio_datos_a_grabar);


///**** PODRIAN IR EN UTILS *****///
void eliminador_de_mutex_en_diccionario(void* argumento);

pthread_t iniciar_hilo_para_recibir_conexiones();
void* recibir_conexiones();
pthread_t iniciar_hilo_para_comunicarse_con_broker(char* cola_a_suscribirse, op_code codigo_suscripcion);
void* conectar_recibir_y_enviar_mensajes(void* argumentos);

int conectar_a_broker_y_reintentar_si_hace_falta(char* mensaje_de_logueo_al_reintentar_conexion);
void verificar_estado_del_envio_y_cerrar_conexion(char* tipo_mensaje, int estado, int conexion);


///**** FUNCIONES PARA HILOS DE GAMECARD *****///
void iniciar_hilo_para_tratar_y_responder_mensaje(int id_mensaje_recibido, void* mensaje, op_code codigo_operacion);
void* atender_new_pokemon(void* argumentos);
void* atender_catch_pokemon(void* argumentos);
void* atender_get_pokemon(void* argumentos);
void retener_conectar_librerar_recursos_caught(t_catch_pokemon* mensaje, int id_mensaje_recibido, int resultado_caught, char* posicion_buscada_en_string,
		char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades);
void conectar_enviar_verificar_caught(int id_mensaje_recibido, int resultado_caught);


///**** FUNCIONES PARA ARCHIVOS POKEMON *****///
bool verificar_si_existe_archivo_pokemon(char* nombre_pokemon);
void crear_archivo_pokemon(char* nombre_pokemon);
void pedir_archivo_pokemon(char* nombre_pokemon);
bool intentar_abrir_archivo_pokemon(char* nombre_pokemon);
void retener_un_rato_y_liberar_archivo_pokemon(char* nombre_pokemon);
void liberar_archivo_pokemon(char* nombre_pokemon);

int buscar_posicion_en_archivo_pokemon(char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades, char* posicion_buscada_en_string);
t_list* obtener_todas_las_posiciones_de_archivo_pokemon(char* nombre_pokemon);
char** generar_array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades(char* nombre_pokemon);
int agregar_cantidad_en_archivo_pokemon(t_new_pokemon* mensaje, char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades, char* posicion_buscada_en_string);
void reducir_cantidad_en_archivo_pokemon(char* nombre_pokemon, int indice_de_busqueda, char** array_de_todo_el_archivo_pokemon_con_posiciones_y_cantidades, char* posicion_buscada_en_string);

#endif
