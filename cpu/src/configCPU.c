#include <configCPU.h>

//iniciar config
t_config *crear_config(void){
    t_config *nuevo_config_cpu = config_create("/configCPU/cpu.config");
    if(!nuevo_config_cpu){
        perror("No se pudo crear el config");
        exit(EXIT_FAILURE);
    }
}

void destruir_config(t_config *config){
config_destroy(config);
}