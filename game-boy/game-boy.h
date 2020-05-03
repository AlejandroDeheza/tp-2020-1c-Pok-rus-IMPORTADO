#ifndef GAME-BOY_H_
#define GAME-BOY_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>

#define TAMANIO_MAXIMO_ARGUMENTO 20+1

void verificarEntrada(int argc);
int configuracionInicial(char* ip, char* puerto, t_log** logger, t_config** config, char *argv[]);
void destinoBroker(char** ip, char** puerto, t_config* config);
void destinoTeam(char** ip, char** puerto, t_config* config);
void destinoGamecard(char** ip, char** puerto, t_config* config);
void despacharMensaje(int conexion, int argc, char *argv[]);

void enviarAlgoParaProbar(int conexion);	//PARA PROBAR A MANO
void enviarNew(int conexion, char *nombre, char *cuartoArg, char *quintoArg, char *sextoArg);
void enviarAppeared(int conexion, char *nombre, char *cuartoArg, char *quintoArg);
void enviarCatch(int conexion, char *nombre, char *cuartoArg, char *quintoArg);
void enviarCaught(int conexion, char *tercerArg);
void enviarGet(int conexion, char *nombre);
void terminar_programa(int, t_log*, t_config*);

#endif
