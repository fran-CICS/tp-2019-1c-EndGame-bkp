/*
 * Utilidades.c
 *
 *  Created on: 8 abr. 2019
 *      Author: utnso
 */
#include "Utilidades.h"

void iniciar_logger(void) {
	g_logger = log_create(PATH_LOG, "Memoria", false, LOG_LEVEL_DEBUG);

	logger(escribir_loguear, l_info,"Se comenzó a iniciar el proceso memoria");
}

char* reconstruir_path_archivo(char* directorio, char* nombre_archivo) {
	char* path = string_new();
	string_append(&path, directorio);
	string_append(&path, nombre_archivo);
	return path;
}

void separar_path_pasado_por_parametro(char ** nombre_archivo, char ** directorio, char ** parametros){
	char** path_a_separar = string_n_split(parametros[1], 20, "/");

	int final = 0,cont = 0;
	while(path_a_separar[final]){
		final++;
		if(path_a_separar[final]!=NULL){
			cont++;
		}
	}
	string_append(nombre_archivo, path_a_separar[cont]);
	logger(escribir_loguear, l_debug,"El nombre del archivo es %s", *nombre_archivo);

	string_append(directorio,"/");

	int cont2 = 0;
	while(cont2 <= (cont-1)){
		string_append_with_format(directorio, "%s/", path_a_separar[cont2]);
		cont2++;
	}

	logger(escribir_loguear, l_debug,"EL directorio es %s", *directorio);

	int j = 0;
	while(path_a_separar[j]){
		free(path_a_separar[j]);
		j++;
	}

	free(path_a_separar);
}

void iniciar_config(int cantidad_parametros, char ** parametros) {
	char * directorio=string_new();
	char * nombre_archivo=string_new();
	if (cantidad_parametros>1){
		separar_path_pasado_por_parametro(&nombre_archivo, &directorio, parametros);
		g_config = config_create(parametros[1]);
	} else {
		free(directorio);
		free(nombre_archivo);
		directorio=string_duplicate(DIRECTORIO_CONFIG_DEFAULT);
		nombre_archivo=string_duplicate(NOMBRE_ARCH_CONFIG_DEFAULT);
		char* path = reconstruir_path_archivo(directorio, nombre_archivo);
		g_config = config_create(path);
		free(path);
	}
	validar_apertura_archivo_configuracion();
	iniciar_escucha_cambios_conf(directorio, nombre_archivo);
}

void validar_apertura_archivo_configuracion() {
	if (g_config == NULL) {
		logger(escribir_loguear,l_error, "No encontré el archivo de configuración");
		log_destroy(g_logger);
		exit(EXIT_FAILURE);
	}
}

void iniciar_escucha_cambios_conf(char * directorio, char * nombre_archivo){
	pthread_t hilo_cambios_conf;
	t_path_archivo_conf * str_ruta_archivo_conf=malloc(sizeof(t_path_archivo_conf));
	str_ruta_archivo_conf->directorio=directorio;
	str_ruta_archivo_conf->nombre_archivo=nombre_archivo;
	int resultado_de_hacer_el_hilo = pthread_create (&hilo_cambios_conf, NULL
			, escuchar_cambios_conf, str_ruta_archivo_conf);
	if(resultado_de_hacer_el_hilo!=0){
		if(resultado_de_hacer_el_hilo!=0){
			logger(escribir_loguear,l_error
					,"Error al crear el hilo de escucha de cambios de conf, levantá la memoria de nuevo");
			free(str_ruta_archivo_conf);
			terminar_programa(EXIT_FAILURE);
		}
	}
	pthread_detach(hilo_cambios_conf);
}

void *escuchar_cambios_conf(void * estructura_path){

	pthread_mutex_lock(&M_RUTA_ARCHIVO_CONF);
	ruta_archivo_conf=estructura_path;
	pthread_mutex_unlock(&M_RUTA_ARCHIVO_CONF);

	char buffer[BUF_LEN];

	pthread_mutex_lock(&M_CONF_FD);
	conf_fd = inotify_init();
	pthread_mutex_unlock(&M_CONF_FD);

	if (conf_fd < 0) {
		logger(escribir_loguear,l_error, "No se van a poder escuchar cambios en el archivo de configuración");
		free(ruta_archivo_conf);
		return EXIT_SUCCESS;
	}
	pthread_mutex_lock(&M_WATCH_DESCRIPTOR);
	watch_descriptor = inotify_add_watch(conf_fd, ruta_archivo_conf->directorio, IN_MODIFY);
	pthread_mutex_unlock(&M_WATCH_DESCRIPTOR);

	int length = read(conf_fd, buffer, BUF_LEN);
	if (length < 0) {
		logger(escribir_loguear,l_error, "No se van a poder escuchar cambios en el archivo de configuración");
		free(ruta_archivo_conf);
		return EXIT_SUCCESS;
	}

	pthread_mutex_lock(&M_PATH_ARCHIVO_CONFIGURACION);
	path_archivo_configuracion=reconstruir_path_archivo(ruta_archivo_conf->directorio, ruta_archivo_conf->nombre_archivo);
	pthread_mutex_unlock(&M_PATH_ARCHIVO_CONFIGURACION);

	while (length>0) {
		struct inotify_event *event = (struct inotify_event *) &buffer[0];
		if (event->len>0 && (event->mask & IN_MODIFY)
				&& string_equals_ignore_case(event->name, ruta_archivo_conf->nombre_archivo)) {

			actualizar_campos_retardo(event);
			length = read(conf_fd, buffer, BUF_LEN);
			if (conf_fd < 0) {
				logger(escribir_loguear,l_error, "No se van a escuchar más cambios en el archivo de configuración");
				return EXIT_SUCCESS;
			}
		}
	}

	return EXIT_SUCCESS;
}

void actualizar_campos_retardo(struct inotify_event* event) {
	logger(escribir_loguear, l_trace,
			"\nEl archivo de configuración %s fue modificado\n", event->name);

	//Vuelve a abrir el archivo de conf para leer los nuevos valores de retardo
	t_config* l_config = config_create(path_archivo_configuracion);

	pthread_mutex_lock(&M_RETARDO_ACCESO_MEMORIA);
	RETARDO_ACCESO_MEMORIA = config_get_int_value(l_config,
			CLAVE_CONFIG_RETARDO_ACCESO_MEMORIA);
	pthread_mutex_unlock(&M_RETARDO_ACCESO_MEMORIA);

	logger(escribir_loguear, l_debug,
			"Nuevo valor de retardo de acceso a memoria principal: %d",
			RETARDO_ACCESO_MEMORIA);

	pthread_mutex_lock(&M_RETARDO_ACCESO_FILESYSTEM);
	RETARDO_ACCESO_FILESYSTEM = config_get_int_value(l_config,
			CLAVE_CONFIG_RETARDO_ACCESO_FILESYSTEM);
	pthread_mutex_unlock(&M_RETARDO_ACCESO_FILESYSTEM);

	logger(escribir_loguear, l_debug,
			"Nuevo valor de retardo de acceso a filesystem: %d",
			RETARDO_ACCESO_FILESYSTEM);
	config_destroy(l_config);
}

void obtener_valor_configuracion(char* clave, t_config* archivo, void(*obtener)(void)){
	if (config_has_property(archivo, clave)){
		obtener();
	}
}

void obtener_puerto_escucha() {
	PUERTO_ESCUCHA = config_get_string_value(g_config,
			CLAVE_CONFIG_PUERTO_ESCUCHA);
	logger(escribir_loguear, l_debug, "Se obtuvo configuración 'Puerto de escucha': %s",PUERTO_ESCUCHA);
}

void obtener_ip_filesystem(){
	IP_FILESYSTEM=config_get_string_value(g_config, CLAVE_CONFIG_IP_FILESYSTEM);
	logger(escribir_loguear, l_debug, "Se obtuvo configuración 'IP de filesystem': %s",IP_FILESYSTEM);
}

void obtener_puerto_filesystem(){
	PUERTO_FILESYSTEM=config_get_int_value(g_config, CLAVE_CONFIG_PUERTO_FILESYSTEM);
	logger(escribir_loguear, l_debug, "Se obtuvo configuración 'Puerto de filesystem': %d",PUERTO_FILESYSTEM);
}

void obtener_tamanio_memoria(){
	TAMANIO_MEMORIA=config_get_int_value(g_config, CLAVE_CONFIG_TAMANIO_MEMORIA);
	logger(escribir_loguear, l_debug, "Se obtuvo configuración 'Tamanio memoria': %d",TAMANIO_MEMORIA);
}

void obtener_numero_memoria(){
	NUMERO_MEMORIA=config_get_int_value(g_config, CLAVE_CONFIG_NUMERO_MEMORIA);
	logger(escribir_loguear, l_debug, "Se obtuvo configuración 'Numero de memoria': %d",NUMERO_MEMORIA);
}

void obtener_ip_seeds(){
	IPS_SEEDS=config_get_array_value(g_config, CLAVE_CONFIG_IP_SEEDS);
	logger(escribir_loguear, l_debug, "Se obtuvo configuración 'IP de Seeds'");
}

void obtener_puerto_seeds(){
	PUERTOS_SEEDS=config_get_array_value(g_config, CLAVE_CONFIG_PUERTO_SEEDS);
	logger(escribir_loguear, l_debug, "Se obtuvo configuración 'Puerto de Seeds'");
}

void obtener_retardo_acceso_memoria(){
	pthread_mutex_lock(&M_RETARDO_ACCESO_MEMORIA);
	RETARDO_ACCESO_MEMORIA=config_get_int_value(g_config, CLAVE_CONFIG_RETARDO_ACCESO_MEMORIA);
	pthread_mutex_unlock(&M_RETARDO_ACCESO_MEMORIA);
	logger(escribir_loguear, l_debug, "Se obtuvo configuración 'Retardo de acceso a memoria principal': %d",RETARDO_ACCESO_MEMORIA);
}

void obtener_retardo_acceso_filesystem(){
	pthread_mutex_lock(&M_RETARDO_ACCESO_FILESYSTEM);
	RETARDO_ACCESO_FILESYSTEM=config_get_int_value(g_config, CLAVE_CONFIG_RETARDO_ACCESO_FILESYSTEM);
	pthread_mutex_unlock(&M_RETARDO_ACCESO_FILESYSTEM);
	logger(escribir_loguear, l_debug, "Se obtuvo configuración 'Retardo de acceso a filesystem': %d",RETARDO_ACCESO_FILESYSTEM);
}

void obtener_tiempo_journal(){
	TIEMPO_JOURNAL=config_get_long_value(g_config, CLAVE_CONFIG_TIEMPO_JOURNAL);
	logger(escribir_loguear, l_debug, "Se obtuvo configuración 'Tiempo de journal': %d",TIEMPO_JOURNAL);
}

void obtener_tiempo_gossiping(){
	TIEMPO_GOSSIPING=config_get_long_value(g_config, CLAVE_CONFIG_TIEMPO_GOSSIPING);
	logger(escribir_loguear, l_debug, "Se obtuvo configuración 'Tiempo de gossiping': %d",TIEMPO_GOSSIPING);
}

void leer_config(void) {
	logger(escribir_loguear, l_info,"Cargando archivo de configuración...");
	obtener_valor_configuracion(CLAVE_CONFIG_PUERTO_ESCUCHA, g_config, obtener_puerto_escucha);
	obtener_valor_configuracion(CLAVE_CONFIG_IP_FILESYSTEM, g_config, obtener_ip_filesystem);
	obtener_valor_configuracion(CLAVE_CONFIG_PUERTO_FILESYSTEM, g_config, obtener_puerto_filesystem);
	obtener_valor_configuracion(CLAVE_CONFIG_TAMANIO_MEMORIA, g_config, obtener_tamanio_memoria);
	obtener_valor_configuracion(CLAVE_CONFIG_NUMERO_MEMORIA, g_config, obtener_numero_memoria);

	obtener_valor_configuracion(CLAVE_CONFIG_IP_SEEDS, g_config, obtener_ip_seeds);
	obtener_valor_configuracion(CLAVE_CONFIG_PUERTO_SEEDS, g_config, obtener_puerto_seeds);
	construir_lista_seeds();

	obtener_valor_configuracion(CLAVE_CONFIG_RETARDO_ACCESO_MEMORIA, g_config, obtener_retardo_acceso_memoria);
	obtener_valor_configuracion(CLAVE_CONFIG_RETARDO_ACCESO_FILESYSTEM, g_config, obtener_retardo_acceso_filesystem);
	obtener_valor_configuracion(CLAVE_CONFIG_TIEMPO_JOURNAL, g_config, obtener_tiempo_journal);
	obtener_valor_configuracion(CLAVE_CONFIG_TIEMPO_GOSSIPING, g_config, obtener_tiempo_gossiping);
	logger(escribir_loguear, l_info,"Se cargó archivo de configuración exitosamente");
}

void construir_lista_seeds(){
	seeds=list_create();
	for(int i=0;IPS_SEEDS[i]!=NULL;i++){
		t_memo_del_pool *memoria_del_pool=
				crear_memo_del_pool(string_duplicate(IPS_SEEDS[i]),string_duplicate(PUERTOS_SEEDS[i]));
		list_add(seeds, memoria_del_pool);
	}
}

void configurar_signals(void) {
	struct sigaction signal_struct;
	signal_struct.sa_handler = captura_signal;
	signal_struct.sa_flags = 0;

	sigemptyset(&signal_struct.sa_mask);

	sigaddset(&signal_struct.sa_mask, SIGPIPE);
	if (sigaction(SIGPIPE, &signal_struct, NULL) < 0) {
		logger(escribir_loguear, l_error, " SIGACTION error ");
	}

	sigaddset(&signal_struct.sa_mask, SIGINT);
	if (sigaction(SIGINT, &signal_struct, NULL) < 0) {
		logger(escribir_loguear, l_error, " SIGACTION error ");
	}

}

void captura_signal(int signo){

    if(signo == SIGINT)
    {
    	logger(escribir_loguear, l_warning,"Finalizando proceso memoria...");
    	terminar_programa(EXIT_SUCCESS);
    }
    else if(signo == SIGPIPE)
    {
    	logger(escribir_loguear, l_error," Se desconectó un proceso al que se quizo enviar.");
    }

}

void escribir_por_pantalla(int tipo_esc, int tipo_log, char* console_buffer,
		char* log_colors[8], char* msj_salida) {

	if ((tipo_esc == escribir) || (tipo_esc == escribir_loguear)) {
		console_buffer = string_from_format("%s%s%s", log_colors[tipo_log],
				msj_salida, log_colors[0]);
		printf("%s", console_buffer);
		printf("%s","\n");
		fflush(stdout);
		free(console_buffer);
	}
}

void definir_nivel_y_loguear(int tipo_esc, int tipo_log, char* msj_salida) {
	if ((tipo_esc == loguear) || (tipo_esc == escribir_loguear)) {
		if (tipo_log == l_info) {
			log_info(g_logger, msj_salida);
		} else if (tipo_log == l_warning) {
			log_warning(g_logger, msj_salida);
		} else if (tipo_log == l_error) {
			log_error(g_logger, msj_salida);
		} else if (tipo_log == l_debug) {
			log_debug(g_logger, msj_salida);
		} else if (tipo_log == l_trace) {
			log_trace(g_logger, msj_salida);
		}
	}
}

void logger(int tipo_esc, int tipo_log, const char* mensaje, ...){

	//Colores (reset,vacio,vacio,cian,verde,vacio,amarillo,rojo)
	char *log_colors[8] = {"\x1b[0m","","","\x1b[36m", "\x1b[32m", "", "\x1b[33m", "\x1b[31m" };
	char *console_buffer=NULL;
	char *msj_salida = malloc(sizeof(char) * 256);

	//Captura los argumentos en una lista
	va_list args;
	va_start(args, mensaje);

	//Arma el mensaje formateado con sus argumentos en msj_salida.
	vsprintf(msj_salida, mensaje, args);

	escribir_por_pantalla(tipo_esc, tipo_log, console_buffer, log_colors,
			msj_salida);

	definir_nivel_y_loguear(tipo_esc, tipo_log, msj_salida);

	va_end(args);
	free(msj_salida);

	return;
}

void terminar_programa(int codigo_finalizacion){
	logger(escribir_loguear, l_warning,"Se va a finalizar la memoria %d. Bye.", NUMERO_MEMORIA);
	log_destroy(g_logger);
	config_destroy(g_config);
	free(MEMORIA_PRINCIPAL);

	void destructor_memoria_del_pool(void * memo_del_pool){
		free(((t_memo_del_pool*) memo_del_pool)->ip);
		free(((t_memo_del_pool*) memo_del_pool)->puerto);
		free((t_memo_del_pool *) memo_del_pool);
	}
	if(mi_tabla_de_gossip!=NULL && !list_is_empty(mi_tabla_de_gossip)){
		char* ip_address = conocer_ip_propia();
		bool es_memoria_propia(void* memoria){
			return string_equals_ignore_case(((t_memo_del_pool*) memoria)->ip, ip_address);
		}
		void destructor_memoria_propia(void * memo_del_pool){
			free(((t_memo_del_pool*) memo_del_pool)->ip);
			free((t_memo_del_pool *) memo_del_pool);
		}
		pthread_mutex_lock(&M_TABLA_GOSSIP);
		list_remove_and_destroy_by_condition(mi_tabla_de_gossip, es_memoria_propia, destructor_memoria_propia);
		pthread_mutex_unlock(&M_TABLA_GOSSIP);

		bool no_es_una_seed(void* memoria){
			if(list_is_empty(seeds)){
				return true;
			}
			bool es_memoria_seed(void* memoria_seed){
				return ((t_memo_del_pool*) memoria_seed)==((t_memo_del_pool*) memoria);
			}
			return !list_any_satisfy(seeds, es_memoria_seed);
		}
		if(!list_is_empty(mi_tabla_de_gossip)){
			pthread_mutex_lock(&M_TABLA_GOSSIP);
			list_remove_and_destroy_by_condition(mi_tabla_de_gossip, no_es_una_seed, destructor_memoria_del_pool);
			pthread_mutex_unlock(&M_TABLA_GOSSIP);
		}
		pthread_mutex_lock(&M_TABLA_GOSSIP);
		list_destroy(mi_tabla_de_gossip);
		pthread_mutex_unlock(&M_TABLA_GOSSIP);
		free(ip_address);
	}

	if(seeds!=NULL && !list_is_empty(seeds)){
		list_destroy_and_destroy_elements(seeds, destructor_memoria_del_pool);
	}

	pthread_mutex_lock(&M_WATCH_DESCRIPTOR);
	inotify_rm_watch(conf_fd, watch_descriptor);
	pthread_mutex_unlock(&M_WATCH_DESCRIPTOR);

	pthread_mutex_lock(&M_CONF_FD);
	close(conf_fd);
	pthread_mutex_unlock(&M_CONF_FD);

	pthread_mutex_lock(&M_PATH_ARCHIVO_CONFIGURACION);
	free(path_archivo_configuracion);
	pthread_mutex_unlock(&M_PATH_ARCHIVO_CONFIGURACION);

	pthread_mutex_lock(&M_RUTA_ARCHIVO_CONF);
	free(ruta_archivo_conf->directorio);
	free(ruta_archivo_conf->nombre_archivo);
	free(ruta_archivo_conf);
	pthread_mutex_unlock(&M_RUTA_ARCHIVO_CONF);

	if(tabla_de_segmentos!=NULL){
		liberar_tabla_segmentos();
	}
	if(bitmap_marcos!=NULL){
		liberar_bitmap_marcos();
	}

	if(SOCKET_LISS>0){
		close(SOCKET_LISS);
	}
	close(SERVER_MEMORIA);

	apagar_semaforos();
	exit(codigo_finalizacion);
}

int iniciar_servidor(){
	int server_socket = escucharEn(atoi(PUERTO_ESCUCHA));

	if(server_socket < 0)
	{
		logger(escribir_loguear,l_error,"Falló la creación del socket servidor\n");
		exit(1);
	}
	else
	{
		logger(escribir_loguear,l_trace,"Socket servidor (%d) escuchando", server_socket);
	}
	SERVER_MEMORIA=server_socket;

	return server_socket;
}

struct addrinfo* crear_addrinfo(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	getaddrinfo("127.0.0.1", PUERTO_ESCUCHA, &hints, &serverInfo);
	return serverInfo;
}

void mostrar_mensaje_previa_conexion_con(char *proceso, char *ip, int puerto){
	logger(escribir_loguear, l_info, "Se intentara conectar a la ip %s, puerto %d de %s", ip, puerto, proceso);
}

void validar_comunicacion(int socket, char* proceso){
	if(socket<0){
		logger(escribir_loguear,l_error, "No se pudo conectar con el proceso %s ", proceso);
		cerrar_socket_y_terminar(socket);
	}
}

void cerrar_socket_y_terminar(int socket){
	terminar_programa(EXIT_FAILURE);
}

void loguear_handshake_erroneo(char* proceso) {
	logger(escribir_loguear, l_error,
			"No se pudo hacer handshake con el proceso %s ", proceso);
}

void loguear_handshake_exitoso(int socket, char* proceso) {
	logger(escribir_loguear, l_info,
			"Se realizo el handshake con %s en el socket %d", proceso, socket);
}

void mandar_handshake_a(char * proceso, int socket, enum PROCESO enumProceso){
	logger(escribir_loguear, l_info, "Se intentara mandar handshake a %s", proceso);
	if (enviarHandshake(MEMORIA, enumProceso, socket) == 0) {
		loguear_handshake_erroneo(proceso);
		cerrar_socket_y_terminar(socket);
	}
	loguear_handshake_exitoso(socket, proceso);
}

void enviar_sin_esperar_tabla_gossip(int socket) {
	t_tabla_gossiping descriptores;
	descriptores.lista = mi_tabla_de_gossip;
	prot_enviar_mi_tabla_gossiping(descriptores, socket);
	logger(escribir_loguear, l_info, "Envie mi tabla de gossip por el socket %d",socket);
}

void enviar_y_esperar_tabla_gossip(int socket) {
	t_tabla_gossiping descriptores;
	descriptores.lista = mi_tabla_de_gossip;
	prot_enviar_y_esperar_tabla_gossiping(descriptores, socket);
	logger(escribir_loguear, l_info, "Envie mi tabla de gossip por el socket %d y la voy a recibir",socket);
}

void recibir_handshakes(int socket){
	enum PROCESO procesoRecibido;
	recibir(socket,&procesoRecibido,sizeof(procesoRecibido));
	enum PROCESO procesoMemoria = MEMORIA;
	enviar(socket, &procesoMemoria, sizeof(MEMORIA));
	if(procesoRecibido == KERNEL) {
		loguear_handshake_exitoso(socket, _KERNEL);
		prot_enviar_int(NUMERO_MEMORIA,socket);
	}
	if(procesoRecibido == MEMORIA){
		loguear_handshake_exitoso(socket, _OTRA_MEMORIA);
	}
}

void inicializar_semaforos(){
	pthread_mutex_init(&M_RETARDO_ACCESO_MEMORIA, NULL);
	pthread_mutex_init(&M_RETARDO_ACCESO_FILESYSTEM, NULL);
	pthread_mutex_init(&M_WATCH_DESCRIPTOR, NULL);
	pthread_mutex_init(&M_CONF_FD, NULL);
	pthread_mutex_init(&M_PATH_ARCHIVO_CONFIGURACION, NULL);
	pthread_mutex_init(&M_RUTA_ARCHIVO_CONF, NULL);
	pthread_mutex_init(&M_JOURNALING, NULL);
	pthread_mutex_init(&M_TABLA_GOSSIP, NULL);
}

void apagar_semaforos(){
	pthread_mutex_destroy(&M_RETARDO_ACCESO_MEMORIA);
	pthread_mutex_destroy(&M_RETARDO_ACCESO_FILESYSTEM);
	pthread_mutex_destroy(&M_WATCH_DESCRIPTOR);
	pthread_mutex_destroy(&M_CONF_FD);
	pthread_mutex_destroy(&M_PATH_ARCHIVO_CONFIGURACION);
	pthread_mutex_destroy(&M_RUTA_ARCHIVO_CONF);
	pthread_mutex_destroy(&M_JOURNALING);
	pthread_mutex_destroy(&M_TABLA_GOSSIP);
}

void imprimir_informacion_memoria_ajena(void * memoria_ajena){
	logger(escribir_loguear, l_debug, "El ip de la memoria es: %s", (*(t_memo_del_pool*)memoria_ajena).ip);
	logger(escribir_loguear, l_debug, "El puerto de la memoria es: %s", (*(t_memo_del_pool*)memoria_ajena).puerto);
}

void obtener_y_combinar_tablas_gossip(int socket, int tamanio) {
	logger(escribir_loguear, l_debug,
			"Voy a recibir la tabla de gossiping de la memoria en el socket: %d",
			socket);
	tp_tabla_gossiping tabla_ajena = prot_recibir_tabla_gossiping(tamanio,
			socket);
	//list_iterate(tabla_ajena->lista, imprimir_informacion_memoria_ajena);
	agregar_memorias_no_existentes_en_mi_tabla_gossip(tabla_ajena->lista);
	//Libero la lista
	prot_free_tp_tabla_gossiping(tabla_ajena);
}

void recibir_tabla_de_gossip(int socket, int tamanio){
	obtener_y_combinar_tablas_gossip(socket, tamanio);
}

void recibir_y_enviar_tabla_de_gossip(int socket, int tamanio){
	obtener_y_combinar_tablas_gossip(socket, tamanio);
	enviar_sin_esperar_tabla_gossip(socket);
}

void agregar_memorias_no_existentes_en_mi_tabla_gossip(t_list * memorias){
	void verificar_si_memoria_existe_en_mi_tabla_para_agregarla(void * memoria_ajena){
		bool memoria_existe_en_mi_tabla(void * memoria){
			return string_equals_ignore_case(((t_memo_del_pool*)memoria_ajena)->ip,((t_memo_del_pool*)memoria)->ip)
					&& string_equals_ignore_case(((t_memo_del_pool*)memoria_ajena)->puerto,((t_memo_del_pool*)memoria)->puerto);
		}
		if(!list_any_satisfy(mi_tabla_de_gossip, memoria_existe_en_mi_tabla)){
			pthread_mutex_lock(&M_TABLA_GOSSIP);
			list_add(mi_tabla_de_gossip,
					crear_memo_del_pool(string_duplicate(((t_memo_del_pool*)memoria_ajena)->ip),
							string_duplicate(((t_memo_del_pool*)memoria_ajena)->puerto)));
			pthread_mutex_unlock(&M_TABLA_GOSSIP);
		}
	}
	list_iterate(memorias, verificar_si_memoria_existe_en_mi_tabla_para_agregarla);
}

enum MENSAJES notificar_escrituras_en_memoria_LFS(int socket_con_LFS){
	enum MENSAJES* resultado_anterior = malloc(sizeof(enum MENSAJES));
	*resultado_anterior=REQUEST_SUCCESS;
	pthread_mutex_lock(&M_JOURNALING);

	t_list* paginas_modificadas=recopilar_paginas_modificadas();

	insertar_cada_registro_modificado_en_LFS(resultado_anterior, paginas_modificadas, socket_con_LFS);
	limpiar_tablas_de_segmentos_y_paginas();

	pthread_mutex_unlock(&M_JOURNALING);

	list_destroy(paginas_modificadas);

	enum MENSAJES resultado=*resultado_anterior;
	free(resultado_anterior);
	return resultado;
}

void remover_memoria_cerrada_de_tabla_gossip(int un_socket){
	bool es_socket_de_memoria(void* memoria){
		return ((t_memo_del_pool*) memoria)->socket==un_socket;
	}
	void reiniciar_socket_memoria(void * memoria){
		((t_memo_del_pool*) memoria)->socket=0;
		logger(escribir_loguear, l_trace, "Removí la memoria %s, %s de mi tabla de gossip",
				((t_memo_del_pool*) memoria)->ip, ((t_memo_del_pool*) memoria)->puerto);
	}
	pthread_mutex_lock(&M_TABLA_GOSSIP);
	list_remove_and_destroy_by_condition(mi_tabla_de_gossip, es_socket_de_memoria, reiniciar_socket_memoria);
	pthread_mutex_unlock(&M_TABLA_GOSSIP);
}

t_memo_del_pool* crear_memo_del_pool(char* ip, char* puerto) {
	t_memo_del_pool* memoria_a_utilizar = malloc(sizeof(t_memo_del_pool));
	memoria_a_utilizar->ip = ip;
	memoria_a_utilizar->puerto = puerto;
	memoria_a_utilizar->socket = 0;
	return memoria_a_utilizar;
}

double obtenerTimestamp(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long result = (((unsigned long long)tv.tv_sec)*1000+((unsigned long long)tv.tv_usec)/1000);
	double a = result;
	return a;
}
