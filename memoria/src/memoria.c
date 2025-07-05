//hacemos las llamadas de las funciones principales de los Modulos
#include <memoria.h>

int main(int argc, char* argv[]){
    leer_config();
    iniciar_logger_memoria();
    
    //iniciar_servidor_memoria();   
    iniciar_lista_pids(); // crea Lista_de_pids como t_list*
   
    int pid = 1;
 
    enlazar_pid_nombre_archivo(pid, "/home/utnso/tp-2025-1c-Sinergia-SO-13/memoria/src/Pseudocodigo/pseudocodigo.txt");
    log_debug(logger_memoria, "hola");

    for (int pc = 1; pc <= 4; pc++) {
        char* instruccion = obtener_instruccion(pid, pc);
        printf("Instrucción PID %d - PC %d: %s\n", pid, pc, instruccion);
    }

    //tabla_de_paginas_raiz = crear_tabla(0, memoria_config.CANTIDAD_NIVELES, memoria_config.ENTRADAS_POR_TABLA);

    return 0;
}
