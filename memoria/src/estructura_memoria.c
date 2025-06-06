#include "estructura_memoria.h"
#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <variables_globales_memoria.h>

// Declaracion de Variables Globales
int TAM_PAGINA;
int CANTIDAD_NIVELES;
int TAM_MEMORIA;
char* info_a_escribir;
char* info_a_leer;
int info_a_buscar;
int informacion;
t_metricas listado_metricas;

// Variables de Paginacion Procesos
int tabla_dir_logica[5][2];
int pagina_proceso[64][2];
int** tabla_paginas = NULL;
float tamanio_marco;

void inicializar_tabla_dir_logica() {
    tamanio_marco = (float) TAM_PAGINA / 64.0;
    tabla_paginas = malloc(CANTIDAD_NIVELES * sizeof(int*));
    for (int i = 0; i < CANTIDAD_NIVELES; i++) {
        tabla_paginas[i] = malloc(2 * sizeof(int));
    }
}

void liberar_tabla_dir_logica() {
    for (int i = 0; i < CANTIDAD_NIVELES; i++) {
        free(tabla_paginas[i]);
    }
    free(tabla_paginas);
}

bool buscar_en_pagina(int info_a_buscar) {
    for (int i = 0; i < 64; i++) {
        if (pagina_proceso[i][1] == info_a_buscar) {
            return true;
        }
    }
    return false;
}

char* funcion_escritura(int pagina, char* info_a_escribir, int direccion) {
    for (int i = 0; i < 64; i++) {
        if (pagina_proceso[i][0] == pagina && pagina_proceso[i][1] == direccion) {
            return "OK";
        }
    }
    return "No OK";
}

char* funcion_lectura(int pagina, int tamanio, int direccion) {
    static char info_leida[64];

    for (int i = 0; i < 64; i++) {
        if (pagina_proceso[i][0] == pagina && pagina_proceso[i][1] == direccion) {
            snprintf(info_leida, sizeof(info_leida), "Dato en [%d][%d]", i, 1);
            return info_leida;
        }
    }

    snprintf(info_leida, sizeof(info_leida), "0");
    return info_leida;
}

bool actualizar_pagina(int pagina_usuario, int info) {
    for (int i = 0; i < 64; i++) {
        if (pagina_proceso[i][0] == pagina_usuario && pagina_proceso[i][1] == info) {
            return true;
        }
    }
    return false;
}

/*
//Las tablas de páginas, que representarán el espacio de Kernel.

void* tabla_de_paginas(int ENTRADAS_POR_TABLA){
    //Paginacion
}
*/


/*
//leer txt
void memory_dump(){

}
*/


//Estructuras: La memoria contará principalmente con 4 estructuras:

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
            break;
    }
    return NULL;
}


/*
DESTRUCCION DE PROCESO
mensaje q se le manda al momento de destruir un proceso:AF_MCTP
Destrucción de Proceso: “## PID: <PID> - Proceso Destruido - Métricas - Acc.T.Pag: <ATP>; Inst.Sol.: <Inst.Sol.>; SWAP: <SWAP>; Mem.Prin.: <Mem.Prin.>; Lec.Mem.: <Lec.Mem.>; Esc.Mem.: <Esc.Mem.>”
*/