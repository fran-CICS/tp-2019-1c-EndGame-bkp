/*
 * server.c
 *
 *  Created on: 13 abr. 2019
 *      Author: utnso
 */
#include "server.h"


struct fds{
	t_list *lista;
}fd_conocidos;


//funcion para crear el hilo de server lissandra
int lanzarServer(){//@@crear una funcion que lance hilos de estos especiales seria mas lindo
	log_info(LOGGERFS,"Iniciando hilo de server lissandra");
	int resultadoDeCrearHilo = pthread_create( &threadServer, NULL, crearServerLissandra, (void*)NULL);
	if(resultadoDeCrearHilo){
		log_error(LOGGERFS,"Error al crear hilo de server lissandra, return code: %d",resultadoDeCrearHilo);
		exit(EXIT_FAILURE);
	}else{
		log_info(LOGGERFS,"Hilo de server lissandra creado exitosamente");
		return EXIT_SUCCESS;
	}

}

int list_mayor_int(t_list *lista){
	int size=list_size(lista),mayor=-1;
	for (int i=0;i<size;i++){
		int elemento = list_get(lista,i);
		if(elemento>mayor)mayor=elemento;
	}
	return mayor;
}


void fdDestroyer(void * elemento){
	free((int)elemento);
}

//log_info(LOGGERFS,"Iniciando server de lissandra");

//funcion que maneja la creacion y uso del server
void* crearServerLissandra(){
	//de momento uso este LOG que es del filesystem

	fd_conocidos->lista = list_create();

	int escuchador = escucharEn(configuracionDelFS.puertoEscucha);//creo puerto de escucha

	fd_set maestro;//creo filedescriptor principal
	FD_ZERO(&maestro);
	FD_SET(escuchador,&maestro);

	list_add(fd_conocidos->lista,escuchador);//agrego el fd escuchador, que es el que va a escuchar nuevas solicitudes de conexion

	while(1){//loop del select

		fd_set copia_maestro = maestro;//creo copia para que select no destruya los datos del maestro
		select(list_mayor_int(fd_conocidos->lista),&copia_maestro,NULL,NULL,NULL);

		//@ultimo param es timeout

		if(FD_ISSET(escuchador,&copia_maestro)){//aceptar nueva conexion

			int cliente_nuevo = aceptarConexion(escuchador);//@checkear si hubo error
			//@handshake
			list_add(fd_conocidos->lista,cliente_nuevo);
			FD_SET(cliente_nuevo,maestro);//agrego el nuevo cliente a los fd que conoce el maestro
		}
		else{//recibir mensaje de un cliente

			for(int i =1; i < list_size(fd_conocidos->lista);i++){//arranca de 1 xq en 0 siempre esta el escuchador

				int cliente = list_get(fd_conocidos->lista,i);

				if(FD_ISSET(cliente,&copia_maestro)){
					char msg[50]="";
					int bytes_recibidos = recv(cliente,msg,sizeof(msg),NULL);

					if(bytes_recibidos<=0){//remover este cliente
						cerrarConexion(cliente);
						FD_CLR(cliente,&maestro);

						bool _fdID(void * elem){//inner function para remover el fd que cierro
							return (int)elem == cliente;
						}

						list_remove_and_destroy_by_condition(fd_conocidos->lista,_fdID,fdDestroyer);
					}
					else{
						char rta[10]="Hola!\n";
						send(cliente,rta,sizeof(rta),NULL);
						//procesar msg
					}

				}

			}
		}



	}

	//cerrarConexion(servidor); en algun momento
	//pthread_exit(0);
}//@Agregar todo al log
