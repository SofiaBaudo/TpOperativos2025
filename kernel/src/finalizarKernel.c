#include <finalizarKernel.h>

void terminarPrograma(){
     log_destroy(kernel_logger);
    log_destroy(kernel_debug_log);
    config_destroy(config_kernel);
}