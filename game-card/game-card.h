#ifndef GAME_CARD_H_
#define GAME_CARD_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/bitarray.h>
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

t_config* CONFIG = NULL;
t_log* LOGGER = NULL;

t_bitarray* BITMAP = NULL;
t_config* METADATA_METADATA_BIN = NULL;

int ID_MANUAL_DEL_PROCESO = 0;

typedef struct
{
	int entero;
	void* stream;
} argumentos_de_hilo;

void ejecutar_antes_de_terminar(int numero_senial);

void verificar_e_interpretar_entrada(int argc, char *argv[]);

void iniciar_file_system();

void generar_estructura_file_system_si_hace_falta(char* punto_montaje_file_system);
t_config* config_para_file_system(char* punto_montaje_file_system, char* path_archivo);

void crear_carpeta_si_no_existe(char* path_carpeta_con_nombre);
void escribir_y_cerrar_archivo_si_no_existe(char* path_archivo_con_nombre, char* datos_a_grabar, int tamanio_datos_a_grabar);
void sobrescribir_y_cerrar_archivo(char* path_archivo_con_nombre, char* datos_a_grabar, int tamanio_datos_a_grabar);
void generar_bloques_bin_que_hagan_falta(char* punto_montaje_file_system, int cantidad_bloques_del_fs, t_bitarray* bitarray);

t_bitarray* mapear_bitmap_en_memoria(char* archivo, size_t size_memoria_a_mapear);


pthread_t iniciar_hilo_de_mensajes(char* cola_a_suscribirse, op_code codigo_suscripcion);
void* conectar_recibir_y_enviar_mensajes(void* argumentos);
int conectar_a_broker_y_reintentar_si_hace_falta(char* mensaje_de_logueo_al_reintentar_conexion);

void iniciar_hilo_para_tratar_y_responder_mensaje(int id_mensaje_recibido, void* mensaje, op_code codigo_operacion);
void* atender_new_pokemon(void* argumentos);
void* atender_catch_pokemon(void* argumentos);
void* atender_get_pokemon(void* argumentos);

void conectar_enviar_verificar_caught(int id_mensaje_recibido, int resultado_caught);
void pedir_archivo(char* nombre_pokemon);
void retener_un_rato_y_liberar_archivo(char* nombre_pokemon);
void verificar_estado_del_envio_y_cerrar_conexion(char* tipo_mensaje, int estado, int conexion);

#endif
