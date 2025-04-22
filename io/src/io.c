#include <utils/utils.h>
#include <io.h>

struct NodoColasPeticiones{ //creo cola de peticiones donde se van a ir ejecutando 
    int info;
    Nodo *sgte;
}

int main(int argc, char* argv[]) {

    NodoColasPeticiones *colaFin; 
    NodoColasPeticiones *colaInicio;

    inicializar_IO();
    printf("\n");
    log_info(io_logger,"Estoy aca en IO");
    return 0;
}

bool colaVacia{
    if(colaInicio->sgte == NULL){
        return true;
    }
    return false;
}
void agregarPeticion{ //esto me parece que tendria que ir en kernel porque el es el que hace las peticiones, y que io simplemente vaya sacando de la cola y ejecute.
    if(colaVacia){
        //agregar al principio de la cola
    }
    else{
        //agregar al final de la cola
    }
}


