//hacemos las llamadas de las funciones principales de los Modulos
#include <memoria.h>

int main(int argc, char* argv[]){
    leer_config();
    iniciar_logger_memoria();
    iniciar_servidor_memoria();              
    //tabla_de_paginas_raiz = crear_tabla(0, memoria_config.CANTIDAD_NIVELES, memoria_config.ENTRADAS_POR_TABLA);

    return 0;
}
