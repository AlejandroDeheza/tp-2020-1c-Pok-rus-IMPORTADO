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

void executar_antes_de_terminar(int numero_senial);

void iniciar_file_system();

void generar_estructura_file_system_si_hace_falta(char* punto_montaje_file_system);
t_config* config_para_file_system(char* punto_montaje_file_system, char* path_archivo);

void crear_carpeta_si_no_existe(char* path_carpeta_con_nombre);
void escribir_y_cerrar_archivo_si_no_existe(char* path_archivo_con_nombre, char* datos_a_grabar, int tamanio_datos_a_grabar);
void sobrescribir_y_cerrar_archivo(char* path_archivo_con_nombre, char* datos_a_grabar, int tamanio_datos_a_grabar);
void generar_bloques_bin_que_hagan_falta(char* punto_montaje_file_system, int cantidad_bloques_del_fs, t_bitarray* bitarray);

t_bitarray* mapear_bitmap_en_memoria(char* archivo, size_t size_memoria_a_mapear);

int suscribirse_a_broker(t_config* config, op_code nombre_cola);

#endif
