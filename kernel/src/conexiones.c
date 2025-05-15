
#include <conexiones.h>

//sacamos los close porque se recibe mas de un io y mas de una cpu

t_list *ios_conectados = NULL; // variable global
t_list *cpus_conectadas = NULL;

void atender_kernel_dispatch(){
   int servidor_kernel = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH,kernel_logger,"El kernel se conecto y esta esperando al dispatch");
   if (servidor_kernel == -1) {
       log_error(kernel_logger, "Error al iniciar el servidor de kernel");
       exit(EXIT_FAILURE);
   }
   log_debug(kernel_logger, "Servidor de kernel para dispatch iniciado en el puerto %s", PUERTO_ESCUCHA_DISPATCH);// Log de inicio del servidor
   while (1) { // sigue ciclando mientras no se desconecte el cliente
       // Aca esperan al cliente (CPU)
       int cliente_dispatch = esperar_cliente(servidor_kernel,kernel_logger,"CPU_DISPATCH");
       if (cliente_dispatch == -1) {
               log_error(kernel_logger, "Error al aceptar un cliente");
               continue;
           }
       int *puntero_al_dispatch = malloc(sizeof(*puntero_al_dispatch));
       memcpy(puntero_al_dispatch, &cliente_dispatch, sizeof(int));
       //*puntero_al_dispatch = cliente_dispatch;
       pthread_t hilo_kernel_dispatch;
       pthread_create(&hilo_kernel_dispatch,NULL,manejar_kernel_dispatch,(void*)puntero_al_dispatch); //Creamos el hilo
       pthread_detach(hilo_kernel_dispatch);//El hilo se desacopla del hilo principal.
   }
}
void* manejar_kernel_dispatch(void *socket_dispatch){
   int dispatch = *((int *)socket_dispatch); // Desreferencio el puntero para obtener el socket del cliente
   free(socket_dispatch);
   op_code dispatch_id = recibir_op_code(dispatch); // !!!!!!!!!!DESPUES VER DE UNIFICAR LA FUNCION Q HIZO JERE EN EL UTILS DE RECIBIR CON UN CODE_OP PERO QUE SEA OP_CODE!!!!!!!!!!!!!!!!
   log_info(kernel_logger,"el dispatch es %d" ,dispatch);
   switch (dispatch_id)
   {
       case HANDSHAKE_CPU_DISPATCH:
           //LOG_INFO : ES EL LOG OBLIGATORIO
           log_info(kernel_logger, "## CPU-DISPATCH Conectado - FD del socket: %d", dispatch);
           enviar_op_code(dispatch, HANDSHAKE_ACCEPTED);
           int *id = malloc(sizeof(int));
           recv(dispatch,id,sizeof(int),0);
           log_debug(kernel_debug_log,"Se conecto la cpu con id %i",*id);
           list_add(cpus_conectadas,id);
           log_debug(kernel_debug_log,"Se agrego a la lista la cpu con id: %i ", *id); //%i espera un entero
           //acá tendriamos que esperar otro opcode que puede ser una syscall o alguna otra cosa
           break;
           //case HANDSHAKE_CPU_INTERRUPT:
          
       default:
           log_warning(kernel_logger, "No se pudo identificar al cliente; op_code: %d", dispatch); //AVISA Q FUCNIONA MAL
           break;
   }
   return NULL;
}

void atender_kernel_interrupt(){
   int servidor_kernel_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT,kernel_logger,"El kernel se conecto y esta esperando al interrupt");
   if (servidor_kernel_interrupt == -1) {
       log_error(kernel_logger, "Error al iniciar el servidor de kernel");
       exit(EXIT_FAILURE);
   }
   log_debug(kernel_logger, "Servidor de kernel para interrupt iniciado en el puerto %s", PUERTO_ESCUCHA_INTERRUPT);// Log de inicio del servidor
   while (1) { // sigue ciclando mientras no se desconecte el cliente
       // Aca esperan al cliente (CPU)
       int cliente_interrupt = esperar_cliente(servidor_kernel_interrupt,kernel_logger,"CPU_INTERRUPT");
       if (cliente_interrupt == -1) {
               log_error(kernel_logger, "Error al aceptar un cliente");
               continue;
           }
       int *puntero_al_interrupt = malloc(sizeof(*puntero_al_interrupt));
       memcpy(puntero_al_interrupt, &cliente_interrupt, sizeof(int));
       //*puntero_al_dispatch = cliente_dispatch;
       pthread_t hilo_kernel_interrupt;
       pthread_create(&hilo_kernel_interrupt,NULL,manejar_kernel_interrupt,(void*)puntero_al_interrupt); //Creamos el hilo
       pthread_detach(hilo_kernel_interrupt);//El hilo se desacopla del hilo principal.
   }
}

void* manejar_kernel_interrupt(void *socket_interrupt){
   int interrupt = *((int *)socket_interrupt); // Desreferencio el puntero para obtener el socket del cliente
   free(socket_interrupt);
   op_code interrupt_id = recibir_op_code(interrupt); // !!!!!!!!!!DESPUES VER DE UNIFICAR LA FUNCION Q HIZO JERE EN EL UTILS DE RECIBIR CON UN CODE_OP PERO QUE SEA OP_CODE!!!!!!!!!!!!!!!!
   log_info(kernel_logger,"el interrupt es %d" ,interrupt);
   switch (interrupt_id)
   {
       case HANDSHAKE_CPU_INTERRUPT:
           //LOG_INFO : ES EL LOG OBLIGATORIO
           log_info(kernel_logger, "## CPU-INTERRUPT Conectado - FD del socket: %d", interrupt);
           enviar_op_code(interrupt, HANDSHAKE_ACCEPTED);    
    
           //acá tendriamos que esperar otro opcode que puede ser una syscall o alguna otra cosa
           break;        
           default:
           log_warning(kernel_logger, "No se pudo identificar al cliente; op_code: %d", interrupt); //AVISA Q FUCNIONA MAL
           break;
   }
   return NULL;
}

void atender_kernel_io(){
  
   int servidor_kernel = iniciar_servidor(PUERTO_ESCUCHA_IO,kernel_logger,"El kernel se conecto y esta esperando al IO");
   if (servidor_kernel == -1) {
       log_error(kernel_logger, "Error al iniciar el servidor de kernel");
       exit(EXIT_FAILURE);
   }
   log_debug(kernel_logger, "Servidor de kernel para dispatch iniciado en el puerto %s", PUERTO_ESCUCHA_IO);// Log de inicio del servidor
   while (1) { // sigue ciclando mientras no se desconecte el cliente
    cliente_io = esperar_cliente(servidor_kernel,kernel_logger,"IO"); //este es el socket que hay que utilizar para comunicarse
   if (cliente_io == -1) {
           log_error(kernel_logger, "Error al aceptar un cliente");
           continue;
       }
      
   int *puntero_al_io= malloc(sizeof(*puntero_al_io));
   //*puntero_al_io = cliente_io;
   memcpy(puntero_al_io, &cliente_io, sizeof(int));
   pthread_t hilo_kernel_io;
   pthread_create(&hilo_kernel_io,NULL,manejar_kernel_io,(void*)puntero_al_io); //Creamos el hilo
   pthread_detach(hilo_kernel_io);//El hilo se desacopla del hilo principal.
      
   }
   }

void* manejar_kernel_io(void *socket_io){
   int io = *((int *)socket_io); // Desreferencio el puntero para obtener el socket del cliente
   free(socket_io);
   op_code io_id = recibir_op_code(io);
 
   // log_info(kernel_logger, "Valor recibido en io_id: %d", io_id); verificammos que el valor recibido sea el correcto
   switch (io_id)
   {       
       case HANDSHAKE_IO:
           //LOG_INFO : ES EL LOG OBLIGATORIO

            log_info(kernel_logger, "## IO Conectado - FD del socket: %d", io);
            printf("\n");
            enviar_op_code(io, HANDSHAKE_ACCEPTED);  
            t_paquete *paquete = recibir_paquete(io);
            char *nombre = deserializar_nombre_io(paquete);
            log_debug(kernel_debug_log,"EL nombre tiene la cantidad de : %i",(int)strlen(nombre));
            log_info(kernel_logger,"se conecto el io con nombre; %s",nombre); // %s espera un puntero a char
            list_add(ios_conectados,nombre);
            log_debug(kernel_debug_log,"Se agrego el io %s a la lista",nombre);
            break;
            default:
           log_warning(kernel_logger, "No se pudo identificar al cliente; op_code: %d", io); //AVISA Q FUCNIONA MAL
           break;
   }
   return NULL;
}

int iniciar_conexion_kernel_memoria(){ //aca tendriamos que mandar el proceso con el atributo del tamaño ya agarrado de cpu
   int fd_kernel_memoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA);


   enviar_op_code(fd_kernel_memoria, HANDSHAKE_KERNEL); //avisa que es Kernel.


   op_code respuesta = recibir_op_code(fd_kernel_memoria); //recibe un entero que devuelve el kernel cuandola conexion esta hecha.
   if (respuesta == HANDSHAKE_ACCEPTED){
       log_info(kernel_logger, "Conexion con memoria establecida correctamente");
       return fd_kernel_memoria; // en realidad tendriamos que devolver si se puede o no iniciar cierto proceso
   }
   else{
       log_error(kernel_logger, "Error en la conexion con memoria");
       exit(EXIT_FAILURE);
   }
}
//aca hay que hacer una funcion que cierre la conexion con memoria cuando yo quiera, haciendo un close del socket fdkernelmemoria 

void cerrar_conexion(int socket){
    close (socket);
}

void solicitar_rafaga_de_io(int duracion){
    enviar_op_code(cliente_io,EJECUTAR_RAFAGA_IO); // solicita al io ejecutar una rafaga
    op_code respuesta = recibir_op_code(cliente_io); // recibe la respuesta
    if(respuesta == RAFAGA_ACEPTADA){
        enviar_entero(cliente_io,duracion); // si hay una instancia de io disponible le manda la duracion de la rafaga
    }
    else{
        // hay que ver esto
    
} // esta funcion capaz estaria mejor que solo devuelva la respuesta pero para ir probandola la dejamos en void
}

bool solicitar_permiso_a_memoria(int socket,int tamanio){
    op_code respuesta;
    enviar_entero(socket, tamanio);
    respuesta = recibir_op_code(socket);
    if(respuesta == HAY_ESPACIO_EN_MEMORIA){
        return true;
    }
    else{
        if(respuesta == NO_HAY_ESPACIO_EN_MEMORIA){
            return false;
        }
        else{
            return false;
        }
    }
   
}