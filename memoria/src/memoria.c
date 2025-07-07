//hacemos las llamadas de las funciones principales de los Modulos
#include <memoria.h>


int main(int argc, char* argv[]){

    // Iniciar modulo
    if (!inicializar_memoria()) {
        // Si falla la inicialización, limpiar y salir
        return EXIT_FAILURE;
    }

    // Servidor y lógica principal
    iniciar_servidor_memoria();

    return 0;
}