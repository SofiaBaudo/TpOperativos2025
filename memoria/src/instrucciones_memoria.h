//Define de Instrucciones Memoria
#ifndef INSTRUCCIONES_MEMORIA_H
#define INSTRUCCIONES_MEMORIA_H

#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <servidor_memoria.h>
#include <variables_globales_memoria.h>
#include <utils/utils.h>


typedef struct lista_de_pids{
    struct infoPid pid;
    struct lista_de_pids *siguiente;
} lista_de_pids;

typedef struct {
    int pid;
    char* path_pseudo;
    struct lista_de_pcs *sublista;
} infoPid;

typedef struct {
    struct pc_con_instruccion pc;
    struct lista_de_pcs *siguiente;
}lista_de_pcs;

typedef struct {
    int pc;
    char* instruccion_pc;
}pc_con_instruccion;

void iniciar_lista_procesos_instrucciones();
t_instruccion* analizar_linea(char* linea);
t_instruccion* obtener_instruccion(int pid, int pc);
t_list* cargar_instrucciones_desde_archivo(char* PATH_INSTRUCCIONES);
void registrar_instrucciones_proceso(int pid, char* PATH_INSTRUCCIONES);
void destruir_instruccion(t_instruccion* instr);
void destruir_proceso_instrucciones(int pid);

#endif