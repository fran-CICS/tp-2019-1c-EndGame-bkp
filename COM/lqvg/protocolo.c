#include "protocolo.h"

void prot_enviar_int(int i,int sock){
	enviar(sock,&i,sizeof(i));
}

int prot_recibir_int(int sock){
	int i;
	recibir(sock,&i,sizeof(i));
	return i;
}

void prot_enviar_error(enum MENSAJES error, int socket){//para enviar cuando la tabla no existe,etc
	t_paquete* paquete = crear_paquete(error);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

void prot_enviar_create(char* nombreDeLaTabla, char* tipoDeConsistencia,
		int numeroDeParticiones, int tiempoDeCompactacion,int socket){
	t_paquete* paquete = crear_paquete(CREATE);
	agregar_string_a_paquete(paquete, nombreDeLaTabla, strlen(nombreDeLaTabla)+1);
	agregar_string_a_paquete(paquete, tipoDeConsistencia, strlen(tipoDeConsistencia)+1);
	agregar_int_a_paquete(paquete, numeroDeParticiones);
	agregar_int_a_paquete(paquete, tiempoDeCompactacion);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

tp_create prot_recibir_create(int tamanio_paq,int socket){
	void * buffer = malloc(tamanio_paq);
	recibir(socket, buffer, tamanio_paq);
	int tamanio_nom_tabla;
	int tamanio_tipo_consistencia;
	int numeroDeParticiones;
	int tiempoDeCompactacion;
	int desplazamiento = 0;
	memcpy(&tamanio_nom_tabla, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	char* nom_tabla=malloc(tamanio_nom_tabla);
	memcpy(nom_tabla, buffer+desplazamiento, tamanio_nom_tabla);
	desplazamiento+=tamanio_nom_tabla;
	memcpy(&tamanio_tipo_consistencia, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	char* tipo_consistencia=malloc(tamanio_tipo_consistencia);
	memcpy(tipo_consistencia, buffer+desplazamiento, tamanio_tipo_consistencia);
	desplazamiento+=tamanio_tipo_consistencia;
	memcpy(&numeroDeParticiones, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(&tiempoDeCompactacion, buffer+desplazamiento, sizeof(int));
	tp_create param_create=malloc(sizeof(t_create));
	param_create->nom_tabla=nom_tabla;
	param_create->tipo_consistencia=tipo_consistencia;
	param_create->numero_particiones=numeroDeParticiones;
	param_create->tiempo_compactacion=tiempoDeCompactacion;
	free(buffer);
	return param_create;
}
void prot_enviar_respuesta_create(int socket){
	t_paquete* paquete = crear_paquete(REQUEST_SUCCESS);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

enum MENSAJES prot_recibir_respuesta_create(int socket){
	//posibles MENSAJES a recibir del fs: REQUEST_SUCCESS, TABLA_YA_EXISTIA, NO_HAY_MAS_BLOQUES_EN_EL_FS
	t_cabecera cabecera = recibirCabecera(socket);
	return cabecera.tipoDeMensaje;
}

void prot_enviar_select(char *nom_tabla, uint16_t key, int socket){
	t_paquete* paquete = crear_paquete(SELECT);
	agregar_string_a_paquete(paquete, nom_tabla, strlen(nom_tabla)+1);
	agregar_uint16_t_a_paquete(paquete, key);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

tp_select prot_recibir_select(int tamanio_paq, int socket){
	void * buffer = malloc(tamanio_paq);
	recibir(socket, buffer, tamanio_paq);
	int tamanio_nom_tabla;
	uint16_t key;
	int desplazamiento = 0;
	memcpy(&tamanio_nom_tabla, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	char* nom_tabla=malloc(tamanio_nom_tabla);
	memcpy(nom_tabla, buffer+desplazamiento, tamanio_nom_tabla);
	desplazamiento+=tamanio_nom_tabla;
	memcpy(&key, buffer+desplazamiento, sizeof(uint16_t));
	tp_select param_select=malloc(sizeof(t_select));
	param_select->nom_tabla=nom_tabla;
	param_select->key=key;
	free(buffer);
	return param_select;
}

void prot_enviar_respuesta_select(char * value,uint16_t key,double timestamp, int socket){
	t_paquete* paquete = crear_paquete(REQUEST_SUCCESS);
	agregar_string_a_paquete(paquete, value, strlen(value)+1);
	agregar_uint16_t_a_paquete(paquete,key);
	agregar_double_a_paquete(paquete,timestamp);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

tp_select_rta prot_recibir_respuesta_select(int tamanio_paq,int socket){
	//posibles MENSAJES a recibir del fs: REQUEST_SUCCESS, TABLA_NO_EXISTIA
	void * buffer = malloc(tamanio_paq);
	recibir(socket, buffer, tamanio_paq);
	int tamanio_value;
	uint16_t key;
	double timestamp;
	int desplazamiento = 0;
	memcpy(&tamanio_value, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	char* value=malloc(tamanio_value);
	memcpy(value, buffer+desplazamiento, tamanio_value);
	desplazamiento+=tamanio_value;
	memcpy(&key, buffer+desplazamiento, sizeof(uint16_t));
	desplazamiento+=sizeof(uint16_t);
	memcpy(&timestamp, buffer+desplazamiento, sizeof(double));
	tp_select_rta param_select_rta =malloc(sizeof(t_select_rta));
	param_select_rta->value=value;
	param_select_rta->key = key;
	param_select_rta->timestamp = timestamp;
	free(buffer);
	return param_select_rta;
}

void prot_enviar_respuesta_insert(int socket){
	t_paquete* paquete = crear_paquete(REQUEST_SUCCESS);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

enum MENSAJES prot_recibir_respuesta_insert(int socket){
	t_cabecera cabecera = recibirCabecera(socket);
	return cabecera.tipoDeMensaje;
}

void prot_enviar_insert(char *nom_tabla, uint16_t key, char * value, double timestamp, int socket){
	t_paquete* paquete = crear_paquete(INSERT);
	agregar_string_a_paquete(paquete, nom_tabla, strlen(nom_tabla)+1);
	agregar_uint16_t_a_paquete(paquete, key);
	agregar_string_a_paquete(paquete, value, strlen(value)+1);
	agregar_double_a_paquete(paquete, timestamp);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

tp_insert prot_recibir_insert(int tamanio_paq, int socket){
	void * buffer = malloc(tamanio_paq);
	recibir(socket, buffer, tamanio_paq);
	int tamanio_nom_tabla, tamanio_value;
	uint16_t key;
	double timestamp;
	int desplazamiento = 0;
	memcpy(&tamanio_nom_tabla, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	char* nom_tabla=malloc(tamanio_nom_tabla);
	memcpy(nom_tabla, buffer+desplazamiento, tamanio_nom_tabla);
	desplazamiento+=tamanio_nom_tabla;
	memcpy(&key, buffer+desplazamiento, sizeof(uint16_t));
	desplazamiento+= sizeof(uint16_t);
	memcpy(&tamanio_value, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	char* value=malloc(tamanio_value);
	memcpy(value, buffer+desplazamiento, tamanio_value);
	desplazamiento+=tamanio_value;
	memcpy(&timestamp, buffer+desplazamiento, sizeof(double));
	tp_insert param_insert=malloc(sizeof(t_insert));
	param_insert->nom_tabla=nom_tabla;
	param_insert->key=key;
	param_insert->value=value;
	param_insert->timestamp=timestamp;
	free(buffer);
	return param_insert;
}

void prot_enviar_drop(char * nom_tabla,int socket){
	t_paquete* paquete = crear_paquete(DROP);
	agregar_string_a_paquete(paquete, nom_tabla, strlen(nom_tabla)+1);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

tp_drop prot_recibir_drop(int tamanio_paq, int socket){
	void * buffer = malloc(tamanio_paq);
	recibir(socket, buffer, tamanio_paq);
	int tamanio_nom_tabla;
	int desplazamiento = 0;
	memcpy(&tamanio_nom_tabla, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	char* nom_tabla=malloc(tamanio_nom_tabla);
	memcpy(nom_tabla, buffer+desplazamiento, tamanio_nom_tabla);
	tp_drop param_drop=malloc(sizeof(t_drop));
	param_drop->nom_tabla=nom_tabla;
	free(buffer);
	return param_drop;
}

void prot_enviar_respuesta_drop(int socket){
	t_paquete* paquete = crear_paquete(REQUEST_SUCCESS);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

enum MENSAJES prot_recibir_respuesta_drop(int socket){
	//posibles MENSAJES a recibir del fs: REQUEST_SUCCESS, TABLA_NO_EXISTIA
	t_cabecera cabecera = recibirCabecera(socket);
	return cabecera.tipoDeMensaje;
}

void prot_enviar_describe(char *nom_tabla,int socket){//para una tabla
	t_paquete* paquete = crear_paquete(DESCRIBE);
	agregar_string_a_paquete(paquete, nom_tabla, strlen(nom_tabla)+1);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

tp_describe prot_recibir_describe(int tamanio_paq, int socket){
	void * buffer = malloc(tamanio_paq);
	recibir(socket, buffer, tamanio_paq);
	int tamanio_nom_tabla;
	int desplazamiento = 0;
	memcpy(&tamanio_nom_tabla, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	char* nom_tabla=malloc(tamanio_nom_tabla);
	memcpy(nom_tabla, buffer+desplazamiento, tamanio_nom_tabla);
	tp_describe param_describe=malloc(sizeof(t_describe));
	param_describe->nom_tabla=nom_tabla;
	free(buffer);
	return param_describe;
}

void prot_enviar_respuesta_describe(char* nom_tabla,int particiones, char* consistencia, int tiempo_compactacion, int socket){
	t_paquete* paquete = crear_paquete(REQUEST_SUCCESS);
	agregar_string_a_paquete(paquete,nom_tabla,strlen(nom_tabla)+1);
	agregar_int_a_paquete(paquete,particiones);
	agregar_string_a_paquete(paquete, consistencia, strlen(consistencia)+1);
	agregar_int_a_paquete(paquete,tiempo_compactacion);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

tp_describe_rta prot_recibir_respuesta_describe(int tamanio_paq,int socket){
	//posibles MENSAJES a recibir del fs: REQUEST_SUCCESS, TABLA_NO_EXISTIA
	void * buffer = malloc(tamanio_paq);
	recibir(socket, buffer, tamanio_paq);
	int tamanio_nom_tabla;
	int particiones;
	int tamanio_consistencia;
	int tiempo_compactacion;
	int desplazamiento = 0;
	memcpy(&tamanio_nom_tabla, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	char* nom_tabla = malloc(tamanio_nom_tabla);
	memcpy(nom_tabla, buffer+desplazamiento, tamanio_nom_tabla);
	desplazamiento+=tamanio_nom_tabla;
	memcpy(&particiones, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(&tamanio_consistencia, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	char* consistencia=malloc(tamanio_consistencia);
	memcpy(consistencia, buffer+desplazamiento, tamanio_consistencia);
	desplazamiento+=tamanio_consistencia;
	memcpy(&tiempo_compactacion, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);
	tp_describe_rta param_describe_rta =malloc(sizeof(t_describe_rta));
	param_describe_rta->nombre=nom_tabla;
	param_describe_rta->particiones=particiones;
	param_describe_rta->consistencia=consistencia;
	param_describe_rta->tiempoDeCompactacion=tiempo_compactacion;
	free(buffer);
	return param_describe_rta;
}

void prot_enviar_describeAll(int socket){
	t_paquete* paquete = crear_paquete(DESCRIBE);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

void prot_enviar_respuesta_describeAll(t_describeAll_rta descripciones, int socket){
	t_paquete* paquete = crear_paquete(REQUEST_SUCCESS);
	int cantidad_descripciones = descripciones.lista->elements_count;
	agregar_int_a_paquete(paquete,cantidad_descripciones);
	for(int i=0;i<cantidad_descripciones;i++){
		tp_describe_rta descriptor = (tp_describe_rta)list_get(descripciones.lista,i);
		agregar_string_a_paquete(paquete,descriptor->nombre,strlen(descriptor->nombre)+1);
		agregar_int_a_paquete(paquete,descriptor->particiones);
		agregar_string_a_paquete(paquete, descriptor->consistencia, strlen(descriptor->consistencia)+1);
		agregar_int_a_paquete(paquete,descriptor->tiempoDeCompactacion);
	}
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

tp_describeAll_rta prot_recibir_respuesta_describeAll(int tamanio_paq, int socket){
	//posibles MENSAJES a recibir del fs: REQUEST_SUCCESS, TABLA_NO_EXISTIA(no hay tablas en el fs)
	void * buffer = malloc(tamanio_paq);
	recibir(socket, buffer, tamanio_paq);
	int cantidad_descripciones;
	int desplazamiento=0;
	int tamanio_nombre;
	int tamanio_consistencia;
	tp_describeAll_rta descripciones = malloc(sizeof(t_describeAll_rta));
	descripciones->lista = list_create();

	memcpy(&cantidad_descripciones, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	for(int i =0;i<cantidad_descripciones;i++){//agrego cada descriptor
		tp_describe_rta descriptor = malloc(sizeof(t_describe_rta));

		memcpy(&tamanio_nombre,buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		descriptor->nombre = malloc(tamanio_nombre);
		memcpy(descriptor->nombre,buffer+desplazamiento,tamanio_nombre);
		desplazamiento+=tamanio_nombre;
		memcpy(&descriptor->particiones, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(&tamanio_consistencia, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		descriptor->consistencia=malloc(tamanio_consistencia);
		memcpy(descriptor->consistencia, buffer+desplazamiento, tamanio_consistencia);
		desplazamiento+=tamanio_consistencia;
		memcpy(&descriptor->tiempoDeCompactacion, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);

		list_add(descripciones->lista,descriptor);
	}
	free(buffer);
	return descripciones;
}
void free_tp_describeAll_rta(void* d){
	tp_describe_rta unDescriptor = d;
	free(unDescriptor->nombre);
	free(unDescriptor->consistencia);
	free(unDescriptor);
}

void prot_free_tp_describeAll_rta(tp_describeAll_rta descriptores){
	//una unica funcion para liberar todo lo referido a la lista, no usar nada mas
	list_destroy_and_destroy_elements(descriptores->lista,free_tp_describeAll_rta);
	free(descriptores);
}

void prot_enviar_journal(int socket){
	t_paquete* paquete = crear_paquete(JOURNAL);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

void prot_enviar_respuesta_journaling(int socket){
	t_paquete* paquete = crear_paquete(REQUEST_SUCCESS);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

enum MENSAJES prot_recibir_respuesta_journal(int socket){
	t_cabecera cabecera = recibirCabecera(socket);
	return cabecera.tipoDeMensaje;
}

void prot_enviar_pedido_tabla_gossiping(int socket){
	t_paquete* paquete = crear_paquete(PEDIDO_KERNEL_GOSSIP);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

void prot_enviar_mi_tabla_gossiping_a_kernel(t_tabla_gossiping tabla_de_gossip, int socket){
	t_paquete* paquete = crear_paquete(PEDIDO_KERNEL_GOSSIP);
	enviar_tabla_gossip(tabla_de_gossip, socket, paquete);
}

void enviar_tabla_gossip(t_tabla_gossiping tabla_de_gossip, int socket,
		t_paquete* paquete) {
	int cantidad_descripciones = tabla_de_gossip.lista->elements_count;
	agregar_int_a_paquete(paquete, cantidad_descripciones);
	for (int i = 0; i < cantidad_descripciones; i++) {
		tp_memo_del_pool descriptor = (tp_memo_del_pool) list_get(
				tabla_de_gossip.lista, i);
		agregar_string_a_paquete(paquete, descriptor->ip,
				strlen(descriptor->ip) + 1);
		agregar_string_a_paquete(paquete, descriptor->puerto,
				strlen(descriptor->puerto) + 1);
	}
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

void prot_enviar_mi_tabla_gossiping(t_tabla_gossiping tabla_de_gossip, int socket){
	t_paquete* paquete = crear_paquete(RECIBIR_GOSSIPING);
	enviar_tabla_gossip(tabla_de_gossip, socket, paquete);
}

void prot_enviar_y_esperar_tabla_gossiping(t_tabla_gossiping tabla_de_gossip, int socket){
	t_paquete* paquete = crear_paquete(ENVIAR_Y_RECIBIR_GOSSIPING);
	enviar_tabla_gossip(tabla_de_gossip, socket, paquete);
}

tp_tabla_gossiping prot_recibir_tabla_gossiping(int tamanio_paq, int socket){
	void * buffer = malloc(tamanio_paq);
	recibir(socket, buffer, tamanio_paq);
	int cantidad_descripciones;
	int desplazamiento=0;
	int tamanio_ip;
	int tamanio_puerto;
	tp_tabla_gossiping descripciones = malloc(sizeof(t_tabla_gossiping));
	descripciones->lista=list_create();

	memcpy(&cantidad_descripciones, buffer + desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	for(int i=0; i<cantidad_descripciones;i++){
		tp_memo_del_pool descriptor = malloc(sizeof(t_memo_del_pool));

		memcpy(&tamanio_ip, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		descriptor->ip=malloc(tamanio_ip);
		memcpy(descriptor->ip, buffer+desplazamiento, tamanio_ip);
		desplazamiento+=tamanio_ip;
		memcpy(&tamanio_puerto, buffer+desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		descriptor->puerto=malloc(tamanio_puerto);
		memcpy(descriptor->puerto, buffer+desplazamiento, tamanio_puerto);
		desplazamiento+=tamanio_puerto;

		list_add(descripciones->lista,descriptor);
	}
	free(buffer);
	return descripciones;
}

void free_tp_tabla_gossiping(void* d){
	tp_memo_del_pool unDescriptor = d;
	free(unDescriptor->ip);
	free(unDescriptor->puerto);
	free(unDescriptor);
}

void prot_free_tp_tabla_gossiping(tp_tabla_gossiping descriptores){
	//una unica funcion para liberar todo lo referido a la lista, no usar nada mas
	list_destroy_and_destroy_elements(descriptores->lista,free_tp_tabla_gossiping);
	free(descriptores);
}

