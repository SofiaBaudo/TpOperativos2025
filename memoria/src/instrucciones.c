#include "instrucciones.h"
#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

t_list* lista_instrucciones;

void iniciar_lista_procesos_instrucciones() {
    lista_instrucciones = list_create();
}

  
//SIRVE(CREEMOS)
/*
//Convierte una línea completa del pseudocódigo, como "READ 10 20", en una estructura t_instruccion con: el enum correspondiente (por ejemplo: READ) una lista de los parámetros como strings (["10", "20"])la cantidad de parámetros (2)

t_instruccion* analizar_linea(char* linea) {
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
*/


//Crear archivo de pseudocodigo con las instrucciones de cada proceso

void crear_archivo_pseucodigo(int pid,int tamanio){
    char* archivo_proceso = str"Archivo"
    FILE* archivo = fopen(archivo_proceso, "wr");

}

//SIRVE(CREEMOS)
/*
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
        list_add(instrucciones, analizar_linea(linea));
    }

    free(linea);
    fclose(archivo);
    return instrucciones;
}



*/
//Guarda la lista de instrucciones de un proceso en una estructura junto con su PID, y lo agrega a una lista global (procesos_instrucciones).
//se usa cuando la Memoria crea un nuevo proceso.
/*

//SIRVE(CREEMOS)

void registrar_instrucciones_proceso(int pid, char* path_archivo) {
    t_proceso_instrucciones* nuevo = malloc(sizeof(t_proceso_instrucciones));
    nuevo->pid = pid;
    nuevo->instrucciones = cargar_instrucciones_desde_archivo(path_archivo);
    list_add(procesos_instrucciones, nuevo);
}
*/

//SIRVE(CREEMOS)

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


//NO SIRVE(CREEMOS)

//Busca la instrucción número pc (program counter) del proceso con PID pid.
/*
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
}*/

//SIRVE(CREEMOS)

//Libera toda la memoria usada por una sola t_instruccion
//cuando se quiere limpiar memoria, cuando finaliza un proceso
/*
void destruir_instruccion(t_instruccion* instr) {
    for (int i = 0; i < instr->cantidad_parametros; i++) {
        free(instr->parametros[i]);
    }
    free(instr->parametros);
    free(instr);
}
*/

//SIRVE(CREEMOS)

//Busca el proceso con ese pid en la lista global, libera cada instrucción de su lista, libera la estructura del proceso, lo saca de la lista global procesos_instrucciones
//Se usa al destruir un proceso (cuando pasa a EXIT creo)
/*
void destruir_proceso_instrucciones(int pid) {
    for (int i = 0; i < list_size(procesos_instrucciones); i++) {
        t_proceso_instrucciones* p = list_get(procesos_instrucciones, i);
        if (p->pid == pid) {
           // list_destroy_and_destroy_elements(p->instrucciones, (void*)destruir_instruccion);
            list_remove(procesos_instrucciones, i);
            free(p);
            return;
        }
    }
}
*/