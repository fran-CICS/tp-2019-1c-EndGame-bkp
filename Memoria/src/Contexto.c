/*
 * Contexto.c
 *
 *  Created on: 8 abr. 2019
 *      Author: utnso
 *      Variables globales y contexto general
 */
#include "Contexto.h"

/* Variables globales*/
char* PUERTO_ESCUCHA;
char* IP_FILESYSTEM;
int PUERTO_FILESYSTEM;
int TAMANIO_MEMORIA;
int NUMERO_MEMORIA;
int TAMANIO_VALUE;
int SOCKET_LISS=-1;
int SERVER_MEMORIA;
char** IPS_SEEDS;
char** PUERTOS_SEEDS;
int RETARDO_ACCESO_MEMORIA;
int RETARDO_ACCESO_FILESYSTEM;
long TIEMPO_JOURNAL;
long TIEMPO_GOSSIPING;
char * MEMORIA_PRINCIPAL;
t_list * seeds=NULL;
t_list * mi_tabla_de_gossip=NULL;

/* Semáforos mutex de variables globales */
pthread_mutex_t M_RETARDO_ACCESO_MEMORIA;
pthread_mutex_t M_RETARDO_ACCESO_FILESYSTEM;
pthread_mutex_t M_WATCH_DESCRIPTOR;
pthread_mutex_t M_CONF_FD;
pthread_mutex_t M_PATH_ARCHIVO_CONFIGURACION;
pthread_mutex_t M_RUTA_ARCHIVO_CONF;
pthread_mutex_t M_JOURNALING;
pthread_mutex_t M_TABLA_GOSSIP;

t_conexion_cliente conexiones_cliente[MAX_CLIENTES];
t_log* g_logger;
t_config* g_config;
t_bitarray *bitmap_marcos=NULL;
t_list * tabla_de_segmentos=NULL;
