/*
 * Consola.c
 *
 *  Created on: 15 abr. 2019
 *      Author: utnso
 */

#include "Consola.h"

void loguear_cant_menor_params(char *nombre_comando) {
	logger(escribir_loguear, l_error,
			"La cantidad de parámetros es menor a los necesarios para el comando %s",nombre_comando);
}

void loguear_comienzo_ejecucion_sentencia(char *nombre_comando) {
	logger(escribir_loguear, l_trace, "Se ejecutará la sentencia %s",nombre_comando);
}

void loguear_cant_mayor_params(char *nombre_comando) {
	logger(escribir_loguear, l_warning
			, "La cantidad de parámetros es mayor a los necesarios para el comando %s",nombre_comando);
}

void loguear_param_erroneo(char *parametro) {
	logger(escribir_loguear, l_error,
			"El parámetro %s es erróneo",parametro);
}

void limpiar_dos_parametros(char* param1, char* param2) {
	limpiar_parametro(param1);
	limpiar_parametro(param2);
}

void consola_select(char** comandos){
	int cant_param_correcta = 0;
	char *nombre_tabla = NULL;
	char *string_key = NULL;
	obtener_dos_parametros(comandos, &nombre_tabla, &string_key, _SELECT);
	cant_param_correcta=validar_parametro_consola(&nombre_tabla);
	cant_param_correcta=validar_parametro_consola(&string_key);
	if(cant_param_correcta<0){
		loguear_cant_menor_params(_SELECT);
		return;
	}

	loguear_dos_parametros_recibidos(nombre_tabla,string_key);
	char* end;
	int key=strtol(string_key, &end, 10);
	if(*end){
		loguear_param_erroneo(string_key);
		limpiar_dos_parametros(nombre_tabla, string_key);
		return;
	}

	pthread_mutex_lock(&M_JOURNALING);
	loguear_comienzo_ejecucion_sentencia(_SELECT);
	tp_select_rta_a_kernel rta_select = realizar_select(nombre_tabla, key);
	pthread_mutex_unlock(&M_JOURNALING);

	if(rta_select->respuesta==NO_HAY_MAS_MARCOS_EN_LA_MEMORIA){
		loguear_no_hay_mas_marcos_libres();
	}

	if(rta_select->value!=NULL){
		free(rta_select->value);
	}
	free(rta_select);

	limpiar_dos_parametros(nombre_tabla, string_key);
}

void limpiar_cuatro_parametros(char* param1, char* param2, char* param3,
		char* param4) {
	//Limpio el nombre_tabla
	limpiar_parametro(param1);
	//Limpio la key
	limpiar_parametro(param2);
	//Limpio la value
	limpiar_parametro(param3);
	//Limpio la timestamp
	limpiar_parametro(param4);
}

void logeuar_param_err_limpiar_cuatro_restantes(char* param1, char* param2,
		char* param3, char* param4) {
	loguear_param_erroneo(param1);
	limpiar_cuatro_parametros(param2, param1, param3, param4);
}

void loguear_no_hay_mas_marcos_libres() {
	logger(escribir_loguear, l_error,
			"No hay mas marcos libres en esta memoria, ejecutar Journal");
}

void consola_insert(char * comando_puro, char** comandos){
	int cant_param_correcta = 0;
	char *nombre_tabla = NULL;
	char *string_key = NULL;
	char *value = NULL;
	char *string_timestamp = NULL;
	obtener_cuatro_parametros_insert(comando_puro, comandos
			, &nombre_tabla, &string_key, &value, &string_timestamp, _INSERT);
	cant_param_correcta=validar_parametro_consola(&nombre_tabla);
	cant_param_correcta=validar_parametro_consola(&string_key);
	cant_param_correcta=validar_parametro_consola(&value);
	validar_parametro_consola(&string_timestamp);
	if(cant_param_correcta<0){
		logger(escribir_loguear, l_error,
					"Los parámetros del insert son incorrectos");
		return;
	}

	loguear_tres_parametros_recibidos(nombre_tabla, string_key, value);
	char* end_key;
	uint16_t key=(uint16_t)strtol(string_key, &end_key, 10);
	if(*end_key){
		logeuar_param_err_limpiar_cuatro_restantes(string_key, nombre_tabla,
				value, string_timestamp);
		return;
	}
	double timestamp;
	if(string_contains(string_timestamp, "\"")){
		free(string_timestamp);
		string_timestamp="-1";
	}
	if(string_equals_ignore_case(string_timestamp, "-1")){
		timestamp=obtenerTimestamp();
	} else {
		char* end_timestamp;
		timestamp=strtod(string_timestamp, &end_timestamp);
		if(*end_timestamp){
			logeuar_param_err_limpiar_cuatro_restantes(string_timestamp, nombre_tabla,
					value, string_key);
			return;
		}
	}
	logger(escribir_loguear, l_debug,"El timestamp fue '%.0f'",timestamp);

	if(strlen(value)>TAMANIO_VALUE){
		logger(escribir_loguear, l_error,"El tamanio del value excede al permitido, volve a intentar");
	}else{
		pthread_mutex_lock(&M_JOURNALING);
		loguear_comienzo_ejecucion_sentencia(_INSERT);
		int resultado_insert=realizar_insert(nombre_tabla, timestamp, key, value);
		pthread_mutex_unlock(&M_JOURNALING);
		if(resultado_insert<0){
			loguear_no_hay_mas_marcos_libres();
		}
	}

	//Limpio el nombre_tabla
	limpiar_cuatro_parametros(nombre_tabla, string_key, value,
			string_timestamp);
}

void consola_create(char** comandos){
	int cant_param_correcta = 0;
	char *nombre_tabla = NULL;
	char *tipo_consistencia = NULL;
	char *string_numero_particiones = NULL;
	char *string_compaction_time = NULL;
	obtener_cuatro_parametros(comandos, &nombre_tabla, &tipo_consistencia
			, &string_numero_particiones, &string_compaction_time, _CREATE);
	cant_param_correcta=validar_parametro_consola(&nombre_tabla);
	cant_param_correcta=validar_parametro_consola(&tipo_consistencia);
	cant_param_correcta=validar_parametro_consola(&string_numero_particiones);
	cant_param_correcta=validar_parametro_consola(&string_compaction_time);
	if(cant_param_correcta<0){
		loguear_cant_menor_params(_CREATE);
		return;
	}

	loguear_cuatro_parametros_recibidos(nombre_tabla, tipo_consistencia, string_numero_particiones, string_compaction_time);
	char* end_numero_particiones;
	int numero_particiones=strtol(string_numero_particiones, &end_numero_particiones, 10);
	if(*end_numero_particiones){
		logeuar_param_err_limpiar_cuatro_restantes(string_numero_particiones , tipo_consistencia,
				nombre_tabla, string_compaction_time);
		return;
	}
	char* end_compaction_time;
	long compaction_time=strtol(string_compaction_time, &end_compaction_time, 10);
	if(*end_compaction_time){
		logeuar_param_err_limpiar_cuatro_restantes(string_compaction_time, tipo_consistencia,
				string_numero_particiones, nombre_tabla);
		return;
	}

	loguear_comienzo_ejecucion_sentencia(_CREATE);
	realizar_create(nombre_tabla, tipo_consistencia, numero_particiones, compaction_time);

	//Limpio el nombre_tabla
	limpiar_parametro(nombre_tabla);

	//Limpio la tipo_consistencia
	limpiar_parametro(tipo_consistencia);

	//Limpio la string_numero_particiones
	limpiar_parametro(string_numero_particiones);

	//Limpio la string_compaction_time
	limpiar_parametro(string_compaction_time);

}

void consola_describe(char** comandos){
	char *nombre_tabla = NULL;
	obtener_un_parametro(comandos, &nombre_tabla, _DESCRIBE);

	loguear_comienzo_ejecucion_sentencia(_DESCRIBE);
	if(nombre_tabla==NULL){
		realizar_describe_de_todas_las_tablas();
	}else{
		loguear_un_parametros_recibido(nombre_tabla);
		tp_describe_particular_rta_a_kernel rta_describe_particular = realizar_describe_para_tabla_particular(nombre_tabla);

		if(rta_describe_particular->respuesta==REQUEST_SUCCESS){
			free(rta_describe_particular->nombre);
			free(rta_describe_particular->consistencia);
		}
		free(rta_describe_particular);
	}

	//Limpio el nombre_tabla
	limpiar_parametro(nombre_tabla);

}

void consola_drop(char** comandos){
	int cant_param_correcta = 0;
	char *nombre_tabla = NULL;
	obtener_un_parametro(comandos, &nombre_tabla, _DROP);
	cant_param_correcta=validar_parametro_consola(&nombre_tabla);
	if(cant_param_correcta<0){
		loguear_cant_menor_params(_DROP);
		return;
	}
	loguear_un_parametros_recibido(nombre_tabla);

	pthread_mutex_lock(&M_JOURNALING);
	loguear_comienzo_ejecucion_sentencia(_DROP);
	realizar_drop(nombre_tabla);
	pthread_mutex_unlock(&M_JOURNALING);

	//Limpio el nombre_tabla
	limpiar_parametro(nombre_tabla);

}

void consola_journal(int socket_LFS){

	loguear_comienzo_ejecucion_sentencia(_JOURNAL);

	//hace el journaling
	notificar_escrituras_en_memoria_LFS(socket_LFS);

}

int consola_obtener_key_comando(char * buf_comando,char** comandos, int LFS_fs)
{
	int key = -1;
	char *comando=comandos[0];

	if(comando == NULL)
		return key;

	if(string_equals_ignore_case(comando, "select")){
		consola_select(comandos);
		return 0;
	} else if(string_equals_ignore_case(comando, "insert")){
		consola_insert(buf_comando, comandos);
		return 0;
	} else if(string_equals_ignore_case(comando, "create")){
		consola_create(comandos);
		return 0;
	} else if(string_equals_ignore_case(comando, "describe")){
		consola_describe(comandos);
		return 0;
	} else if(string_equals_ignore_case(comando, "drop")){
		consola_drop(comandos);
		return 0;
	} else if(string_equals_ignore_case(comando, "journal")){
		free(comando);
		free(comandos);
		consola_journal(LFS_fs);
		return 0;
	} else {
		free(comando);
		free(comandos);
	}

	return key;
}

void limpiar_elementos_comando(int cant_parametros, char** elementos) {
	for (int i = 0; i < cant_parametros; i++) {
		free(elementos[i]);
	}
}

void obtener_un_parametro(char** comandos, char** parametro1, char * proceso){
	int j=0;

	while(comandos[j])
	{
		switch(j)
		{
			case 1:
				*parametro1 = string_duplicate(comandos[j]);
				break;
			case 2:
				loguear_cant_mayor_params(proceso);
		}

		j++;
	}

	limpiar_elementos_comando(j, comandos);

	free(comandos);
}

void loguear_un_parametros_recibido(char * param1){
	logger(escribir_loguear, l_debug,"El parámetro fue '%s'", param1);
}

void obtener_dos_parametros(char** comandos, char** parametro1, char** parametro2, char * proceso){
	int j=0;

	while(comandos[j])
	{
		switch(j)
		{
			case 1:
				*parametro1 = string_duplicate(comandos[j]);
				break;
			case 2:
				*parametro2 = string_duplicate(comandos[j]);
				break;
			case 3:
				loguear_cant_mayor_params(proceso);
		}

		j++;
	}

	limpiar_elementos_comando(j, comandos);

	free(comandos);
}

void loguear_dos_parametros_recibidos(char * param1, char * param2){
	logger(escribir_loguear, l_debug,"Los parámetros fueron '%s' y '%s'", param1, param2);
}

void loguear_tres_parametros_recibidos(char * param1, char * param2, char * param3){
	logger(escribir_loguear, l_debug,"Los parámetros fueron '%s', '%s' y '%s'", param1, param2, param3);
}

void obtener_cuatro_parametros(char** comandos, char** parametro1,
		char** parametro2, char** parametro3, char** parametro4, char * proceso){
	int j=0;

	while(comandos[j])
	{
		switch(j)
		{
			case 1:
				*parametro1 = string_duplicate(comandos[j]);
				break;
			case 2:
				*parametro2 = string_duplicate(comandos[j]);
				break;
			case 3:
				*parametro3 = string_duplicate(comandos[j]);
				break;
			case 4:
				*parametro4 = string_duplicate(comandos[j]);
				break;
			case 5:
				loguear_cant_mayor_params(proceso);
		}

		j++;
	}

	limpiar_elementos_comando(j, comandos);

	free(comandos);
}

void obtener_cuatro_parametros_insert(char * comando_puro, char** comandos, char** parametro1,
		char** parametro2, char** parametro3, char** parametro4, char * proceso){
	int j=0;

	while(comandos[j])
	{
		switch(j)
		{
			case 1:
				*parametro1 = string_duplicate(comandos[j]);
				break;
			case 2:
				*parametro2 = string_duplicate(comandos[j]);
				break;
		}

		j++;
	}
	*parametro3 = obtener_value_a_insertar(comando_puro);
	*parametro4 = string_duplicate(comandos[j-1]);

	limpiar_elementos_comando(j, comandos);

	free(comandos);
}

void loguear_cuatro_parametros_recibidos(char * param1, char * param2, char * param3, char* param4){
	logger(escribir_loguear, l_debug,"Los parámetros fueron '%s', '%s', '%s' y '%s'"
			, param1, param2, param3, param4);
}

int validar_parametro_consola(char ** parametro){
	if(*parametro!=NULL){
		return 0;
	}
	*parametro="-1";
	return -1;
}

void limpiar_parametro(char* parametro) {
	//Limpio el parametro
	if (parametro != NULL && !string_equals_ignore_case(parametro, "-1")) {
		free(parametro);
		parametro = NULL;
	}
}

int consola_derivar_comando(char * buffer, int socket_LFS){

	int comando_key;
	int res = 0;
	char** comandos = string_n_split(buffer,10," ");
	if(comandos[0]==NULL){
		logger(escribir_loguear, l_error,"No pude interpretar el comando, proba de nuevo");
		return res;
	}

	// Obtiene la clave del comando a ejecutar y manda a ejecutarlo
	comando_key = consola_obtener_key_comando(buffer, comandos, socket_LFS);

	if(comando_key<0){
		logger(escribir_loguear, l_error,"No conozco ese comando, proba de nuevo");
		return res;
	}

	return res;
}

int consola_leer_stdin(char *read_buffer, size_t max_len)
{
	char c = '\0';
	int i = 0;

	memset(read_buffer, 0, max_len);

	ssize_t read_count = 0;
	ssize_t total_read = 0;

	do{
		read_count = read(STDIN_FILENO, &c, 1);

		if (read_count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			logger(escribir_loguear,l_error,"Error en read() desde STDIN");
		  return -1;
		}
		else if (read_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		  break;
		}
		else if (read_count > 0) {
		  total_read += read_count;

		  read_buffer[i] = c;
		  i++;

		  if (total_read > max_len) {
			//log_info(logger,"Message too large and will be chopped. Please try to be shorter next time.\n");
			fflush(STDIN_FILENO);
			break;
		  }
		}

	}while ((read_count > 0) && (total_read < max_len) && (c != '\n'));

	size_t len = strlen(read_buffer);
	if (len > 0 && read_buffer[len - 1] == '\n'){
		read_buffer[len - 1] = '\0';
	}

	//log_info(logger,"Read from stdin %zu bytes. Let's prepare message to send.\n", strlen(read_buffer));

	return 0;
}

