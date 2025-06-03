//Incluir las libreriasAdd commentMore actions

#include <estructura_memoria.h>
#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <variables_globales_memoria.h>

/*Estructuras: La memoria contará principalmente con 4 estructuras:*/

//Un espacio contiguo de memoria (representado por un void*). Este representará el espacio de usuario de la misma, donde los procesos podrán leer y/o escribir.
void* espacio_usuario(int pid, int pc, char* proceso_lectura_escritura) {
    if (strcmp(proceso_lectura_escritura, "WRITE") == 0) {
        // función escritura
        //Función escritura
    } else if (strcmp(proceso_lectura_escritura, "READ") == 0) {
        // función lectura
        //Función lectura
    } else {
        // log error
        //Log error
    }
}

//Las tablas de páginas, que representarán el espacio de Kernel.

void tabla_de_paginas(int ENTRADAS_POR_TABLA){
    int pagina [ENTRADAS_POR_TABLA] [2]; //FIJA 
    
    //Paginacion
}

//Un archivos SWAP denominado swapfile.bin.
//swap = manda a suspendido
//guardamos los procesos suspendidos en el archivo swapfile.bin

void SWAP(int pid){
    //SWAP de Memoria
   FILE *archivo_swap;
   archivo_swap = fopen(PATH_SWAPHILE,"ab+");
    break;
    }


/*
DESTRUCCION DE PROCESO
mensaje q se le manda al momento de destruir un proceso:AF_MCTP
Destrucción de Proceso: “## PID: <PID> - Proceso Destruido - Métricas - Acc.T.Pag: <ATP>; Inst.Sol.: <Inst.Sol.>; SWAP: <SWAP>; Mem.Prin.: <Mem.Prin.>; Lec.Mem.: <Lec.Mem.>; Esc.Mem.: <Esc.Mem.>”