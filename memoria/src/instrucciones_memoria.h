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

void iniciar_lista_procesos_instrucciones();
t_instruccion* analizar_linea(char* linea);
t_instruccion* obtener_instruccion(int pid, int pc);
t_list* cargar_instrucciones_desde_archivo(char* PATH_INSTRUCCIONES);
void registrar_instrucciones_proceso(int pid, char* PATH_INSTRUCCIONES);
void destruir_instruccion(t_instruccion* instr);
void destruir_proceso_instrucciones(int pid);

#endif