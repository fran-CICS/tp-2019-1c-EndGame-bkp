/*
 * TestGeneral.c
 *
 *  Created on: 17 may. 2019
 *      Author: utnso
 */

#include "TestGeneral.h"

int correr_tests(){
	CU_initialize_registry();

	CU_pSuite tests_gestion_seg_pag =
			CU_add_suite("Suite para probar gestión de la seg paginada", setup_gestion_seg_pag, clean_gestion_seg_pag);
	CU_add_test(tests_gestion_seg_pag, "test_calcular_cantidad_marcos_MP", test_calcular_cantidad_marcos_MP);
	CU_add_test(tests_gestion_seg_pag, "test_colocar_key_en_MP", test_colocar_key_en_MP);
	CU_add_test(tests_gestion_seg_pag, "test_modificar_key_en_MP", test_modificar_key_en_MP);

	CU_pSuite tests_comandos =
				CU_add_suite("Suite para probar interpretación de comandos", setup_comandos, clean_comandos);
	CU_add_test(tests_comandos, "test_tomar_value_insert", test_tomar_value_insert);
	CU_add_test(tests_comandos, "test_tomar_value_insert_una_comilla", test_tomar_value_insert_una_comilla);
	CU_add_test(tests_comandos, "test_tomar_value_insert_sin_comillas", test_tomar_value_insert_sin_comillas);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}

int setup_gestion_seg_pag(){
	TAMANIO_MEMORIA=256;
	TAMANIO_VALUE=3;
	MEMORIA_PRINCIPAL = reservar_total_memoria();
	g_logger= log_create("/home/utnso/TestsMemoria.log", "TestsMemoria", true, LOG_LEVEL_DEBUG);

	inicializar_bitmap_marcos();
	inicializar_tabla_segmentos();

	return 0;
}

int clean_gestion_seg_pag(){
	free(MEMORIA_PRINCIPAL);
	log_destroy(g_logger);

	liberar_bitmap_marcos();
	liberar_tabla_segmentos();

	return 0;
}

void test_calcular_cantidad_marcos_MP(){
	setup_gestion_seg_pag();
	int tamanio_marco = obtener_tamanio_marco();
	CU_ASSERT_EQUAL(obtener_cantidad_marcos_en_MP(tamanio_marco), 23);
}

void test_colocar_key_en_MP(){

}

void test_modificar_key_en_MP(){

}

int setup_comandos(){
	return 0;
}

int clean_comandos(){
	return 0;
}

void test_tomar_value_insert(){
	setup_comandos();
	CU_ASSERT_STRING_EQUAL(obtener_value_a_insertar("INSERT TABLA1 3 \"Mi nombre es Lissandra\" 1548421507")
			,"Mi nombre es Lissandra");
}

void test_tomar_value_insert_una_comilla(){
	setup_comandos();
	CU_ASSERT_PTR_NULL(obtener_value_a_insertar("INSERT TABLA1 3 \"Mi nombre"));
}

void test_tomar_value_insert_sin_comillas(){
	setup_comandos();
	CU_ASSERT_PTR_NULL(obtener_value_a_insertar("INSERT TABLA1 3"));
}
