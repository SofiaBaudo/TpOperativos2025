#include <utils/utils.h>
#include <io.h>



int main(int argc, char* argv[]) {


   char *nombre = argv[1]; // esto devuelve el nombre que se esta pasando en el json
   inicializar_IO(nombre);


    printf("\n");
    log_info(io_logger,"Estoy aca en IO");
    printf("\n");
    return 0;
}



