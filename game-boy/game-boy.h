#ifndef GAMEBOY_H_
#define GAMEBOY_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>

#define TAMANIO_MAXIMO_ARGUMENTO 20+1

void verificarEntrada(int argc);
int configuracionInicial(char* ip, char* puerto, t_log** logger, t_config** config, char *argv[]);
void seleccionarFuncion(char **funcion, char *primerArg);

void despacharMensaje(int conexion, int argc, char *argv[]);
void enviarAlgoParaProbar(int conexion);	//PARA PROBAR A MANO
void enviarNew(int conexion, int argc, char *argv[]);
void enviarAppeared(int conexion, int argc, char *argv[]);
void enviarCatch(int conexion, int argc, char *argv[]);
void enviarCaught(int conexion, int argc, char *argv[]);
void enviarGet(int conexion, int argc, char *argv[]);
void terminar_programa(int, t_log*, t_config*);

#endif
