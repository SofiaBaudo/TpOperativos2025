#ifndef INSTRUCCIONES_MEMORIA_H
#define INSTRUCCIONES_MEMORIA_H

#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <servidor_memoria.h>
#include <variables_globales_memoria.h>
#include <utils/utils.h>


//Variables y Structs Globales

typedef struct {
    int pid;
    t_list* instrucciones;
} t_proceso_instrucciones; //tipo de dato

extern t_list* lista_procesos_instrucciones; //lista global
op_code string_a_instruccion(char* token);

t_list* cargar_instrucciones_desde_archivo(char* path);
t_instruccion* obtener_instruccion(int pid, int pc);

//Declaracion de Prototipos de Funciones

void registrar_instrucciones_proceso(int pid, char* path);
void iniciar_lista_procesos_instrucciones();

//tengo q destruir el proceso? al terinar? void destruir_proceso_instrucciones(int pid);
//void destruir_instruccion(t_instruccion* instr);

#endif