
/*
 * api.h
 *
 *  Created on: 5 abr. 2019
 *      Author: utnso
 */

#ifndef API_H_
#define API_H_

#include <pthread.h>
#include <readline/readline.h>
#include "lissandra.h"
#include "funcionesComunes.h"
#include "dump.h"

int lanzarConsola();
int lanzarCompactador();
void *funcionHiloCompactador(void *arg);
void *funcionHiloConsola(void *arg);
int esperarPorHilos();
char** parser_instruccion(char* linea);
int consolaSelect(char* nombreDeLaTabla,uint16_t key);
int consolaInsert(char* nombreDeLaTabla,uint16_t key,char* valor,double timestamp);
int consolaInsertSinTime(char* nombreDeLaTabla,uint16_t key,char* valor);
int consolaCreate(char* nombreDeLaTabla,char* tipoDeConsistencia,int numeroDeParticiones,int tiempoDeCompactacion);
int consolaDescribe();
int consolaDescribeDeTabla(char* nombreDeLaTabla);
int consolaDrop(char* nombreDeLaTabla);
int man();
int reloadConfig();
int imprimirConfiguracionDelSistema();
int imprimirMemtableEnPantalla();

#endif /* SRC_API_H_ */
