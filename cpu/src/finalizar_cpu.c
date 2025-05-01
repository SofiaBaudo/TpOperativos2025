#include <finalizar_cpu.h>

void terminarPrograma(){
    log_destroy(cpu_logger);
    log_destroy(cpu_log_debug);
    config_destroy(cpu_config);
}