//Incluir las librerias

#include <cpu.h>
#include "traduccion.h"

int main(int argc, char* argv[]){
    int id = atoi(argv[1]);  
    
    inicializar_CPU(id);
    log_info(cpu_logger,"inicializacion exitosa"); 
    
    return 0;
}