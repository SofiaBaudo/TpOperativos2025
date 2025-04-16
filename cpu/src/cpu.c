//Incluir las librerias

#include <cpu.h>


int main(int argc, char* argv[]){
    inicializar_CPU();
    log_info(cpu_logger,"inicializacion exitosa");
    log_destroy(cpu_logger);
}




