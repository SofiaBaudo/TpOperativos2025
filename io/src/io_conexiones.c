#include <io_conexiones.h>

int tiempo_de_suspension = 0;

//el request no sabria como se pasaria en parametro pongo asi para que tenga algo

void esperar_peticion(){
    while (1){
        log_info(io_logger,"Entre al while y estoy esperando");
        t_paquete* paquete = recibir_paquete(fd_kernel);
        log_info(io_logger,"RECIBI EL PAQUETE");
        op_code operacion = obtener_codigo_de_operacion(paquete);
        int pid = deserializar_pid_memoria(paquete);
        int tiempo_de_suspension = deserializar_milisegundos(paquete);
        log_info(io_logger, "recibi la operacion: %i", operacion); //tendria que ser 16
        if(operacion == EJECUTAR_RAFAGA_IO){
            if(tiempo_de_suspension>0){
                log_info(io_logger,"## PID: <%i> - Inicio de IO - Tiempo: <%i>", pid,tiempo_de_suspension);
                ejecutarPeticion(tiempo_de_suspension);
                tiempo_de_suspension  = 0;
                log_info(io_logger,"## PID: <%i> - Fin de IO",pid);
            }
       }
       operacion = -1; //valor para que deje de estar en "RAFAGA_ACEPTADA"
    }
}

//el sistema esta bloqueado hasta que llega la peticion que lo saca de la suspension. 
void ejecutarPeticion(int tiempo){
    printf("En proceso y recibi: %i ", tiempo);
    printf("\n");
    usleep(tiempo*1000); //es como que procesa el tiempo que dura la peticion
    enviar_op_code(fd_kernel,FIN_DE_IO);    
    //avisar_que_finalice();
}
