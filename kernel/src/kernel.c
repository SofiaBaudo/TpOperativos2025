#include <kernel.h>

int main(int argc, char* argv[]) {

    inicializar_kernel();
    imprimir_configs();
    log_destroy(kernel_logger);
    log_destroy(kernel_debug_log);
    config_destroy(config_kernel);
}

