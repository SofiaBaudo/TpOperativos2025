#include <main.h>

int main(int argc, char* argv[]) {

    inicializar_kernel();
    
    log_destroy(kernel_logger);
    log_destroy(kernel_debug_log);
    config_destroy(config_kernel);
}

