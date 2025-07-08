
#include <conexiones.h>


//sacamos los close porque se recibe mas de un io y mas de una cpu

t_list *ios_conectados = NULL; // variable global
t_list *cpus_conectadas = NULL;
int cliente_dispatch;
int cliente_interrupt;
void *atender_kernel_dispatch(){
   int servidor_kernel = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH,kernel_logger,"El kernel se conecto y esta esperando al dispatch");
   if (servidor_kernel == -1) {
       log_error(kernel_logger, "Error al iniciar el servidor de kernel");
       exit(EXIT_FAILURE);
   }
   log_debug(kernel_logger, "Servidor de kernel para dispatch iniciado en el puerto %s", PUERTO_ESCUCHA_DISPATCH);// Log de inicio del servidor
    while (1) { // sigue ciclando mientras no se desconecte el cliente
    // Aca esperan al cliente (CPU)
    int cliente_dispatch;
    cliente_dispatch = esperar_cliente(servidor_kernel,kernel_logger,"CPU_DISPATCH");
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
           int id;
           recv(dispatch, &id, sizeof(int), 0);
           struct instancia_de_cpu *aux = malloc(sizeof(struct instancia_de_cpu));
           aux->id_cpu = id;
           aux->puede_usarse = true;
           aux->socket_para_comunicarse = dispatch;
           log_debug(kernel_debug_log,"Se conecto la cpu con id %i",aux->id_cpu);
           pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
           list_add(cpus_conectadas,aux);
           pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
           log_debug(kernel_debug_log,"Se agrego a la lista la cpu con id: %i ", aux->id_cpu); //%i espera un entero
           int tamanio_lista = list_size(cpus_conectadas);
           log_debug(kernel_debug_log,"La lista tiene %i cpus",tamanio_lista);
           sem_post(&CPUS_LIBRES);
           //acá tendriamos que esperar otro opcode que puede ser una syscall o alguna otra cosa
           break;
           //case HANDSHAKE_CPU_INTERRUPT:
          
       default:
           log_warning(kernel_logger, "No se pudo identificar al cliente; op_code: %d", dispatch); //AVISA Q FUCNIONA MAL
           break;
   }
   return NULL;
}

void *atender_kernel_interrupt(){
   int servidor_kernel_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT,kernel_logger,"El kernel se conecto y esta esperando al interrupt");
   if (servidor_kernel_interrupt == -1) {
       log_error(kernel_logger, "Error al iniciar el servidor de kernel");
       exit(EXIT_FAILURE);
   }
   log_debug(kernel_logger, "Servidor de kernel para interrupt iniciado en el puerto %s", PUERTO_ESCUCHA_INTERRUPT);// Log de inicio del servidor
   while (1) { // sigue ciclando mientras no se desconecte el cliente
       // Aca esperan al cliente (CPU)
       int cliente_interrupt;
       cliente_interrupt = esperar_cliente(servidor_kernel_interrupt,kernel_logger,"CPU_INTERRUPT");
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
            //buscar la cpu con el id
            //aux->socket_interrupt = interrupt;
           //acá tendriamos que esperar otro opcode que puede ser una syscall o alguna otra cosa
           break;        
           default:
           log_warning(kernel_logger, "No se pudo identificar al cliente; op_code: %d", interrupt); //AVISA Q FUCNIONA MAL
           break;
   }
   return NULL;
}

void *atender_kernel_io(){
  
    int servidor_kernel = iniciar_servidor(PUERTO_ESCUCHA_IO,kernel_logger,"El kernel se conecto y esta esperando al IO");
    if (servidor_kernel == -1) {
        log_error(kernel_logger, "Error al iniciar el servidor de kernel");
        exit(EXIT_FAILURE);
    }
    log_debug(kernel_logger, "Servidor de kernel para dispatch iniciado en el puerto %s", PUERTO_ESCUCHA_IO);// Log de inicio del servidor
    while (1) { // sigue ciclando mientras no se desconecte el cliente
        int cliente_io;
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
            struct instancia_de_io *io_aux = malloc(sizeof(struct instancia_de_io)); 
            t_paquete *paquete = recibir_paquete(io);
            char *nombre = deserializar_nombre_io(paquete);
            log_debug(kernel_debug_log,"EL nombre tiene la cantidad de : %i",(int)strlen(nombre));
            io_aux->socket_io_para_comunicarse = io;
            io_aux->nombre = nombre;
            io_aux->hay_procesos_esperando = malloc(sizeof(sem_t));
            pthread_mutex_lock(&mx_usar_recurso[REC_IO]);
            int pos = buscar_IO_solicitada(ios_conectados,nombre);
            pthread_mutex_unlock(&mx_usar_recurso[REC_IO]);
            if(pos ==-1){
                sem_init(io_aux->hay_procesos_esperando,0,0);
            }
            else{  
                pthread_mutex_lock(&mx_usar_recurso[REC_IO]);
                struct instancia_de_io *io_que_ya_estaba = list_get(ios_conectados,pos);
                pthread_mutex_unlock(&mx_usar_recurso[REC_IO]);
                io_aux->hay_procesos_esperando = io_que_ya_estaba->hay_procesos_esperando;
            }
            pthread_mutex_lock(&mx_usar_recurso[REC_IO]);
            list_add(ios_conectados,io_aux);
            pthread_mutex_unlock(&mx_usar_recurso[REC_IO]);
            log_debug(kernel_debug_log,"Se conecto una instancia de: %s",nombre);
            pthread_create(&io_aux->hilo_instancia_de_io,NULL,esperar_io_proceso,io_aux);
            pthread_detach(io_aux->hilo_instancia_de_io);
        break;
        default:
            log_warning(kernel_logger, "No se pudo identificar al cliente; op_code: %d", io); //AVISA Q FUCNIONA MAL
        break;
   }
   return NULL;
}


void *esperar_io_proceso(void *instancia_de_io) { //el aux
    struct instancia_de_io *io_aux = instancia_de_io;
    while (true){
        sem_wait(io_aux->hay_procesos_esperando); //positivos = cant procesos esperando, negativo = cant ios disponibles
        pthread_mutex_lock(&mx_usar_cola_estado[BLOCKED]);
        struct pcb *proceso = buscar_proceso_bloqueado_por_io(io_aux->nombre);
        pthread_mutex_unlock(&mx_usar_cola_estado[BLOCKED]);
        enviar_entero(io_aux->socket_io_para_comunicarse,proceso->proxima_rafaga_io);
        int respuesta = recibir_entero(io_aux->socket_io_para_comunicarse);
        
        switch(respuesta){
            case FIN_DE_IO: //Corresponde al enum de fin de IO
                pthread_mutex_lock(&mx_usar_cola_estado[BLOCKED]);
                int pos = buscar_en_lista(colaEstados[BLOCKED],proceso->pid);
                pthread_mutex_unlock(&mx_usar_cola_estado[BLOCKED]);
                if(pos!=-1){
                    sacar_proceso_de_cola_de_estado(proceso,BLOCKED);
                    proceso->proxima_estimacion = calcular_proxima_estimacion(proceso); 
                    transicionar_a_ready(proceso,BLOCKED);
                }
                else{
                    sacar_proceso_de_cola_de_estado(proceso,SUSP_BLOCKED);
                    transicionar_a_susp_ready(proceso);
                }
                break;
            case -1: //desconexion de la instancia con la que estamos trabajando
                finalizar_proceso(proceso,BLOCKED);
                //hacer funcion que devuelva la cantidad de ios con cierto nombre
                break;  
        }
    }
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


void solicitar_rafaga_de_io(int duracion,struct instancia_de_io *io_a_usar){
    enviar_op_code(io_a_usar->socket_io_para_comunicarse,EJECUTAR_RAFAGA_IO); // solicita al io ejecutar una rafaga
    op_code respuesta = recibir_op_code(io_a_usar->socket_io_para_comunicarse); // recibe la respuesta
    if(respuesta == RAFAGA_ACEPTADA){
        enviar_entero(io_a_usar->socket_io_para_comunicarse,duracion); // si hay una instancia de io disponible le manda la duracion de la rafaga
    }
    else{
        // hay que ver esto
    } // esta funcion capaz estaria mejor que solo devuelva la respuesta pero para ir probandola la dejamos en void
}

bool solicitar_permiso_a_memoria(int socket,struct pcb* proceso,op_code operacion){
    op_code respuesta;
    //enviar_entero(socket, tamanio);
    t_buffer *buffer = crear_buffer_de_envio_de_proceso(proceso->pid,proceso->ruta_del_archivo_de_pseudocodigo,proceso->tamanio);//tiene que tener el tamanio, el pid, el archivo de pseudocodigo
    crear_paquete(operacion,buffer,socket);
    respuesta = recibir_op_code(socket);
    if(respuesta == ACEPTAR_PROCESO){
        return true;
    }
    else{
        return false;
    }
   
}

int buscar_IO_solicitada(t_list *lista, char* nombre_io) {
    if (list_is_empty(lista)) {
        log_debug(kernel_debug_log, "La lista no tiene ninguna instancia de IO\n");
        return -1;
    }

    t_list_iterator *iterador = list_iterator_create(lista);
    int pos = 0;

    while (list_iterator_has_next(iterador)) {
        struct instancia_de_io *io_aux = list_iterator_next(iterador);
        if (strcmp(io_aux->nombre, nombre_io) == 0) {
            list_iterator_destroy(iterador);
            return pos;
        }
        pos++;
    }

    list_iterator_destroy(iterador);
    log_debug(kernel_debug_log, "La instancia de IO con nombre %s no se encuentra en la lista\n", nombre_io);
    return -1;
}

struct pcb* obtener_primero(t_list *lista){
    if(!lista){
        return NULL;
    }
    pthread_mutex_lock(&mx_usar_recurso[REC_IO]);
    struct pcb* aux = list_remove(lista,0);
    pthread_mutex_unlock(&mx_usar_recurso[REC_IO]);
    return aux;
}
void liberar_io(struct instancia_de_io *io){
    free(io->nombre);
    sem_destroy(io->hay_procesos_esperando);
    free(io);
}

void recorrer_lista_y_finalizar_procesos(t_list * lista){
    if (lista == NULL) { //no deberia pasar nunca porque esta sincronizado pero por ahora lo dejamos
        printf("Lista nula\n");
    return;
    }
    t_list_iterator *aux = list_iterator_create(lista); //arranca apuntando a NULL, no a donde apunta a lista
    while (list_iterator_has_next(aux)) { //es true mientras haya un siguiente al cual avanzar.
        list_iterator_next(aux);
        struct pcb *proceso = obtener_primero(lista);
        pthread_mutex_lock(&mx_usar_cola_estado[BLOCKED]);
        int pos = buscar_en_lista(colaEstados[BLOCKED],proceso->pid);
        pthread_mutex_unlock(&mx_usar_cola_estado[BLOCKED]);
        if(pos!=-1){ //Sigue en BLOCKED
            finalizar_proceso(proceso,BLOCKED);
        }
        else{ //Se suspendio
            finalizar_proceso(proceso,SUSP_BLOCKED);
        }
    }
    list_iterator_destroy(aux);
    return;
}