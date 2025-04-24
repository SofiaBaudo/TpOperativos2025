#ifndef INSTRUCCIONES
#define INSTRUCCIONES

#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inicializar_memoria.h"

typedef struct {
    int pid;
    t_list* instrucciones;
} t_proceso;


void iniciar_lista_procesos();
void cargar_instrucciones_proceso(int pid, char* path);
char* obtener_instruccion(int pid, int pc);

#endif