#include <main.h>

//#define PUERTO 5555

int main(int argc, char* argv[]) {
    printf("hola\n");
    t_log *logger;
    logger = iniciar_logger("archivoCPULog.log","LOGGER_CPU");
    log_info(logger, "Estoy en CPU\n");
    destruir_logger(logger);

    printf("logger destruido\n");
    return 0;
}

int inicializarCPU(){
    //aca se tiene que llamar todas funciones que inicialicen la cpu
    //struct addrinfo hints, *cpu_info;
    return 0;
}

