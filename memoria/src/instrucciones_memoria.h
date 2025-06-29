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

typedef struct infoPid{
    int pid;
    char* PATH_INSTRUCCIONES;
    t_list *sublista;
} infoPid;

typedef struct pc_con_instruccion{
    int pc;
    char* instruccion_pc;
}pc_con_instruccion;

void iniciar_lista_pids();
char* obtener_instruccion(int pid, int pc);
infoPid *buscar_info_pid(int pid);
void enlazar_pid_nombre_archivo (int pid, char* PATH_INSTRUCCIONES);
void cargar_instrucciones_desde_archivo(int pid, char* PATH_INSTRUCCIONES);

extern t_list *Lista_de_pids;   

#endif