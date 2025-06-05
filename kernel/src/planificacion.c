#include <planificacion.h>
#include <readline/readline.h>
#include <string.h>

t_list* procesos = NULL;
t_list* colaEstados[7]={NULL};
int identificador_del_proceso = 0;
struct pcb *ultimo_proceso_en_entrar;
int rafaga_de_prueba = 4;
void crear_proceso(int tamanio,char *ruta_archivo) { // tambien tiene que recibir el tamanio y el path
    struct pcb* pcb = malloc(sizeof(struct pcb));
    pcb = inicializar_un_proceso(pcb,tamanio,ruta_archivo);
    //pcb -> estado = NEW; // seguramente no sirva mucho
    transicionar_a_new(pcb);
    log_info(kernel_logger,"Se creo el proceso con el PID: %i",identificador_del_proceso);
    log_info(kernel_logger,"Su rafaga es de: %i",rafaga_de_prueba);
    incrementar_var_global_id_proceso();
  return; 
}

void *planificador_proceso_mas_chico_primero(){
    esperar_enter_por_pantalla();
    log_debug(kernel_debug_log,"INICIANDO PLANIFICADOR DE LARGO PLAZO TMCP");
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN_NEW); // si no hay nada espera a que llegue un proceso
        sem_wait(&INTENTAR_INICIAR);
        struct pcb* primer_proceso = obtener_primer_proceso_de_new();
        if(primer_proceso->pid == ultimo_proceso_en_entrar->pid){
                bool respuesta = consultar_si_puede_entrar(primer_proceso);
                log_debug(kernel_debug_log,"Conexion con memoria cerrada");
                if(respuesta == true){
                    struct pcb *proceso = sacar_primero_de_la_lista(NEW);
                    cambiarEstado(proceso,NEW,READY);
                    sem_post(&CANTIDAD_DE_PROCESOS_EN_READY);
                    struct pcb *primer_proceso_actualizado = obtener_primer_proceso_de_new();
                    if(primer_proceso_actualizado!=NULL){
                        pthread_mutex_lock(&mx_ultimo_en_entrar);
                        ultimo_proceso_en_entrar = primer_proceso_actualizado;
                        pthread_mutex_unlock(&mx_ultimo_en_entrar);
                    }
                }
        }
        sem_post(&INTENTAR_INICIAR);
    }
}

void *planificador_largo_plazo_fifo(){
    esperar_enter_por_pantalla();
    log_debug(kernel_debug_log,"INICIANDO PLANIFICADOR DE LARGO PLAZO");
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN_NEW); // si no hay nada espera a que llegue un proceso
        sem_wait(&INGRESO_DEL_PRIMERO); //que los demas esperen a que uno entre
        struct pcb* primer_proceso = obtener_primer_proceso_de_new(); //no lo sacamos de la lista todavia
        bool respuesta = consultar_si_puede_entrar(primer_proceso);
        log_debug(kernel_debug_log,"Conexion con memoria cerrada");
        if (respuesta == true){
            struct pcb *aux = sacar_primero_de_la_lista(NEW); //Una vez que tenemos la confirmacion de memoria ahi si lo sacamos de la lista
            cambiarEstado(aux,NEW,READY);
            sem_post(&INGRESO_DEL_PRIMERO);
            sem_post(&CANTIDAD_DE_PROCESOS_EN_READY);
        }   
        else{
            log_debug(kernel_debug_log,"NO HAY ESPACIO SUFICIENTE EN MEMORIA");
            //aca va un semaforo, esperando para entrar a memoria una vez que termine otro proceso.
            //sem_post(&INGRESO_DEL_PRIMERO); 
        }
          } 
    
    return NULL;
    }

void *planificador_corto_plazo_fifo(){
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN_READY);
        log_debug(kernel_debug_log,"Ya pase pero no hay cpus");
        //sem_wait(&CPUS_LIBRES);
        //sem_wait(&INGRESO_DEL_PRIMERO_READY); // a chequear
        usleep(3000000); 
        log_debug(kernel_debug_log,"PLANI DE CORTO PLAZO INICIADO");
        struct pcb* proceso = sacar_primero_de_la_lista(READY);
        cambiarEstado(proceso,READY,EXEC);
       /* pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
        int pos_cpu = buscar_cpu_libre(cpus_conectadas);
        struct instancia_de_cpu *cpu_aux = list_get(cpus_conectadas,pos_cpu);
        pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
        log_debug(kernel_debug_log,"El proceso %i pasa a ejecutar en la cpu %i",proceso->pid,cpu_aux->id_cpu);
        */
        //poner_a_ejecutar(proceso);
        //sem_post(&INGRESO_DEL_PRIMERO_READY);
    }
}   

void *planificador_corto_plazo_sjf_sin_desalojo(){
      while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN_READY);
        //sem_wait(&CPUS_LIBRES);
        //sem_wait(&INGRESO_DEL_PRIMERO_READY); //Preguntar si es necesario pero creeriamos que con el de cantProcesos y el de las cpus ya esta
        usleep(3000000); 
        log_debug(kernel_debug_log,"PLANI DE CORTO PLAZO INICIADO");
        pthread_mutex_lock(&mx_usar_cola_estado[READY]); //Preguntar en el soporte si tiene sentido el mutex
        list_sort(colaEstados[READY],menor_por_rafaga);
        pthread_mutex_unlock(&mx_usar_cola_estado[READY]);
        struct pcb* proceso = sacar_primero_de_la_lista(READY);
        /*pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
        int pos_cpu = buscar_cpu_libre(cpus_conectadas);
        struct instancia_de_cpu *cpu_aux = list_get(cpus_conectadas,pos_cpu);
        pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
        log_debug(kernel_debug_log,"El proceso %i pasa a ejecutar en la cpu %i",proceso->pid,cpu_aux->id_cpu);
        */
        cambiarEstado(proceso,READY,EXEC);
        //poner_a_ejecutar(proceso);
        //sem_post(&INGRESO_DEL_PRIMERO_READY);
    }
}

//FUNCIONES AUXILIARES

struct pcb* inicializar_un_proceso(struct pcb*pcb,int tamanio,char *ruta_archivo){
    pcb -> pid = identificador_del_proceso; //ver lo de la variable global
    pcb -> pc = 0;
    pcb -> tamanio = tamanio;
    pcb -> ruta_del_archivo_de_pseudocodigo = ruta_archivo;
    pcb->ya_ejecuto_en_cpu = false;
    pcb->ultima_estimacion = atoi(ESTIMACION_INICIAL);
    pcb->rafaga_actual_cpu = rafaga_de_prueba;
    rafaga_de_prueba--;
    //pcb -> rafaga_actual_cpu = calcular_rafaga(pcb); despues descomentar
    return pcb;
}

void ordenar_lista_segun(t_list *lista,bool (*comparador)(void *, void *)){
 list_sort(lista,comparador); 
}

void transicionar_a_new(struct pcb *pcb){
 pthread_mutex_lock(&mx_usar_cola_estado[NEW]); 
    //if(strcmp(ALGORITMO_INGRESO_A_READY,FIFO)==0){
        list_add(colaEstados[NEW],pcb); // es una variable global asi que habria que poner un mutex
    //}else{
        //insertar_ordenado_segun(colaEstados[NEW],pcb,menor_por_tamanio);
        //pthread_mutex_lock(&mx_ultimo_en_entrar);
        //ultimo_proceso_en_entrar = pcb;
        //pthread_mutex_unlock(&mx_ultimo_en_entrar);
    //}
    pthread_mutex_unlock(&mx_usar_cola_estado[NEW]);
    sem_post(&CANTIDAD_DE_PROCESOS_EN_NEW);
    //un signal de un semaforo que le avise al plani de largo plazo por menor tamanio que se creo un proceso
}

void incrementar_var_global_id_proceso(){
    pthread_mutex_lock(&mx_identificador_del_proceso);
    identificador_del_proceso++; 
    pthread_mutex_unlock(&mx_identificador_del_proceso);
}

struct pcb *obtener_primer_proceso_de_new(){
   if(list_is_empty(colaEstados[NEW])){
        return NULL;
    }
    else{
    pthread_mutex_lock(&mx_usar_cola_estado[NEW]); // es una variable global asi que la protegemos (mejor un mx)
        t_list *aux = colaEstados[NEW];
        struct pcb *proceso = list_get(aux, 0);  // Obtener el primer elemento pero sin sacarlo de la lista todavia
    pthread_mutex_unlock(&mx_usar_cola_estado[NEW]); // es una variable global asi que la protegemos (mejor un mx)
    return proceso;
    }
    }

bool consultar_si_puede_entrar(struct pcb *proceso){
    int socket = iniciar_conexion_kernel_memoria();
    bool respuesta = solicitar_permiso_a_memoria(socket,proceso->tamanio); //(Adentro de la funcion, vamos a manejar un op_code)
    cerrar_conexion(socket);
    return respuesta;
}

int buscar_en_lista(t_list *lista, int pid) {
   
    if (!lista) {
        printf("La lista no tiene ningan proceso\n");
        return -1;
    }
    t_list_iterator *aux = list_iterator_create(lista); //arranca apuntando a NULL, no a donde apunta a lista
    int pos = 0;
    while (list_iterator_has_next(aux)) { //es true mientras haya un siguiente al cual avanzar.
        struct pcb *pcb_aux = list_iterator_next(aux);
        if (pcb_aux->pid == pid) { // comparo al pid que estoy apuntando con el pid que busco.
            list_iterator_destroy(aux); //delete del iterador.
            return pos; // devuelvo la posicion en la que se encuentra porque nos va a servir para usar el list_get mas facil
        }
        pos++;
    }

    list_iterator_destroy(aux); // destruyo el iterador creado por mas que no haya encontrado el proceso que queriamos.
    printf("El proceso con PID %d no se encuentra en la lista\n", pid);
    return -1;
}

int buscar_cpu_libre(t_list *lista) {
    if (lista == NULL) { //no deberia pasar nunca porque esta sincronizado pero por ahora lo dejamos
    printf("Lista nula\n");
    return -1;
    }
    t_list_iterator *aux = list_iterator_create(lista); //arranca apuntando a NULL, no a donde apunta a lista
    int pos = 0;
    while (list_iterator_has_next(aux)) { //es true mientras haya un siguiente al cual avanzar.
        struct instancia_de_cpu *cpu_aux = list_iterator_next(aux);
        if (cpu_aux->puede_usarse) { // comparo al pid que estoy apuntando con el pid que busco.
            cpu_aux->puede_usarse = false;
            list_iterator_destroy(aux); //delete del iterador.
            return pos; // devuelvo la posicion en la que se encuentra porque nos va a servir para usar el list_get mas facil
        }
        pos++;
    }
    return -1;
}

int calcular_rafaga(struct pcb *proceso){
    if(proceso->ya_ejecuto_en_cpu){
        int rafaga;
        float alfa = (float)atof(ALFA);
        rafaga = alfa * proceso->duracion_ultima_rafaga + (1-alfa)*proceso->ultima_estimacion;
        return rafaga;
    }
    else{
        return (atoi(ESTIMACION_INICIAL));
    }
}

bool menor_por_rafaga(void* a, void* b){
    struct pcb* p1 = (struct pcb*) a;
    struct pcb* p2 = (struct pcb*) b;
    return p1->rafaga_actual_cpu < p2->rafaga_actual_cpu;
}

bool menor_por_tamanio(void* a, void* b) {
    
    struct pcb* p1 = (struct pcb*) a;
    struct pcb* p2 = (struct pcb*) b;
    return p1->tamanio < p2->tamanio;
}

void insertar_ordenado_segun(t_list *lista, struct pcb *proceso, bool (*comparador)(void *, void *)) {
    list_add_sorted(lista, proceso, comparador);
}

       
void esperar_enter_por_pantalla(){
    char *line;
printf("Se esta esperando un enter por pantalla");
    do {
        line = readline("");
    } while (strlen(line) != 0); // si la longitud es mayor a 0 quiere decir que no se ingreso solo un enter
    free(line);  
}


struct pcb *sacar_primero_de_la_lista(Estado estado){
    struct pcb *aux;
    pthread_mutex_lock(&mx_usar_cola_estado[estado]);
    aux =list_remove(colaEstados[estado],0); //como el list add agrerga al final, sacamos del principio para no romper el comportamiento de la cola
    pthread_mutex_unlock(&mx_usar_cola_estado[estado]);
    return aux;
}

 char *cambiar_a_string(Estado estado) {
     char* nombres_estados[] = { 
        "NEW",
        "READY",
        "BLOCKED",
        "EXEC",
        "EXIT_ESTADO",
        "READY_SUSPEND",
        "BLOCKED_SUSPEND"
    };

    if (estado < 0 || estado > BLOCKED_SUSPEND) {
        return "ESTADO_DESCONOCIDO";
    }

    return nombres_estados[estado];
}

void cambiarEstado (struct pcb* pcb,Estado estadoAnterior, Estado estadoNuevo){
    char *string_estado_nuevo = cambiar_a_string(estadoNuevo);
    char *string_estado_anterior = cambiar_a_string(estadoAnterior);
    log_info(kernel_logger,"El proceso %i cambia del estado %s a %s ",pcb->pid,string_estado_anterior,string_estado_nuevo);
    pthread_mutex_unlock(&mx_usar_cola_estado[estadoNuevo]);
    list_add(colaEstados[estadoNuevo],pcb); //preguntar si hay que usar mutex
    pthread_mutex_unlock(&mx_usar_cola_estado[estadoNuevo]);
}

void mandar_paquete_a_cpu(struct pcb *proceso){
        t_buffer *buffer = crear_buffer_cpu(proceso->pc,proceso->pid);
        crear_paquete(ENVIO_PID_Y_PC,buffer,cliente_dispatch); //esta funcion crea el paquete y tambien lo envia
}

int manejar_dump(struct pcb *aux){
    int socket = iniciar_conexion_kernel_memoria();
    t_buffer *buffer = mandar_pid_a_memoria(aux->pid);
    crear_paquete(DUMP_MEMORY,buffer,socket);
    cambiarEstado(aux,EXEC,BLOCKED);
    int respuesta = recibir_entero(socket);
    return respuesta;
}

int finalizar_proceso(struct pcb*aux){
    cambiarEstado(aux,EXEC,EXIT_ESTADO);
    int socket = iniciar_conexion_kernel_memoria();
    t_buffer *buffer = mandar_pid_a_memoria(aux->pid);
    crear_paquete(EXIT,buffer,socket);
    int confirmacion = recibir_entero(socket);
    return confirmacion;
}

void poner_a_ejecutar(struct pcb* aux){
    bool bloqueante = false;
    while(!bloqueante){
        mandar_paquete_a_cpu(aux);
       t_paquete *paquete = recibir_paquete(cliente_dispatch); //cpu ejecuta una instruccion y nos devuelve el pid con una syscall
        op_code syscall = obtener_codigo_de_operacion(paquete); //deserializa el opcode del paquete
        switch(syscall){
            case INIT_PROC:
                char *nombre_archivo = deserializar_nombre_archivo(paquete);
                int tamanio = deserializar_tamanio (paquete);
                crear_proceso(tamanio,nombre_archivo);
                //avisar que termine
                break;
            case EXIT:
                //int confirmacion = finalizarProceso(aux); descomentar despues
                //preguntar si puede llegar a pasar que se rechace
                //tener en cuenta lo del mediano plazo
                free(aux);
                bloqueante = true;
                break;
            case DUMP_MEMORY:
                int respuesta = manejar_dump(aux); //esta funcion manda el proceso a BLOCKED
                if(respuesta == DUMP_ACEPTADO){
                  cambiarEstado(aux,BLOCKED,READY);  
                }
                else{
                  cambiarEstado(aux,BLOCKED,EXIT);
                }
                bloqueante = true; // a chequear
                break;
                
            case IO:
                //int milisegundos = deserializar_cant_segundos(paquete);
                char *nombre_io_a_usar = deserializar_nombre_syscall_io(paquete);
                //agregar mutex
                int posicionIO = buscar_IO_solicitada(ios_conectados,nombre_io_a_usar);
                if(posicionIO == -1){ //quiere decir que no hay ninguna syscall con ese nombre
                    cambiarEstado(aux,EXEC,EXIT_ESTADO);
                }else{
                    cambiarEstado(aux,EXEC,BLOCKED);
                    pthread_mutex_lock(&mx_usar_recurso[IO]);
                    struct instancia_de_io *io_aux = list_get(ios_conectados,posicionIO);
                    if(io_aux->cantInstancias > 0){
                        //crear paquete con el pid y los milisegundos y mandarlo a IO.
                        pthread_mutex_unlock(&mx_usar_recurso[REC_IO]);
                    }
                    else{
                        list_add(io_aux->procesos_esperando,aux);
                        pthread_mutex_unlock(&mx_usar_recurso[REC_IO]);
                    }
                }
                bloqueante = true;
                break;
            default:
            log_debug(kernel_debug_log,"SYSCALL DESCONOCIDA");
                break;
        }
    }
}

