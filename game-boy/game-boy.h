#ifndef GAMEBOY_H_
#define GAMEBOY_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <commons/error.h>
#include <cliente.h>
#include "../utils/config.h"

void verificarEntrada(int argc, char *argv[]);

void gestionar_envio_de_mensaje(int* conexion, t_config* config, t_log* logger, int argc, char* argv[]);

void logearConexion(t_log* logger, char *primerArg);
void despacharMensaje(int conexion, int argc, char *argv[]);
void logearEnvio(t_log* logger, char *argv[]);

void enviarNew(int conexion, int argc, char *argv[]);
void enviarAppeared(int conexion, int argc, char *argv[]);
void enviarCatch(int conexion, int argc, char *argv[]);
void enviarCaught(int conexion, int argc, char *argv[]);
void enviarGet(int conexion, int argc, char *argv[]);
void enviarLocalized(int conexion, int argc, char *argv[]);

void terminar_programa(int, t_log*, t_config*);

#endif
