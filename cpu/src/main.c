#include <main.h>

//#define PUERTO 5555

int main(int argc, char* argv[]) {
t_log *logger;
printf("hola");
logger = iniciar_logger();
log_info(logger, "Estoy en CPU");
    saludar("cpu");
    return 0;
}

int inicializarCPU(){
    //aca se tiene que llamar todas funciones que inicialicen la cpu
    //struct addrinfo hints, *cpu_info;

}

