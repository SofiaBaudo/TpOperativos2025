//Incluir las librerias

#include <cpu.h>

int main(int argc, char* argv[]){
    int identificador_cpu = 0; // Se recibe de argumento
    inicializar_CPU(identificador_cpu);
    log_info(cpu_logger,"inicializacion exitosa");
    log_destroy(cpu_logger);
}