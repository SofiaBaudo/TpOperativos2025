#include "instrucciones.h"
#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "instrucciones.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

t_list* procesos_instrucciones;

void iniciar_lista_procesos_instrucciones() {
    procesos_instrucciones = list_create();
}

//Convierte el nombre de la instrucción (un string) en el valor correspondiente del enum instruccion.
instruccion string_a_instruccion(char* str) {
    if (strcmp(str, "NOOP") == 0) return NOOP;
    if (strcmp(str, "WRITE") == 0) return WRITE;
    if (strcmp(str, "READ") == 0) return READ;
    if (strcmp(str, "GOTO") == 0) return GOTO;
    if (strcmp(str, "IO") == 0) return IO;
    if (strcmp(str, "INIT_PROC") == 0) return INIT_PROC;
    if (strcmp(str, "DUMP_MEMORY") == 0) return DUMP_MEMORY;
    if (strcmp(str, "EXIT") == 0) return EXIT;
    return -1;
}

//Convierte una línea completa del pseudocódigo, como "READ 10 20", en una estructura t_instruccion con: el enum correspondiente (por ejemplo: READ) una lista de los parámetros como strings (["10", "20"])la cantidad de parámetros (2)

t_instruccion* parsear_linea(char* linea) {
    t_instruccion* instr = malloc(sizeof(t_instruccion));
    char* linea_copia = strdup(linea);

    char* token = strtok(linea_copia, " ");
    instr->codigo = string_a_instruccion(token);

    instr->parametros = malloc(sizeof(char*) * 10);
    instr->cantidad_parametros = 0;

    while ((token = strtok(NULL, " ")) != NULL) {
        instr->parametros[instr->cantidad_parametros++] = strdup(token);
    }

    free(linea_copia);
    return instr;
}

//Lee línea por línea un archivo de pseudocódigo (ej: "proceso1") y convierte cada línea en una t_instruccion, agregándolas a una lista.
//Devuelve un t_list* con todas las instrucciones del proceso, ya parseadas.
t_list* cargar_instrucciones_desde_archivo(char* path_archivo) {
    FILE* archivo = fopen(path_archivo, "r");
    if (!archivo) {
        perror("No se pudo abrir el archivo de pseudocódigo");
        return NULL;
    }

    t_list* instrucciones = list_create();
    char* linea = NULL;
    size_t len = 0;

    while (getline(&linea, &len, archivo) != -1) {
        linea[strcspn(linea, "\n")] = 0;
        list_add(instrucciones, parsear_linea(linea));
    }

    free(linea);
    fclose(archivo);
    return instrucciones;
}

//Guarda la lista de instrucciones de un proceso en una estructura junto con su PID, y lo agrega a una lista global (procesos_instrucciones).
//se usa cuando la Memoria crea un nuevo proceso.

void registrar_instrucciones_proceso(int pid, char* path_archivo) {
    t_proceso_instrucciones* nuevo = malloc(sizeof(t_proceso_instrucciones));
    nuevo->pid = pid;
    nuevo->instrucciones = cargar_instrucciones_desde_archivo(path_archivo);
    list_add(procesos_instrucciones, nuevo);
}

//Busca la instrucción número pc (program counter) del proceso con PID pid.
//Devuelve el puntero a la instrucción que necesita la CPU ejecutar.
/*
t_instruccion* obtener_instruccion(int pid, int pc) {
    for (int i = 0; i < list_size(procesos_instrucciones); i++) {
        t_proceso_instrucciones* p = list_get(procesos_instrucciones, i);
        if (p->pid == pid) {
            return list_get(p->instrucciones, pc);
        }
    }
    return NULL;
}*/

t_instruccion* obtener_instruccion(int pid, int pc) {
//Recorre la lista global procesos_instrucciones.
    for (int i = 0; i < list_size(procesos_instrucciones); i++) {
        t_proceso_instrucciones* p = list_get(procesos_instrucciones, i);
//Busca el proceso con el pid correcto        
        if (p->pid == pid) {
        //Obtiene la instrucción número pc
            t_instruccion* instr = list_get(p->instrucciones, pc);

            // Construir string con parámetros: cadena con los parámetros (para log)
            char parametros[256] = {0};
            for (int j = 0; j < instr->cantidad_parametros; j++) {
                strcat(parametros, instr->parametros[j]);
                if (j < instr->cantidad_parametros - 1)
                    strcat(parametros, " ");
            }

            // Obtiene nombre del enum (instrucción)
            const char* nombres[] = { "NOOP", "WRITE", "READ", "GOTO", "IO", "INIT_PROC", "DUMP_MEMORY", "EXIT" };
            //Loguear la instrucción obtenida
            log_info(logger_memoria, "## PID: %d - Obtener instrucción: %d - Instrucción: %s %s", //LOG OBLIGATORIO
                     pid, pc, nombres[instr->codigo], parametros);

            return instr;
        }
    }
    return NULL;
}

//Libera toda la memoria usada por una sola t_instruccion
//cuando se quiere limpiar memoria, cuando finaliza un proceso

void destruir_instruccion(t_instruccion* instr) {
    for (int i = 0; i < instr->cantidad_parametros; i++) {
        free(instr->parametros[i]);
    }
    free(instr->parametros);
    free(instr);
}

//Busca el proceso con ese pid en la lista global, libera cada instrucción de su lista, libera la estructura del proceso, lo saca de la lista global procesos_instrucciones
//Se usa al destruir un proceso (cuando pasa a EXIT creo)

void destruir_proceso_instrucciones(int pid) {
    for (int i = 0; i < list_size(procesos_instrucciones); i++) {
        t_proceso_instrucciones* p = list_get(procesos_instrucciones, i);
        if (p->pid == pid) {
            list_destroy_and_destroy_elements(p->instrucciones, (void*)destruir_instruccion);
            list_remove(procesos_instrucciones, i);
            free(p);
            return;
        }
    }
}
