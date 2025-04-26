#include <io_conexiones.h>


/*struct peticion{
   int tiempoEspera;
   char descPeticion[100];
};*/


int tiempo_de_suspension = 0;
//el request no sabria como se pasaria en parametro pongo asi para que tenga algo


void esperar_peticion(){
   while (1){
       printf("Entre al while y estoy esperando \n");
       tiempo_de_suspension = recibir_entero(fd_kernel);
       printf("\n");
      if(tiempo_de_suspension>0){
       ejecutarPeticion(tiempo_de_suspension) ;
       tiempo_de_suspension  = 0;
      }
     
   }
}
//el sistema esta bloqueado hasta que llega la peticion que lo saca de la suspension.
void ejecutarPeticion(int tiempo){
   printf("En proceso y recibi: %d ", tiempo);
   printf("\n");
   usleep(tiempo*1000); //es como que procesa el tiempo que dura la peticion
   //avisar_que_finalice()
}

