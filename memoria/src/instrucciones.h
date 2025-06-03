#ifndef INSTRUCCIONES
#define INSTRUCCIONES

#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include "utils/utils.h"
#include "servidor_memoria.h"
#include "inicializar_memoria.h"
#include "variables_globales_memoria.h"

typedef struct t_instruccion {
    op_code codigo;
    char** parametros;
    int cantidad_parametros;
} t_instruccion;

typedef struct {
    int pid;
    t_list* instrucciones;
} t_proceso_instrucciones; //tipo de dato

extern t_list* lista_procesos_instrucciones; //lista global

op_code string_a_instruccion(char* token);


t_list* cargar_instrucciones_desde_archivo(char* path);
t_instruccion* obtener_instruccion(int pid, int pc);
void registrar_instrucciones_proceso(int pid, char* path);
void iniciar_lista_procesos_instrucciones();

//tengo q destruir el proceso? al terinar? void destruir_proceso_instrucciones(int pid);
//void destruir_instruccion(t_instruccion* instr);

#endif