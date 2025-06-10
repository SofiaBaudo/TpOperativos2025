#include <stdlib.h>
#include <stdio.h>
#include <memoria.h>
//#include <inicializar_memoria.h>

//Aca hacemos las llamadas de las funciones principales de los mod

int main(int argc, char* argv[]){
    leer_config();
    iniciar_logger_memoria();
    iniciar_servidor_memoria();              

    return 0;
}
