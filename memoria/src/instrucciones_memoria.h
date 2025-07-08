#ifndef INSTRUCCIONES_MEMORIA_H
#define INSTRUCCIONES_MEMORIA_H

#include "bibliotecas.h"
#include "inicializar_memoria.h"
#include "procesos.h"


// Genera una lista de instrucciones (cada elemento es un char) leyendo el archivo indicado
// El caller debe liberar la lista y cada string con list_destroy_and_destroy_elements(..., free)
t_list* generar_instrucciones_proceso(int pid, const char* path_pseudocodigo);

// Devuelve una copia de la instrucción (char) para el proceso y PC dado, o NULL si no existe
char* obtener_instruccion_proceso(int pid, int pc);

#endif