//Incluir las librerias

#include <cpu.h>

int main(int argc, char* argv[]){
    int id = atoi(argv[1]);
    inicializar_CPU(id);
    log_info(cpu_logger,"inicializacion exitosa");
    //log_destroy(cpu_logger);
    //config_destroy(cpu_config);
}
//tiene un ciclo de instruccion