//Incluir las librerias

#include <estructura_memoria.h>
#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <variables_globales_memoria.h>

//Declaracion de Variables Globales

int pagina [2] [cantidad_de_marcos]; //FIJA 
int niveles_de_pagina = CANTIDAD_NIVELES; //Niveles de Paginacion
int tamanio_pagina = TAM_PAGINA;
int tamanio_total = TAM_MEMORIA;
int tamanio_memoria_usuario = 0;
int tamanio_memoria_paginas = 0;
int tamanio_marcos = 1;
int cantidad_de_paginas = 0;
int cantidad_de_marcos = 0;
cantidad_de_marcos = tamanio_pagina/tamanio_marcos;
int tamanio_ocupado = 0;
int tamanio_disponible = 0;
int memoria_paginada = 0;
char* proceso_lectura_escritura;

//Hay que Hacer lista de Paginas y sus niveles de Paginacion

//Estructuras: La memoria contará principalmente con 4 estructuras:

//Un espacio contiguo de memoria (representado por un void*). Este representará el espacio de usuario de la misma, donde los procesos podrán leer y/o escribir.
void* espacio_usuario(int pid, int pc, char* proceso_lectura_escritura){
    if (strcmp(proceso_lectura_escritura, "WRITE") == 0) {
        //Función escritura
        funcion_escritura_usuario(info_escribir);
    } else if (strcmp(proceso_lectura_escritura, "READ") == 0) {
        //Función lectura
        funcion_lectura_usuario(info_leer);
    } else {
        //Log error
         log_error(logger_memoria, "Error al mandar la Solicitud de Escritura o Lectura");
    }
}

//Función lectura
void funcion_lectura_usuario(char* info_leer){
    //Buscar en Pagina la Informacion Buscada
}

//Función lectura
void funcion_escritura_usuario(char* info_escribir){
    //Buscar en Pagina Proximo Espacio Disponible
}

//Las tablas de páginas, que representarán el espacio de Kernel.

void tablas_de_paginas(){
    //Paginar
    tamanio_disponible = tamanio total - tamanio ocupado;
    //Llega Informacion desde Kernel(Proceso)
    if(tamanio_total == tamanio_disponible)
    {
        NO_HAY_ESPACIO_EN_MEMORIA;
        //(Hacer SWAP Procesos o lo que se necesite en el Momento)(Depende la Situacion)
    }
    else{
        HAY_ESPACIO_EN_MEMORIA;
        //(Meter en Memoria el Proceso o lo que se pida en el momento)
        //Recibo de Kernel el Proceso a guardar
        escribir_proceso_memoria();
    }
}

void escribir_proceso_memoria(int proceso recibido){
    proceso_paginado[];
    busqueda_memoria_disponible();
 
}

void busqueda_memoria_disponible(){

}

//Un archivos SWAP denominado swapfile.bin.
//swap = manda a suspendido
//guardamos los procesos suspendidos en el archivo swapfile.bin

void SWAP(int pid){
    //SWAP de Memoria(Hay que Paginas Antes de Hacer SWAP)
   FILE *archivo_swap;
   archivo_swap = fopen(PATH_SWAPHILE,"ab+");

}

//Listado de métricas por proceso.
/*
Cantidad de accesos a Tablas de Páginas
Cantidad de Instrucciones solicitadas
Cantidad de bajadas a SWAP
Cantidad de subidas a Memoria Principal o al espacio contiguo de memoria
Cantidad de Lecturas de memoria
Cantidad de Escrituras de memoria
*/

void metricas_proceso(int pid,int pagina){
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
    case listado_metricas.cant_acceso_tabla_pagina:
        listado_metricas.cant_acceso_tabla_pagina++;
    break;
    default:
        //log_error 
    break;
    }
}

/*
DESTRUCCION DE PROCESO
mensaje q se le manda al momento de destruir un proceso:AF_MCTP
Destrucción de Proceso: “## PID: <PID> - Proceso Destruido - Métricas - Acc.T.Pag: <ATP>; Inst.Sol.: <Inst.Sol.>; SWAP: <SWAP>; Mem.Prin.: <Mem.Prin.>; Lec.Mem.: <Lec.Mem.>; Esc.Mem.: <Esc.Mem.>”
*/