#include <io_conexiones.h>

struct peticion{
    int tiempoEspera;
    char descPeticion[100];
};

//el request no sabria como se pasaria en parametro pongo asi para que tenga algo

//el sistema esta bloqueado hasta que llega la peticion que lo saca de la suspension. 
void obtenerPeticion(peticion){
    printf("En proceso /n");
    sleep(peticion.tiempoEspera); //es como que procesa el tiempo que dura la peticion
}

