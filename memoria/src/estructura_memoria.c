#include <estructura_memoria.h>
#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <variables_globales_memoria.h>

//Declaracion de Variables Globales

int pagina [1] [2];

char* info_a_escribir;
char* info_a_leer;

t_metricas listado_metricas;

/*Estructuras: La memoria contará principalmente con 4 estructuras:*/
/*
//Un espacio contiguo de memoria (representado por un void*). Este representará el espacio de usuario de la misma, donde los procesos podrán leer y/o escribir.
void* espacio_usuario(int pid, int pc, char* proceso_lectura_escritura){
    if (strcmp(proceso_lectura_escritura, "WRITE") == 0) {
        funcion_escritura(info_a_escribir);
    } 
    else if (strcmp(proceso_lectura_escritura, "READ") == 0) {
        funcion_lectura(info_a_leer);
    } 
    else {
        //Log error
    }
    return NULL; //cuando quede implementada hay que chequear que devolver
}

*/
/*
//Función escritura en Memoria de usuario (viene de cpu)

void* funcion_escritura(char* info_a_escribir){
 
}

//Función lectura en Memoria de Usuario (viene de cpu)
void* funcion_lectura(char* info_a_leer){
    
}

//Las tablas de páginas, que representarán el espacio de Kernel.

void* tabla_de_paginas(int ENTRADAS_POR_TABLA){
    //Paginacion
}

*/

//Un archivos SWAP denominado swapfile.bin.
//swap = manda a suspendido
//guardamos los procesos suspendidos en el archivo swapfile.bin

/*
void* SWAP(int pid){
   //SWAP de Memoria(Hay que Paginas Antes de Hacer SWAP)
   FILE *archivo_swap;
   archivo_swap = fopen(PATH_SWAPHILE,"ab+");
}
*/

//Listado de métricas por proceso.
/*
Cantidad de accesos a Tablas de Páginas
Cantidad de Instrucciones solicitadas
Cantidad de bajadas a SWAP
Cantidad de subidas a Memoria Principal o al espacio contiguo de memoria
Cantidad de Lecturas de memoriaMore actions
Cantidad de Escrituras de memoria
*/

/*
void* metricas_proceso(int pid,int pagina){
    switch (listado_metricas.pid)
    {
    case listado_metricas.cant_acceso_tabla_pagina:
        listado_metricas.cant_acceso_tabla_pagina++;
    break;
    case listado_metricas.instrucciones_solicitadas:
        listado_metricas.instrucciones_solicitadas++;
    break;
    case listado_metricas.bajadas_swap:
        listado_metricas.bajadas_swap++;
    break;
    case listado_metricas.cant_subidas_memoria_principal:
        listado_metricas.cant_subidas_memoria_principal++;
    break;
    case listado_metricas.cant_lecturas_memoria:
        listado_metricas.cant_lecturas_memoria++;
    break;
    case listado_metricas.cant_escrituras_memoria:
        listado_metricas.cant_escrituras_memoria++;
    break;
    default:
    //log_error 
    break;
    }
}  
*/

void* metricas_proceso(int pid, tipo_metrica metrica) {
    switch (metrica) {
        case ACCESO_TABLA:
            listado_metricas.cant_acceso_tabla_pagina++;
            break;
        case INSTRUCCIONES_SOLICITADAS:
            listado_metricas.instrucciones_solicitadas++;
            break;
        case BAJADAS_SWAP:
            listado_metricas.bajadas_swap++;
            break;
        case SUBIDAS_MEMORIA:
            listado_metricas.cant_subidas_memoria_principal++;
            break;
        case LECTURAS_MEMORIA:
            listado_metricas.cant_lecturas_memoria++;
            break;
        case ESCRITURAS_MEMORIA:
            listado_metricas.cant_escrituras_memoria++;
            break;
        default:
            // log_error
            break;
    }
    return NULL;
}

/*
DESTRUCCION DE PROCESO
mensaje q se le manda al momento de destruir un proceso:AF_MCTP
Destrucción de Proceso: “## PID: <PID> - Proceso Destruido - Métricas - Acc.T.Pag: <ATP>; Inst.Sol.: <Inst.Sol.>; SWAP: <SWAP>; Mem.Prin.: <Mem.Prin.>; Lec.Mem.: <Lec.Mem.>; Esc.Mem.: <Esc.Mem.>”
*/