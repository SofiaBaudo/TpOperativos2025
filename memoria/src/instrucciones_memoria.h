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
    struct lista_de_pcs *sublista;
} infoPid;

typedef struct lista_de_pids{
    infoPid info_pid;
    struct lista_de_pids *siguiente;
} lista_de_pids;

typedef struct pc_con_instruccion{
    int pc;
    char* instruccion_pc;
}pc_con_instruccion;

typedef struct lista_de_pcs{
    pc_con_instruccion info_pc;
    struct lista_de_pcs *siguiente;
}lista_de_pcs;

void iniciar_lista_pids();
char* obtener_instruccion(int pid, int pc);
lista_de_pcs *buscar_lista_pid(int pid);
void enlazar_pid_nombre_archivo (int pid, char* PATH_INSTRUCCIONES);
void cargar_instrucciones_desde_archivo(int pid, char* PATH_INSTRUCCIONES);


extern lista_de_pcs *Lista_de_pcs;
extern lista_de_pids *Lista_de_pids;   

#endif