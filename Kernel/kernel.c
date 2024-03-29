/*
 * kernel.c
 *
 *  Created on: 16 abr. 2019
 *      Author: utnso
 */

#include "kernel.h"

int main(int argc,char** argv) {
	if(inicializarVariablesGlobales()==EXIT_SUCCESS){
	inicializarLogKernel();
	inicializarSemaforos();
	inicializarListas();
	iniciar_config(argc,argv);
	if(levantarConfiguracionInicialDelKernel()==EXIT_SUCCESS){
	configurar_signals();
	if(conectarse_con_primera_memoria(configKernel.ipMemoria, configKernel.puertoMemoria)==EXIT_SUCCESS){
		iniciar_proceso_describe_all();
		iniciar_pedido_gossip();
		iniciar_metrics();
		if(lanzarConsola()==EXIT_SUCCESS){
			if(lanzarPlanificador()==EXIT_SUCCESS){
			if(lanzarPCP()==EXIT_SUCCESS){
			if(esperarAQueTermineLaConsola()==EXIT_FAILURE){
				terminar_programa(EXIT_FAILURE);
			}}}}}}}

	return EXIT_SUCCESS;
}
