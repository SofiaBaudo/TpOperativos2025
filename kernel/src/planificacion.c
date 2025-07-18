#include <planificacion.h>
#include <readline/readline.h>
#include <string.h>

t_list* procesos = NULL;
t_list* colaEstados[7]={NULL};
int identificador_del_proceso = 0;
struct pcb *proximo_a_consultar;


void crear_proceso(int tamanio,char *ruta_archivo) { // 
    struct pcb* proceso = malloc(sizeof(struct pcb)); //creo estructura con tamaño del proceso.
    proceso = inicializar_un_proceso(proceso,tamanio,ruta_archivo); //inicializo las estructuras del proceso
    transicionar_a_new(proceso); // mando el proceso al estado new
    log_info(kernel_logger,"## (< PID: %i>) Se crea el proceso - ESTADO: NEW",proceso->pid);
    incrementar_var_global_id_proceso(); //incremento la variable global, si proceso que llega es N, el proximo es N+1
    return; 
}

void *planificador_largo_plazo_fifo(){
    esperar_enter_por_pantalla(); //espera a un enter para poder empezar
    log_debug(kernel_debug_log,"INICIANDO PLANIFICADOR DE LARGO PLAZO");
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[NEW]); // inicializado en 0, cuando llega algo a NEW, se activa.
        sem_wait(&INTENTAR_INICIAR_NEW); //activa el planificador si se suspende un proceso, finaliza un proceso o se pone a ejecutar.
        sem_wait(&UNO_A_LA_VEZ[NEW]); //este semaforo hace que el planificador maneje un proceso a la vez.
        if(!list_is_empty(colaEstados[SUSP_READY])){ //espera a que la cola de suspendido ready este vacía para poder actuar.
            sem_wait(&SUSP_READY_SIN_PROCESOS);
        }
        struct pcb* primer_proceso = obtener_copia_primer_proceso_de(NEW); //obtengo una copia del primer proceso de la cola de NEW.
        bool respuesta = consultar_si_puede_entrar(primer_proceso,INICIALIZAR_PROCESO_DESDE_NEW); //consulto a memoria si hay espacio disponible para entrar.
        if(respuesta){
            primer_proceso = sacar_primero_de_la_lista(NEW); //Una vez que tenemos la confirmacion de memoria ahi si lo sacamos de la cola de NEW
            transicionar_a_ready(primer_proceso,NEW); //mandamos el proceso a READY.
            sem_post(&INTENTAR_INICIAR_NEW); //incremento el semaforo para que pueda iniciar el planificador.
        }   
        else{
            sem_post(&CANTIDAD_DE_PROCESOS_EN[NEW]); //si no hay espacio en memoria, aviso que el proceso sigue estando en new.
        }
        sem_post(&UNO_A_LA_VEZ[NEW]);//me aseguro que se siga tratando de a un proceso.
    } 
    return NULL;
}

void *planificador_largo_plazo_proceso_mas_chico_primero(){
    esperar_enter_por_pantalla();
    log_debug(kernel_debug_log,"INICIANDO PLANIFICADOR DE LARGO PLAZO TMCP");
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[NEW]);// inicializado en 0, cuando llega algo a NEW, se activa.
        sem_wait(&INTENTAR_INICIAR_NEW); //que los demas esperen a que uno entre
        sem_wait(&UNO_A_LA_VEZ[NEW]); //este semaforo hace que el planificador maneje un proceso a la vez.
        //verifico que la cola de SUSP_READY este vacía.
        if(!list_is_empty(colaEstados[SUSP_READY])){
            sem_wait(&SUSP_READY_SIN_PROCESOS); //espero a que se vacíe la lista de susp ready.
        }
        struct pcb* primer_proceso = obtener_copia_primer_proceso_de(NEW); //obtengo una copia del proceso a consultar
        if(primer_proceso->pid == proximo_a_consultar->pid){ 
            bool respuesta = consultar_si_puede_entrar(primer_proceso,INICIALIZAR_PROCESO_DESDE_NEW);
            if(respuesta){
                primer_proceso = sacar_primero_de_la_lista(NEW); //saco el primer proceso de la lista de NEW
                transicionar_a_ready(primer_proceso,NEW); // lo transiciono a READY
                actualizar_proximo_a_consultar(NEW); //actualizo el proximo proceso a consultar
                if(!list_is_empty(colaEstados[NEW])){ //si la cola de NEW no esta vacía, intento iniciarla  
                    sem_post(&INTENTAR_INICIAR_NEW); 
                }
            }
            else{
                sem_post(&CANTIDAD_DE_PROCESOS_EN[NEW]); //aviso que el proceso sigue en new
            }
        }
        else{
            log_debug(kernel_debug_log,"El ultimo proceso en entrar no es el mas chico");
            sem_post(&CANTIDAD_DE_PROCESOS_EN[NEW]);  //aviso que el proceso sigue en new
        }
        sem_post(&UNO_A_LA_VEZ[NEW]); //me aseguro que se siga tratando de a un proceso a la vez.
    }
}

void *planificador_corto_plazo_fifo(){
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[READY]); //se activa cuando hay un proceso en la cola de ready.
        sem_wait(&CPUS_LIBRES); //se activa cuando hay una cpu libre
        log_debug(kernel_debug_log,"PLANI DE CORTO PLAZO INICIADO");
        pthread_mutex_lock(&mx_usar_recurso[REC_CPU]); //semaforo que hace que no se use el recurso de la cpu dos veces al mismo tiempo.
        int pos_cpu = buscar_cpu_libre(cpus_conectadas); //busco la posicion de la cpu libre.
        struct instancia_de_cpu *cpu_aux = list_get(cpus_conectadas,pos_cpu); //agarro la cpu libre
        log_debug(kernel_debug_log,"La cpu libre es: %i",cpu_aux->id_cpu);
        pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
        struct pcb* proceso = sacar_primero_de_la_lista(READY); //saco el primer proceso de la cola de READY
        cambiarEstado(proceso,READY,EXEC); //le cambio el estado a EXECUTE
        log_debug(kernel_debug_log,"El proceso %i pasa a ejecutar en la cpu %i",proceso->pid,cpu_aux->id_cpu);
        crear_hilo_de_ejecucion(proceso,cpu_aux); // pongo a ejecutar el proceso en la CPU.
    }
}   

/*EL PROCESO ARRANCA CON UNA ESTIMACION INICIAL, POR ESO EN LOS PLANIFICADORES NO SE CALCULA 
LA SIGUIENTE ESTIMACION, SINO QUE SE CALCULA CUANDO TERMINA DE EJECUTAR, SI ES QUE NO SE FINALIZA LA
EJECUCION DEL PROCESO.*/

void *planificador_corto_plazo_sjf_sin_desalojo(){
      while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[READY]); //se activa cuando hay un proceso en la cola de ready.
        sem_wait(&CPUS_LIBRES); //se activa cuando hay una cpu libre
        log_debug(kernel_debug_log,"PLANI DE CORTO PLAZO INICIADO");
        pthread_mutex_lock(&mx_usar_recurso[REC_CPU]); //semaforo que hace que no se use el recurso de la cpu dos veces al mismo tiempo.
        int pos_cpu = buscar_cpu_libre(cpus_conectadas); //busco la posicion de la cpu libre.
        struct instancia_de_cpu *cpu_aux = list_get(cpus_conectadas,pos_cpu); //agarro la cpu libre
        pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
        struct pcb* proceso = sacar_primero_de_la_lista(READY); //Saco el proceso de la cola de READY
        cambiarEstado(proceso,READY,EXEC); //Cambio su estado a EXECUTE
        log_debug(kernel_debug_log,"El proceso %i pasa a ejecutar en la cpu %i",proceso->pid,cpu_aux->id_cpu);
        crear_hilo_de_ejecucion(proceso,cpu_aux); //Creo el hilo donde va a estar ejecutando el proceso con su respectiva CPU.
    }
}

/*EL PROCESO ARRANCA CON UNA ESTIMACION INICIAL, POR ESO EN LOS PLANIFICADORES NO SE CALCULA 
LA SIGUIENTE ESTIMACION, SINO QUE SE CALCULA CUANDO TERMINA DE EJECUTAR, SI ES QUE NO SE FINALIZA LA
EJECUCION DEL PROCESO.*/

void *planificador_corto_plazo_sjf_con_desalojo(){
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[READY]); //se activa cuando hay un proceso en la cola de ready.
        sem_wait(&CPUS_LIBRES); // se activa cuando hay una cpu libre
        sem_wait(&REPLANIFICAR); // avisa en el caso de que se haya replanificado (si se desaloja un proceso)
        pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
        int pos_cpu = buscar_cpu_libre(cpus_conectadas); //busco la cpu libre.
        pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
        struct pcb *proceso = obtener_copia_primer_proceso_de(READY); //obtengo una copia del primer proceso, sin sacarlo
        if(pos_cpu!=-1){ // si hay una cpu libre, pongo a ejecutar normal
            struct instancia_de_cpu *cpu_aux = obtener_cpu(pos_cpu); //obtengo la cpu
            proceso = sacar_primero_de_la_lista(READY); //saco el proceso de la cola de READY
            cambiarEstado(proceso,READY,EXEC); //Cambio su estado a EXECUTE
            crear_hilo_de_ejecucion(proceso,cpu_aux); //creamos un hilo de ejecucion enviando los parametros
        }
        else{
            bool desalojo = ver_si_hay_que_desalojar(proceso); //SI NO HAY UNA CPU LIBRE, ME FIJO SI HAY QUE DESALOJAR.
            if(desalojo){ // si hay que desalojar
                struct instancia_de_cpu *cpu_aux = buscar_cpu_con_proceso_con_mayor_estimacion(); //desalojo al proceso con mayor estimacion que esta ejecutando.
                //temporal_stop(cpu_aux->proceso_ejecutando->duracion_ultima_rafaga);
                enviar_op_code(cpu_aux->socket_interrupt,SOLICITO_DESALOJO); //solicito el desalojo a CPU
                pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
                reanudar_cronometros(cpus_conectadas,list_size(cpus_conectadas)); // reanudo los cronometros
                pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
                sem_post(&CANTIDAD_DE_PROCESOS_EN[READY]); //Porque todavia no desalojamos nada, simplemente dimos el aviso, 
                                                            // el desalojo se hace cuando se esta ejecutando el anterior proceso
            }
            else{
            log_debug(kernel_debug_log,"No se desaloja");
            pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
            reanudar_cronometros(cpus_conectadas,list_size(cpus_conectadas)); //reanudo los cronometros
            pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
            sem_post(&CANTIDAD_DE_PROCESOS_EN[READY]); // //Porque todavia no desalojamos nada, simplemente dimos el aviso, 
                                                    // el desalojo se hace cuando se esta ejecutando el anterior proceso
            }
        }
        sem_post(&CPUS_LIBRES); 
    }
}

void *planificador_mediano_plazo(){ 
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[SUSP_BLOCKED]); //se activa el planificador si llega un proceso a SUSP_BLOCKED
        log_warning(kernel_debug_log,"SE SUSPENDIO UN PROCESO");
        //agregar semaforo de uno a la vez como vector
        struct pcb *proceso = obtener_copia_primer_proceso_de(SUSP_BLOCKED);
        int socket = iniciar_conexion_kernel_memoria();
        enviar_op_code(socket,SUSPENDER_PROCESO);
        t_buffer *buffer = mandar_pid_a_memoria(proceso->pid);
        crear_paquete(SUSPENDER_PROCESO,buffer,socket); //ENVIO A MEMORIA QUE QUIERO SUSPENDER EL PROCESO
        log_warning(kernel_debug_log,"PAQUETE ENVIADO A MEMORIA");
        op_code respuesta = recibir_op_code(socket);
        if(respuesta == SUSPENSION_CONFIRMADA){ //SI SE SUSPENDIO
            cerrar_conexion(socket);
            intentar_iniciar(); //INTENTAMOS INICIAR EL PLANICADOR DE MEDIANO PLAZO 
        }
    }
}

void *planificador_mediano_plazo_fifo(){
    while(1){
    sem_wait(&CANTIDAD_DE_PROCESOS_EN[SUSP_READY]);
    sem_wait(&INTENTAR_INICIAR_SUSP_READY);
    sem_wait(&UNO_A_LA_VEZ[SUSP_READY]);
    log_debug(kernel_debug_log,"ESTOY EN EL PLANI DE MEDIANO PLAZO FIFO");
    struct pcb* primer_proceso = obtener_copia_primer_proceso_de(SUSP_READY); //no lo sacamos de la lista todavia pero obtenemos una referencia
        bool respuesta = consultar_si_puede_entrar(primer_proceso,INICIALIZAR_PROCESO_SUSPENDIDO); //Darle una vuelta de tuerca para que memoria sepa que es un proceso suspendido 
        if (respuesta == true){
            primer_proceso = sacar_primero_de_la_lista(SUSP_READY); //Una vez que tenemos la confirmacion de memoria ahi si lo sacamos de la lista
            transicionar_a_ready(primer_proceso,SUSP_READY);
            sem_post(&INTENTAR_INICIAR_SUSP_READY);
            if(list_is_empty(colaEstados[SUSP_READY])){
                sem_post(&SUSP_READY_SIN_PROCESOS);
            }
        }   
        else{
            sem_post(&CANTIDAD_DE_PROCESOS_EN[SUSP_READY]);
        }
        sem_post(&UNO_A_LA_VEZ[SUSP_READY]);
    }
}

void *planificador_mediano_plazo_proceso_mas_chico_primero(){
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[SUSP_READY]);// si no hay nada espera a que llegue un proceso
        sem_wait(&INTENTAR_INICIAR_SUSP_READY);
        sem_wait(&UNO_A_LA_VEZ[SUSP_READY]);
        struct pcb* primer_proceso = obtener_copia_primer_proceso_de(SUSP_READY);
        if(primer_proceso->pid == proximo_a_consultar->pid){
            bool respuesta = consultar_si_puede_entrar(primer_proceso,INICIALIZAR_PROCESO_SUSPENDIDO);
            if(respuesta == true){
                primer_proceso = sacar_primero_de_la_lista(SUSP_READY);
                transicionar_a_ready(primer_proceso,SUSP_READY);
                sem_post(&INTENTAR_INICIAR_SUSP_READY);
                if(list_is_empty(colaEstados[SUSP_READY])){ //SI LA LISTA ESTA VACIA LE AVISAMOS AL PLANIFICADOR DE LARGO PLAZO 
                    actualizar_proximo_a_consultar(NEW); 
                    sem_post(&SUSP_READY_SIN_PROCESOS);     //QUE PUEDE INTENTAR INICIAR ALGUNO DE LOS QUE ESTAN ESPERANDO EN NEW
                }
                else{
                    actualizar_proximo_a_consultar(SUSP_READY);
                }
                sem_post(&INTENTAR_INICIAR_SUSP_READY);
            }
            else{
                sem_post(&CANTIDAD_DE_PROCESOS_EN[SUSP_READY]);
            }
        }
        else{
            sem_post(&CANTIDAD_DE_PROCESOS_EN[SUSP_READY]);
        }
        sem_post(&UNO_A_LA_VEZ[SUSP_READY]);
    }
}

//FUNCIONES PLANI MEDIANO PLAZO

void* funcion_para_bloqueados(void* arg){ //ES UN HILO QUE BLOQUEA EL PROCESO Y TE DICE SI HAY QUE SUSPENDER EL PROCESO, 
                                            // SI SE PASA DEL TIEMPO Y NO AGARRA LA IO, LO SUSPENDE
    struct pcb* proceso = (struct pcb*) arg; //lo hacemos para que pasen los argumentos al hilo.
    usleep(atoi(TIEMPO_SUSPENSION)*1000); // 
    pthread_mutex_lock(&mx_usar_cola_estado[BLOCKED]);
    int pos = buscar_en_lista(colaEstados[BLOCKED],proceso->pid);
    pthread_mutex_unlock(&mx_usar_cola_estado[BLOCKED]);
    if(pos!=-1){ //ESTA EN BLOQUEADO
        sacar_proceso_de_cola_de_estado(proceso,BLOCKED);
        cambiarEstado(proceso,BLOCKED,SUSP_BLOCKED); //ENVIO EL PROCEOS A SUSP_BLOCKED
        sem_post(&CANTIDAD_DE_PROCESOS_EN[SUSP_BLOCKED]); //AVISO QUE ALGO SE SUSPENDIO.
    }
    return NULL;
}

//FUNCIONES AUXILIARES PARA EL SJF CON DESALOJO

bool ver_si_hay_que_desalojar(struct pcb *proceso){
    pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
    frenar_y_restar_cronometros(cpus_conectadas); //freno los cronometros de los procesos que estan ejecutando
    bool desalojo = recorrer_lista_de_cpus_y_ver_si_corresponde_desalojar(cpus_conectadas,proceso); //recorro la lista de cpus para preguntar 
                                                                                                    // si los procesos que estan ejecutando alli tienen mayor estimacion que el proceso que estoy enviando
    pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
    return desalojo;
}

struct instancia_de_cpu *buscar_cpu_con_proceso_con_mayor_estimacion(){ //devuelve la cpu con su proceso con mayor estimacion.
    pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
    ordenar_lista_segun(cpus_conectadas,menor_por_estimacion_de_los_que_ya_estan_ejecutando);
    struct instancia_de_cpu *ultima_cpu = list_get(cpus_conectadas,list_size(cpus_conectadas)-1);
    pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
    return ultima_cpu;
}

void frenar_y_restar_cronometros(t_list *lista){ //freno los cronometros de todos los procesos que estan ejecutando y les resto lo que ya ejecuto.
    t_list_iterator *aux = list_iterator_create(lista); //arranca apuntando a NULL, no a donde apunta a lista
    while (list_iterator_has_next(aux)) { //es true mientras haya un siguiente al cual avanzar.
        struct instancia_de_cpu *cpu_aux = list_iterator_next(aux);
        temporal_stop(cpu_aux->proceso_ejecutando->duracion_ultima_rafaga);
        int a_restar = temporal_gettime(cpu_aux->proceso_ejecutando->duracion_ultima_rafaga); //es el tiempo que ejecuto.
        cpu_aux->proceso_ejecutando->proxima_estimacion -= a_restar; //a la estimacion le resto lo que ya ejecuto
    }
    list_iterator_destroy(aux);
    return;
}

bool menor_por_estimacion_de_los_que_ya_estan_ejecutando(void* a, void* b){ //compara las estimaciones de los procesos que ejecutan, esto es para el iterador, siempre se va a quedar con el mas grande.
    struct instancia_de_cpu* i1 = (struct instancia_de_cpu*) a;
    struct instancia_de_cpu* i2 = (struct instancia_de_cpu*) b;
    return i1->proceso_ejecutando->proxima_estimacion < i2->proceso_ejecutando->proxima_estimacion;
}

void reanudar_cronometros(t_list *lista,int iterarciones){ //reanudo los cronometros de todos los procesos que frene su ejecucion.
    t_list_iterator *aux = list_iterator_create(lista); //arranca apuntando a NULL, no a donde apunta a lista
    int cantIteraciones = 0;
    while (list_iterator_has_next(aux) && cantIteraciones < iterarciones) { //es true mientras haya un siguiente al cual avanzar.
        struct instancia_de_cpu *cpu_aux = list_iterator_next(aux);
        temporal_resume(cpu_aux->proceso_ejecutando->duracion_ultima_rafaga);
        cantIteraciones++;
    }
    return;
}

//FUNCIONES AUXILIARES

//CREACION DE PROCESOS
struct pcb* inicializar_un_proceso(struct pcb*pcb,int tamanio,char *ruta_archivo){
    pcb -> pid = identificador_del_proceso; //asigno pid
    pcb -> pc = 0; //asigno PC en 0, ya que asi lee la primera instruccion.
    pcb -> tamanio = tamanio; //tamanio enviado por terminal.
    pcb -> ruta_del_archivo_de_pseudocodigo = ruta_archivo; //ruta enviada por terminal
    pcb->ultima_estimacion = atoi(ESTIMACION_INICIAL); //asigno la estimacion inicial
    pcb->proxima_estimacion = atoi(ESTIMACION_INICIAL); //asigno la estimacion inicial
    for(int i=0; i<7; i++){ //declaro las metricas de estado, para que cada vez que cambie de estado se incrementen
        pcb->metricas_de_estado[i] = 0;
    }
    pcb->metricas_de_estado[NEW]++; //al estar inicializando el proceso, aumento la de NEW.
    pcb->metricas_de_tiempo[NEW] = temporal_create(); //creo un cronometro para saber cuanto tiempo estuvo en cada estado.
    return pcb; // retorna el proceso inicializado.
}

void incrementar_var_global_id_proceso(){ //incrementa el pid para futuros procesos.
    pthread_mutex_lock(&mx_identificador_del_proceso);
    identificador_del_proceso++; 
    pthread_mutex_unlock(&mx_identificador_del_proceso);
}

void esperar_enter_por_pantalla(){ // sirve para ingresar el enter por pantalla
    char *line;
    printf("Se esta esperando un enter por pantalla");
    do {
        line = readline("");
    } while (strlen(line) != 0); // si la longitud es mayor a 0 quiere decir que no se ingreso solo un enter
    free(line);  
}

//FUNCIONES CON LISTAS
void ordenar_lista_segun(t_list *lista,bool (*comparador)(void *, void *)){
    list_sort(lista,comparador); 
}

void insertar_ordenado_segun(t_list *lista, struct pcb *proceso, bool (*comparador)(void *, void *)) {
    list_add_sorted(lista, proceso, comparador);
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
    log_debug(kernel_debug_log,"El proceso con PID %d no se encuentra en la lista\n", pid);
    return -1;
}

int buscar_cpu_libre(t_list *lista) { //busca la cpu libre, devuelve su posicion 
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
            return pos; // devuelvo la posicion en la que se encuentra para utilizar el list get 
        }
        pos++;
    }
    return -1;
}

struct instancia_de_cpu *obtener_cpu(int posicion){ //me devuelve la cpu libre gracias a que obtuve en la anterior funcion la posicion.
    pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
    struct instancia_de_cpu *cpu_aux = list_get(cpus_conectadas,posicion);
    pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
    return cpu_aux;
}


bool recorrer_lista_de_cpus_y_ver_si_corresponde_desalojar(t_list *lista,struct pcb *proceso) {
    if (lista == NULL) { //no deberia pasar nunca porque esta sincronizado pero por ahora lo dejamos
        printf("Lista nula\n");
    return false;
    }
    t_list_iterator *aux = list_iterator_create(lista); //arranca apuntando a NULL, no a donde apunta a lista
    while (list_iterator_has_next(aux)) { //es true mientras haya un siguiente al cual avanzar.
        struct instancia_de_cpu *cpu_aux = list_iterator_next(aux); //me voy fijando las cpus con sus respectivos procesos
        if (cpu_aux->proceso_ejecutando->proxima_estimacion > proceso->proxima_estimacion) { /*Si el proceso que esta ejecutando tiene mayor 
                                                                                                tiene mayor estimacion que el proceso nuevo*/ 
            list_iterator_destroy(aux);  //destruyo el iterador
            return true; // corresponde desalojar
        }
    }
    list_iterator_destroy(aux); // si llega aca, es por que no corresponde desalojar.
    return false;
}

//LISTAS DE ESTADOS

struct pcb *obtener_copia_primer_proceso_de(Estado estado){ //devuelve una copia del primer proceso sin tener que sacarlo
    if(list_is_empty(colaEstados[estado])){
        return NULL;
    }
    else{
    pthread_mutex_lock(&mx_usar_cola_estado[estado]); // es una variable global asi que la protegemos (mejor un mx)
    struct pcb *proceso = list_get(colaEstados[estado], 0);  // 
    pthread_mutex_unlock(&mx_usar_cola_estado[estado]); // es una variable global asi que la protegemos (mejor un mx)
    return proceso;
    }
}

struct pcb *sacar_primero_de_la_lista(Estado estado){
    struct pcb *aux;
    pthread_mutex_lock(&mx_usar_cola_estado[estado]);
    aux = list_remove(colaEstados[estado],0); //como el list add agrerga al final, sacamos del principio para no romper el comportamiento de la cola
    pthread_mutex_unlock(&mx_usar_cola_estado[estado]);
    return aux;
}

//CAMBIOS DE ESTADO

void enviar_proceso_a_memoria(struct pcb* proceso){ //aviso a memoria que quiero enviar el proceso 
    int socket = iniciar_conexion_kernel_memoria(); 
    t_buffer* buffer = crear_buffer_de_envio_de_proceso(proceso->pid,proceso->ruta_del_archivo_de_pseudocodigo,proceso->tamanio);
    crear_paquete(INICIALIZAR_PROCESO_DESDE_NEW, buffer, socket);
    cerrar_conexion(socket);
}

void transicionar_a_new(struct pcb *pcb){
 pthread_mutex_lock(&mx_usar_cola_estado[NEW]); 
    if(strcmp(ALGORITMO_INGRESO_A_READY,"FIFO")==0){ //si el algoritmo es FIFO
        list_add(colaEstados[NEW],pcb); //aniado de forma normal a la cola de NEW.
    }else{
        insertar_ordenado_segun(colaEstados[NEW],pcb,menor_por_tamanio); // inserto ordenado a la cola de NEW por menor tamanio.
        pthread_mutex_lock(&mx_proximo_a_consultar);
        proximo_a_consultar = pcb; //actualizo el proximo a consultar
        pthread_mutex_unlock(&mx_proximo_a_consultar);
        sem_post(&INTENTAR_INICIAR_NEW);
    }
    pthread_mutex_unlock(&mx_usar_cola_estado[NEW]);
    sem_post(&CANTIDAD_DE_PROCESOS_EN[NEW]);
}

void transicionar_a_ready(struct pcb *pcb,Estado estadoInicial){
    //pthread_mutex_lock(&mx_usar_cola_estado[READY]); 
    if(strcmp(ALGORITMO_CORTO_PLAZO,"FIFO")==0){
        cambiarEstado(pcb,estadoInicial,READY); //cambio de estado al proceso
    }else{
        if(strcmp(ALGORITMO_CORTO_PLAZO,"SJF_SIN_DESALOJO")==0){
            cambiarEstadoOrdenado(pcb,estadoInicial,READY,menor_por_estimacion); //cambio de estado ordenadamente
        }
        else{
        cambiarEstadoOrdenado(pcb,estadoInicial,READY,menor_por_estimacion);
        sem_post(&REPLANIFICAR); //si el algoritmo tiene DESALOJO, aviso que hay que replanificar.
        }
     }
    //pthread_mutex_unlock(&mx_usar_cola_estado[READY]);
    sem_post(&CANTIDAD_DE_PROCESOS_EN[READY]);
}

void transicionar_a_susp_ready(struct pcb *pcb){
    //pthread_mutex_lock(&mx_usar_cola_estado[SUSP_READY]); 
    if(strcmp(ALGORITMO_INGRESO_A_READY,"FIFO")==0){ 
        cambiarEstado(pcb,SUSP_BLOCKED,SUSP_READY);
    }
    else{
        pthread_mutex_lock(&mx_proximo_a_consultar);
        proximo_a_consultar = pcb;
        pthread_mutex_unlock(&mx_proximo_a_consultar);
        cambiarEstadoOrdenado(pcb,SUSP_BLOCKED,SUSP_READY,menor_por_tamanio);
        sem_post(&INTENTAR_INICIAR_SUSP_READY);
     }
    //pthread_mutex_unlock(&mx_usar_cola_estado[SUSP_READY]);
    sem_post(&CANTIDAD_DE_PROCESOS_EN[SUSP_READY]);
}

void cambiarEstado (struct pcb* proceso,Estado estadoAnterior, Estado estadoNuevo){
    char *string_estado_nuevo = cambiar_a_string(estadoNuevo);
    char *string_estado_anterior = cambiar_a_string(estadoAnterior);
    log_info(kernel_logger,"## (<PID: %i>) Pasa del estado <%s> al estado <%s>", proceso->pid,string_estado_anterior,string_estado_nuevo);
    pthread_mutex_unlock(&mx_usar_cola_estado[estadoNuevo]);
    list_add(colaEstados[estadoNuevo],proceso); 
    pthread_mutex_unlock(&mx_usar_cola_estado[estadoNuevo]);
    proceso->metricas_de_estado[estadoNuevo]++;
    gestionar_metrica_de_tiempo(proceso,estadoAnterior,estadoNuevo);
}

void cambiarEstadoOrdenado(struct pcb* proceso,Estado estadoAnterior, Estado estadoNuevo,bool (*comparador)(void *, void *)){
    log_debug(kernel_debug_log,"CAMBIANDO ESTADO ORDENADAMENTE");
    char *string_estado_nuevo = cambiar_a_string(estadoNuevo);
    char *string_estado_anterior = cambiar_a_string(estadoAnterior);
    log_info(kernel_logger,"## (<PID: %i>) Pasa del estado <%s> al estado <%s>", proceso->pid,string_estado_anterior,string_estado_nuevo);
    pthread_mutex_unlock(&mx_usar_cola_estado[estadoNuevo]);
    list_add_sorted(colaEstados[estadoNuevo],proceso,comparador); 
    pthread_mutex_unlock(&mx_usar_cola_estado[estadoNuevo]);
    proceso->metricas_de_estado[estadoNuevo]++;
    gestionar_metrica_de_tiempo(proceso,estadoAnterior,estadoNuevo);
}

void gestionar_metrica_de_tiempo(struct pcb* proceso, Estado estadoInicial, Estado estadoNuevo){
    temporal_stop(proceso->metricas_de_tiempo[estadoInicial]);
    if(proceso->metricas_de_estado[estadoNuevo]==1){ //osea que es la primera vez que pasa por cierto estado
    proceso->metricas_de_tiempo[estadoNuevo] = temporal_create();
    }
    else{
        temporal_resume(proceso->metricas_de_tiempo[estadoNuevo]);
    }
    
}

char *cambiar_a_string(Estado estado) {
    char* nombres_estados[] = { 
        "NEW",
        "READY",
        "BLOCKED",
        "EXEC",
        "EXIT_ESTADO",
        "SUSP_READY",
        "SUSP_BLOCKED"
    };

    if (estado < 0 || estado > SUSP_BLOCKED) {
        return "ESTADO_DESCONOCIDO";
    }

    return nombres_estados[estado];
}


bool consultar_si_puede_entrar(struct pcb *proceso,op_code operacion){
    int socket = iniciar_conexion_kernel_memoria(); //inicio la conexion de me
    enviar_op_code(socket,operacion); 
    bool respuesta = solicitar_permiso_a_memoria(socket,proceso,operacion); //solicito el permiso
    cerrar_conexion(socket);
    return respuesta;
}

void actualizar_proximo_a_consultar(Estado estadoInicial){
    struct pcb *primer_proceso_actualizado = obtener_copia_primer_proceso_de(estadoInicial);
    if(primer_proceso_actualizado!=NULL){
        pthread_mutex_lock(&mx_proximo_a_consultar);
        proximo_a_consultar = primer_proceso_actualizado;
        pthread_mutex_unlock(&mx_proximo_a_consultar);
    }
}

//CALCULAR PROXIMA ESTIMACION

float calcular_proxima_estimacion(struct pcb *proceso){
    float prox_estimacion;
    long ultima_rafaga = temporal_gettime(proceso->duracion_ultima_rafaga);
    float alfa = (float)atof(ALFA);
    prox_estimacion = alfa * ultima_rafaga + (1-alfa)*proceso->ultima_estimacion;
    proceso->ultima_estimacion = prox_estimacion;
    log_info(kernel_logger,"La proxima estimacion del proceso (< PID: %i>) es: %f", proceso ->pid,proceso->ultima_estimacion);
    temporal_destroy(proceso->duracion_ultima_rafaga);
    proceso->duracion_ultima_rafaga = NULL;
    return prox_estimacion;
}


//COMPARADORES

bool menor_por_estimacion(void* a, void* b){
    struct pcb* p1 = (struct pcb*) a;
    struct pcb* p2 = (struct pcb*) b;
    return p1->proxima_estimacion < p2->proxima_estimacion;
}

bool menor_por_tamanio(void* a, void* b) {
    
    struct pcb* p1 = (struct pcb*) a;
    struct pcb* p2 = (struct pcb*) b;
    return p1->tamanio < p2->tamanio;
}

//MANEJO DE SYSCALLS

void sacar_proceso_de_cola_de_estado(struct pcb *proceso,Estado estado){
    pthread_mutex_lock(&mx_usar_cola_estado[estado]);
    int pos = buscar_en_lista(colaEstados[estado],proceso->pid);
    list_remove(colaEstados[estado],pos);
    pthread_mutex_unlock(&mx_usar_cola_estado[estado]);
}

void mandar_paquete_a_cpu(struct pcb *proceso,struct instancia_de_cpu *cpu){
    t_buffer *buffer = crear_buffer_cpu(proceso->pid,proceso->pc);
    crear_paquete(ENVIO_PID_Y_PC,buffer,cpu->socket_para_comunicarse); //esta funcion crea el paquete y tambien lo envia
}

op_code manejar_dump(struct pcb *aux,struct instancia_de_cpu* cpu_en_la_que_ejecuta){
    log_debug(kernel_debug_log,"ESTOY ACA POR EL DUMP");
    cambiarEstado(aux,EXEC,BLOCKED);
    //sem_post(&CANTIDAD_DE_PROCESOS_EN[BLOCKED]);
    int socket = iniciar_conexion_kernel_memoria();
    enviar_op_code(socket,DUMP_MEMORY);
    t_buffer *buffer = mandar_pid_a_memoria(aux->pid);
    crear_paquete(DUMP_MEMORY,buffer,socket);
    liberar_cpu(cpu_en_la_que_ejecuta);
    op_code respuesta = recibir_op_code(socket);
    return respuesta;
}

void crear_hilo_de_ejecucion(struct pcb *proceso, struct instancia_de_cpu *cpu_aux){
    pthread_t hilo_ejecucion; //declaro el hilo
    struct parametros_de_ejecucion *parametros = malloc(sizeof(struct parametros_de_ejecucion)); // creo la estructura de los parametros
    parametros->proceso = proceso; //asigno el proceso
    parametros->cpu_aux = cpu_aux; //asigno la cpu
    pthread_create(&hilo_ejecucion,NULL,poner_a_ejecutar,(void *)parametros); //creo el hilo de ejecucion y le asigno los parametros
    pthread_detach(hilo_ejecucion); // hago un detach del hilo
}
         
void *poner_a_ejecutar(void *argumentos){
    struct parametros_de_ejecucion* args = (struct parametros_de_ejecucion*) argumentos; 
    struct pcb* proceso = args->proceso;
    struct instancia_de_cpu* cpu_en_la_que_ejecuta = args->cpu_aux;
    free(argumentos);
    cpu_en_la_que_ejecuta->proceso_ejecutando = proceso; //asigno un proceso a la cpu
    proceso->duracion_ultima_rafaga = temporal_create(); //creo una tiempo de rafaga en el proceso
    bool bloqueante = false; 
    while(!bloqueante){ //mientras el bloqueante sea falso.
        recibir_op_code(cpu_en_la_que_ejecuta->socket_para_comunicarse); //cpu con la que se comunica
        mandar_paquete_a_cpu(proceso,cpu_en_la_que_ejecuta); //mando paquete a cpu, le mando pid y pc del proceso
        t_paquete *paquete = recibir_paquete(cpu_en_la_que_ejecuta->socket_para_comunicarse); //cpu ejecuta una instruccion y nos devuelve el pid con una syscalL
        proceso->pc = deserializar_pc(paquete);
        op_code motivo_de_devolucion = obtener_codigo_de_operacion(paquete); //deserializa el opcode del paquete
        if(motivo_de_devolucion!=DESALOJO_ACEPTADO){
            char *syscall = cambiar_syscall_a_string(motivo_de_devolucion);
            log_info(kernel_logger,"## (<PID: %i>) - Solicitó syscall: <%s>",proceso->pid,syscall);
        }
        switch(motivo_de_devolucion){
            case DESALOJO_ACEPTADO:
                temporal_stop(proceso->duracion_ultima_rafaga);
                log_info(kernel_logger,"## (<PID: %i>) - DESALOJADO POR ALGORITMO SJF/SRT",proceso->pid);
                liberar_cpu(cpu_en_la_que_ejecuta);
                proceso->proxima_estimacion = calcular_proxima_estimacion(proceso);
                desalojar_proceso_de_cpu(proceso);
                enviar_op_code(cpu_en_la_que_ejecuta->socket_para_comunicarse,DESALOJO_REALIZADO);
                bloqueante = true;
                break;
            case INIT_PROC:
                char *nombre_archivo = deserializar_nombre_archivo_init_proc(paquete);
                int tamanio = deserializar_tamanio(paquete);
                crear_proceso(tamanio,nombre_archivo);
                proceso->pc++;
                enviar_op_code(cpu_en_la_que_ejecuta->socket_para_comunicarse,SYSCALL_EJECUTADA);
                //avisar que termine
                break;
            case EXIT:
                liberar_paquete(paquete);
                //Hay que sacarlo de la lista de exit
                finalizar_proceso(proceso,EXEC);
                liberar_cpu(cpu_en_la_que_ejecuta);
                enviar_op_code(cpu_en_la_que_ejecuta->socket_para_comunicarse,SYSCALL_EJECUTADA);
                //tener en cuenta lo del mediano plazo
                bloqueante = true;
                break;
            case DUMP_MEMORY:
                liberar_paquete(paquete);
                temporal_stop(proceso->duracion_ultima_rafaga);
                log_error(kernel_debug_log, "ESTOY ACA");
                proceso->pc++;
                sacar_proceso_de_cola_de_estado(proceso,EXEC);
                int respuesta = manejar_dump(proceso,cpu_en_la_que_ejecuta); //esta funcion manda el proceso a BLOCKED y tambien libera la cpu
                if(respuesta == DUMP_ACEPTADO){
                    if(strcmp(ALGORITMO_CORTO_PLAZO,"FIFO")!=0){
                    proceso->proxima_estimacion = calcular_proxima_estimacion(proceso);
                    }
                    sacar_proceso_de_cola_de_estado(proceso,BLOCKED);
                    transicionar_a_ready(proceso,BLOCKED);
                }
                else{
                    finalizar_proceso(proceso,BLOCKED);
                }
                enviar_op_code(cpu_en_la_que_ejecuta->socket_para_comunicarse,SYSCALL_EJECUTADA);
                bloqueante = true; 
                break;   
            case IO:
                proceso->pc++;
                int milisegundos = deserializar_cant_segundos(paquete);
                proceso->proxima_rafaga_io = milisegundos;
                proceso->nombre_io_que_lo_bloqueo = deserializar_nombre_syscall_io(paquete);
                pthread_mutex_lock(&mx_usar_recurso[REC_IO]);
                int pos = buscar_IO_solicitada(ios_conectados,proceso->nombre_io_que_lo_bloqueo);
                pthread_mutex_unlock(&mx_usar_recurso[REC_IO]);
                log_debug(kernel_debug_log,"TERMINE DE BUSCAR EL NOMBRE");
                if(pos == -1){ //quiere decir que no hay ninguna syscall con ese nombre
                    log_warning(kernel_debug_log,"adentro del IF de io");
                    finalizar_proceso(proceso,EXEC);
                    liberar_cpu(cpu_en_la_que_ejecuta);
                }
                else{
                    log_info(kernel_logger,"## (<PID: %i>) - Bloqueado por IO: <%s>",proceso->pid,proceso->nombre_io_que_lo_bloqueo);
                    temporal_stop(proceso->duracion_ultima_rafaga);
                    liberar_cpu(cpu_en_la_que_ejecuta);
                    proceso->proxima_estimacion = calcular_proxima_estimacion(proceso); 
                    sacar_proceso_de_cola_de_estado(proceso,EXEC);
                    cambiarEstado(proceso,EXEC,BLOCKED); //BLOQUEAMOS EL PROCESO, 
                    //creamos el hilo para que bloquee el proceso
                    pthread_create(&proceso->hilo_al_bloquearse,NULL,funcion_para_bloqueados,proceso);
                    pthread_detach(proceso->hilo_al_bloquearse);
                    //Inicializamos el cronometro de bloqueado
                    pthread_mutex_lock(&mx_usar_recurso[REC_IO]);
                    struct instancia_de_io *io_aux = list_get(ios_conectados,pos);
                    pthread_mutex_unlock(&mx_usar_recurso[REC_IO]);
                    sem_post(io_aux->hay_procesos_esperando); //AVISO QUE HAY UNA IO ESPERANDO A EJECUTAR
                    log_debug(kernel_debug_log, "TERMINE DE HACER EL ELSE");
                }
                bloqueante = true;
                break;
            default:
                log_debug(kernel_debug_log,"SYSCALL DESCONOCIDA");
                break;
        }
    
    }
    return NULL;
}

struct pcb* buscar_proceso_bloqueado_por_io(t_list *lista, char *nombre){
    
    if(!lista){ //no deberia pasar nunca porque esta sincronizado pero por ahora lo dejamos
        printf("Lista nula\n");
    return NULL;
    }
    t_list_iterator *iterador = list_iterator_create(lista); //arranca apuntando a NULL, no a donde apunta a lista
    while (list_iterator_has_next(iterador)) { //es true mientras haya un siguiente al cual avanzar.
        struct pcb *proceso = list_iterator_next(iterador);
        if (strcmp(proceso->nombre_io_que_lo_bloqueo,nombre)==0) { // comparo al pid que estoy apuntando con el pid que busco.
            proceso->nombre_io_que_lo_bloqueo = ""; //esto se hace para que si algun otro proceso llama a la funcion este deje de estar disponible
            list_iterator_destroy(iterador); //delete del iterador.
            return proceso;
        }
    }
    list_iterator_destroy(iterador);
    return NULL;
}

void liberar_cpu(struct instancia_de_cpu *cpu){
    cpu->puede_usarse = true;
    cpu->proceso_ejecutando = NULL;
    sem_post(&CPUS_LIBRES);
    if(strcmp(ALGORITMO_CORTO_PLAZO,"SJF_CON_DESALOJO")==0){
        sem_post(&REPLANIFICAR);
    }
}

void desalojar_proceso_de_cpu(struct pcb *proceso_desalojado){
    sacar_proceso_de_cola_de_estado(proceso_desalojado,EXEC);
    log_debug(kernel_debug_log,"Se desaloja de la cola EXECUTE al proceso con id: %i",proceso_desalojado->pid);
    cambiarEstadoOrdenado(proceso_desalojado,EXEC,READY,menor_por_estimacion);
    //transicionar_a_ready(proceso_desalojado,EXEC);
}

void finalizar_proceso(struct pcb *proceso, Estado estadoInicial){
    intentar_iniciar();
    sacar_proceso_de_cola_de_estado(proceso,estadoInicial);
    cambiarEstado(proceso,estadoInicial,EXIT_ESTADO);
    int socket = iniciar_conexion_kernel_memoria();
    t_buffer *buffer = mandar_pid_a_memoria(proceso->pid);
    enviar_op_code(socket,FINALIZAR_PROCESO);
    crear_paquete(FINALIZAR_PROCESO,buffer,socket);
    op_code confirmacion = recibir_op_code(socket);
    cerrar_conexion(socket);
    sacar_proceso_de_cola_de_estado(proceso,EXIT_ESTADO);   
    listar_metricas_de_tiempo_y_estado(proceso); 
    log_info(kernel_logger,"## (<PID: %i>) - Finaliza el proceso",proceso->pid);
    liberar_proceso(proceso); //free de todos los punteros, lo demas se va con el free (proceso) 
}

void listar_metricas_de_tiempo_y_estado(struct pcb *proceso){
    Estado estado_actual;
    for(int i=0; i<7; i++){
        estado_actual = i;
        char *estado_string = cambiar_a_string(estado_actual);
        if(proceso->metricas_de_estado[i]!=0){ //que haya estado alguna vez en ese estado
        log_warning(kernel_logger,"## (<PID: %i>) - Metricas de estado: %s <cantidad: %i> <tiempo: %ld>",proceso->pid,estado_string,proceso->metricas_de_estado[i],temporal_gettime(proceso->metricas_de_tiempo[i]));
        }
    }
}

void liberar_proceso(struct pcb *proceso){
    temporal_destroy(proceso->duracion_ultima_rafaga);
    for (int i=0; i<7;i++){
        if(proceso->metricas_de_estado[i]!=0){
        temporal_destroy(proceso->metricas_de_tiempo[i]);
        }
    }
    free(proceso);
}

void intentar_iniciar(){
    if(!list_is_empty(colaEstados[SUSP_READY])){
        if(strcmp(ALGORITMO_INGRESO_A_READY,"PMCP")==0){
            actualizar_proximo_a_consultar(SUSP_READY);
        }
        sem_post(&INTENTAR_INICIAR_SUSP_READY);
    }
    else{
        if(strcmp(ALGORITMO_INGRESO_A_READY,"PMCP")==0){
            actualizar_proximo_a_consultar(NEW);
        }
        sem_post(&INTENTAR_INICIAR_NEW);
    }
}

char *cambiar_syscall_a_string(op_code syscall){
    switch(syscall){
        case EXIT:
            return "EXIT";
        case IO:
            return "IO";
        case DUMP_MEMORY:
            return "DUMP MEMORY";
        case INIT_PROC:
            return "INIT_PROC";
        default:
            return "Syscall Desconocida";
    }
} 

void liberar_paquete(t_paquete *paquete){
   free(paquete->buffer->stream);
   free(paquete->buffer);
   free(paquete);
}