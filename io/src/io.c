#include <utils/utils.h>
#include <io.h>

int main(int argc, char* argv[]) {

    inicializar_IO();
    printf("\n");
    log_info(io_logger,"Estoy aca en IO");
    return 0;
}