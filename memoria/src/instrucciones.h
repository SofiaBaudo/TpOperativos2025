#ifndef INSTRUCCIONES
#define INSTRUCCIONES


#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/utils.h"
#include "servidor_memoria.h"
#include "inicializar_memoria.h"

typedef struct {
    op_code codigo;
    char** parametros;
    int cantidad_parametros;
} t_instruccion;

typedef struct {
    int pid;
    t_list* instrucciones;
} t_proceso_instrucciones;

t_list* cargar_instrucciones_desde_archivo(char* path);
t_instruccion* obtener_instruccion(int pid, int pc);
void registrar_instrucciones_proceso(int pid, char* path);
void iniciar_lista_procesos_instrucciones();

//tengo q destruir el proceso? al terinar? void destruir_proceso_instrucciones(int pid);
//void destruir_instruccion(t_instruccion* instr);


#endif
