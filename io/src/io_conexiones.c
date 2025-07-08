#include <io_conexiones.h>

int tiempo_de_suspension = 0;

//el request no sabria como se pasaria en parametro pongo asi para que tenga algo

void esperar_peticion(){
    while (1){
        printf("Entre al while y estoy esperando \n");
        op_code operacion = recibir_op_code(fd_kernel);
        printf("\n");
        if(operacion == EJECUTAR_RAFAGA_IO){
        log_debug(io_debug_log,"estoy en el if dentro de operacion = ejecutar_rafaga");
        enviar_op_code(fd_kernel,RAFAGA_ACEPTADA); //falta implementar que no se este ejecutando ninguna rafaga
        tiempo_de_suspension = recibir_entero(fd_kernel);
        if(tiempo_de_suspension>0){
        log_debug(io_debug_log,"estoy en el if de tiempo de suspension");
        
        ejecutarPeticion(tiempo_de_suspension);
        tiempo_de_suspension  = 0;
        }
       }
       operacion = -1; //valor para que deje de estar en "RAFAGA_ACEPTADA"
    }
}

//el sistema esta bloqueado hasta que llega la peticion que lo saca de la suspension. 
void ejecutarPeticion(int tiempo){
    printf("En proceso y recibi: %i ", tiempo);
    printf("\n");
    usleep(tiempo*1000000); //es como que procesa el tiempo que dura la peticion
    //avisar_que_finalice();
}
