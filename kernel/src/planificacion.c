#include <planificacion.h>
#include <readline/readline.h>
#include <string.h>

t_list* procesos = NULL;
t_list* colaEstados[7]={NULL};
int identificador_del_proceso = 0;
struct pcb *proximo_a_consultar;


void crear_proceso(int tamanio,char *ruta_archivo) { // tambien tiene que recibir el tamanio y el path
    struct pcb* pcb = malloc(sizeof(struct pcb)); 
    pcb = inicializar_un_proceso(pcb,tamanio,ruta_archivo); 
    transicionar_a_new(pcb); //aca esta lo de ultimo proceso en entrar
    log_info(kernel_logger,"## (< PID: %i>) Se crea el proceso - ESTADO: NEW",pcb->pid);
    incrementar_var_global_id_proceso();
    return; 
}

void *planificador_largo_plazo_fifo(){
    esperar_enter_por_pantalla();
    log_debug(kernel_debug_log,"INICIANDO PLANIFICADOR DE LARGO PLAZO");
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[NEW]); // si no hay nada espera a que llegue un proceso
        sem_wait(&INTENTAR_INICIAR_NEW); //que los demas esperen a que uno entre
        sem_wait(&UNO_A_LA_VEZ[NEW]); //este semaforo hace que el planificador maneje un proceso a la vez.
        if(!list_is_empty(colaEstados[SUSP_READY])){
            sem_wait(&SUSP_READY_SIN_PROCESOS);
        }
        struct pcb* primer_proceso = obtener_copia_primer_proceso_de(NEW); //no lo sacamos de la lista todavia pero obtenemos una referencia
        bool respuesta = consultar_si_puede_entrar(primer_proceso,INICIALIZAR_PROCESO_DESDE_NEW);
        log_debug(kernel_debug_log,"Conexion con memoria cerrada");
        if(respuesta){
            primer_proceso = sacar_primero_de_la_lista(NEW); //Una vez que tenemos la confirmacion de memoria ahi si lo sacamos de la cola de NEW
            transicionar_a_ready(primer_proceso,NEW);
            sem_post(&INTENTAR_INICIAR_NEW); //incremento el semaforo para que pueda iniciar el planificador.
        }   
        else{
            log_debug(kernel_debug_log,"NO HAY ESPACIO SUFICIENTE EN MEMORIA");
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
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[NEW]);// si no hay nada espera a que llegue un proceso
        sem_wait(&INTENTAR_INICIAR_NEW); //que los demas esperen a que uno entre
        sem_wait(&UNO_A_LA_VEZ[NEW]); //este semaforo hace que el planificador maneje un proceso a la vez.
        //verifico que la cola de SUSP_READY este vacía.
        if(!list_is_empty(colaEstados[SUSP_READY])){
            sem_wait(&SUSP_READY_SIN_PROCESOS); //espero a que se vacíe la lista de susp ready.
        }
        struct pcb* primer_proceso = obtener_copia_primer_proceso_de(NEW); 
        if(primer_proceso->pid == proximo_a_consultar->pid){ 
            bool respuesta = consultar_si_puede_entrar(primer_proceso,INICIALIZAR_PROCESO_DESDE_NEW);
            log_debug(kernel_debug_log,"Conexion con memoria cerrada");
            if(respuesta){
                primer_proceso = sacar_primero_de_la_lista(NEW); //saco el primer proceso de la lista de NEW
                transicionar_a_ready(primer_proceso,NEW); // lo transiciono a READY
                actualizar_proximo_a_consultar(NEW); //actualizo la lista de NEW
                if(!list_is_empty(colaEstados[NEW])){
                    sem_post(&INTENTAR_INICIAR_NEW); //intento iniciar nuevamente la lista de NEW
                }
            }
            else{
                log_debug(kernel_debug_log,"NO HAY ESPACIO SUFICIENTE EN MEMORIA");
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
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[READY]); //espero a que llegue un proceso a ready.
        sem_wait(&CPUS_LIBRES); //espero a que haya cpus libres.
        log_debug(kernel_debug_log,"PLANI DE CORTO PLAZO INICIADO");
        pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
        int pos_cpu = buscar_cpu_libre(cpus_conectadas); 
        struct instancia_de_cpu *cpu_aux = list_get(cpus_conectadas,pos_cpu);
        log_debug(kernel_debug_log,"La cpu libre es: %i",cpu_aux->id_cpu);
        pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
        struct pcb* proceso = sacar_primero_de_la_lista(READY);
        cambiarEstado(proceso,READY,EXEC);
        log_debug(kernel_debug_log,"El proceso %i pasa a ejecutar en la cpu %i",proceso->pid,cpu_aux->id_cpu);
        crear_hilo_de_ejecucion(proceso,cpu_aux);
    }
}   

void *planificador_corto_plazo_sjf_sin_desalojo(){
      while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[READY]);
        sem_wait(&CPUS_LIBRES);
        //sem_wait(&INGRESO_DEL_PRIMERO_READY); //Preguntar si es necesario pero creeriamos que con el de cantProcesos y el de las cpus ya esta
        //usleep(3000000); 
        log_debug(kernel_debug_log,"PLANI DE CORTO PLAZO INICIADO");
        pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
        int pos_cpu = buscar_cpu_libre(cpus_conectadas);
        struct instancia_de_cpu *cpu_aux = list_get(cpus_conectadas,pos_cpu);
        pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
        struct pcb* proceso = sacar_primero_de_la_lista(READY);
        cambiarEstado(proceso,READY,EXEC);
        log_debug(kernel_debug_log,"El proceso %i pasa a ejecutar en la cpu %i",proceso->pid,cpu_aux->id_cpu);
        crear_hilo_de_ejecucion(proceso,cpu_aux);
        //poner_a_ejecutar(proceso,cpu_aux);
    }
}

void *planificador_corto_plazo_sjf_con_desalojo(){
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[READY]);
        sem_wait(&CPUS_LIBRES); 
        sem_wait(&REPLANIFICAR);
        //usleep(200000);
        pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
        int pos_cpu = buscar_cpu_libre(cpus_conectadas);
        pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
        struct pcb *proceso = obtener_copia_primer_proceso_de(READY);
        if(pos_cpu!=-1){ //Quiere decir que hay una cpu libre, seria "el caso facil"
            struct instancia_de_cpu *cpu_aux = obtener_cpu(pos_cpu);
            proceso = sacar_primero_de_la_lista(READY);
            cambiarEstado(proceso,READY,EXEC);
            crear_hilo_de_ejecucion(proceso,cpu_aux); //creamos un hilo de ejecucion enviando los parametros
        }
        else{
            bool desalojo = ver_si_hay_que_desalojar(proceso);
            if(desalojo){
                //cambiar todo para que haya alguna funcion que te devuelva la cpu y no la posicion
                struct instancia_de_cpu *cpu_aux = buscar_cpu_con_proceso_con_mayor_estimacion();
                enviar_entero(cpu_aux->socket_interrupt,1);
                pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
                reanudar_cronometros(cpus_conectadas,list_size(cpus_conectadas)-1);
                pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
                sem_post(&CANTIDAD_DE_PROCESOS_EN[READY]); //Porque todavia no desalojamos nada, simplemente dimos el aviso
            }
            else{
            log_debug(kernel_debug_log,"No se desaloja");
            pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
            reanudar_cronometros(cpus_conectadas,list_size(cpus_conectadas));
            pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
            sem_post(&CANTIDAD_DE_PROCESOS_EN[READY]);
            }
        }
        sem_post(&CPUS_LIBRES); // es para que no se bloquee, pero hay que preguntar en el soporte si las cpus se conectan primero
    }
}

void *planificador_mediano_plazo(){
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[SUSP_BLOCKED]);
        log_warning(kernel_debug_log,"SE SUSPENDIO UN PROCESO");
        intentar_iniciar();
        //agregar semaforo de uno a la vez como vector
        struct pcb *proceso = obtener_copia_primer_proceso_de(SUSP_BLOCKED);
        int socket = iniciar_conexion_kernel_memoria();
        enviar_op_code(socket,SUSPENDER_PROCESO);
        t_buffer *buffer = mandar_pid_a_memoria(proceso->pid);
        crear_paquete(SUSPENDER_PROCESO,buffer,socket);
        log_warning(kernel_debug_log,"PAQUETE ENVIADO A MEMORIA");
        recibir_op_code(socket);
        cerrar_conexion(socket);
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
        log_debug(kernel_debug_log,"Conexion con memoria cerrada");
        if (respuesta == true){
            primer_proceso = sacar_primero_de_la_lista(SUSP_READY); //Una vez que tenemos la confirmacion de memoria ahi si lo sacamos de la lista
            transicionar_a_ready(primer_proceso,SUSP_READY);
            sem_post(&INTENTAR_INICIAR_SUSP_READY);
            if(list_is_empty(colaEstados[SUSP_READY])){
                sem_post(&SUSP_READY_SIN_PROCESOS);
            }
        }   
        else{
            log_debug(kernel_debug_log,"NO HAY ESPACIO SUFICIENTE EN MEMORIA");
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
            log_debug(kernel_debug_log,"Conexion con memoria cerrada");
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

void* funcion_para_bloqueados(void* arg){
    struct pcb* proceso = (struct pcb*) arg; //lo hacemos para que pasen los argumentos al hilo.
    usleep(atoi(TIEMPO_SUSPENSION)*1000); //preguntar
    log_warning(kernel_debug_log,"TERMINO EL TIEMPO DE SUSPENSION");
    pthread_mutex_lock(&mx_usar_cola_estado[BLOCKED]);
    int pos = buscar_en_lista(colaEstados[BLOCKED],proceso->pid);
    pthread_mutex_unlock(&mx_usar_cola_estado[BLOCKED]);
    if(pos!=-1){ //ESTA EN BLOQUEADO
        log_warning(kernel_debug_log,"ADENTRO DEL IF DE FUNCION PARA BLOQUEADOS");
        sacar_proceso_de_cola_de_estado(proceso,BLOCKED);
        cambiarEstado(proceso,BLOCKED,SUSP_BLOCKED); //A chequear
        sem_post(&CANTIDAD_DE_PROCESOS_EN[SUSP_BLOCKED]);
        log_warning(kernel_debug_log,"Sem post hecho");
    }
    return NULL;
}

//FUNCIONES AUXILIARES PARA EL SJF CON DESALOJO

bool ver_si_hay_que_desalojar(struct pcb *proceso){
    pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
    frenar_y_restar_cronometros(cpus_conectadas);
    bool desalojo = recorrer_lista_de_cpus_y_ver_si_corresponde_desalojar(cpus_conectadas,proceso);
    pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
    return desalojo;
}

struct instancia_de_cpu *buscar_cpu_con_proceso_con_mayor_estimacion(){
    pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
    ordenar_lista_segun(cpus_conectadas,menor_por_estimacion_de_los_que_ya_estan_ejecutando);
    struct instancia_de_cpu *ultima_cpu = list_get(cpus_conectadas,list_size(cpus_conectadas)-1);
    pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
    return ultima_cpu;
}

void frenar_y_restar_cronometros(t_list *lista){
    t_list_iterator *aux = list_iterator_create(lista); //arranca apuntando a NULL, no a donde apunta a lista
    while (list_iterator_has_next(aux)) { //es true mientras haya un siguiente al cual avanzar.
        struct instancia_de_cpu *cpu_aux = list_iterator_next(aux);
        temporal_stop(cpu_aux->proceso_ejecutando->duracion_ultima_rafaga);
        int a_restar = temporal_gettime(cpu_aux->proceso_ejecutando->duracion_ultima_rafaga);
        cpu_aux->proceso_ejecutando->proxima_estimacion -= a_restar; //a la estimacion le resto lo que ya ejecuto
    }
    list_iterator_destroy(aux);
    return;
}

bool menor_por_estimacion_de_los_que_ya_estan_ejecutando(void* a, void* b){
    struct instancia_de_cpu* i1 = (struct instancia_de_cpu*) a;
    struct instancia_de_cpu* i2 = (struct instancia_de_cpu*) b;
    return i1->proceso_ejecutando->proxima_estimacion < i2->proceso_ejecutando->proxima_estimacion;
}

void reanudar_cronometros(t_list *lista,int iterarciones){
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
    pcb -> pid = identificador_del_proceso; //ver lo de la variable global
    pcb -> pc = 0;
    pcb -> tamanio = tamanio;
    pcb -> ruta_del_archivo_de_pseudocodigo = ruta_archivo;
    pcb->ultima_estimacion = atoi(ESTIMACION_INICIAL);
    pcb->proxima_estimacion = atoi(ESTIMACION_INICIAL);
    for(int i=0; i<7; i++){
        pcb->metricas_de_estado[i] = 0;
    }
    pcb->metricas_de_estado[NEW]++;
    pcb->metricas_de_tiempo[NEW] = temporal_create();
    return pcb;
}

void incrementar_var_global_id_proceso(){
    pthread_mutex_lock(&mx_identificador_del_proceso);
    identificador_del_proceso++; 
    pthread_mutex_unlock(&mx_identificador_del_proceso);
}

void esperar_enter_por_pantalla(){
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

struct instancia_de_cpu *obtener_cpu(int posicion){
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
        struct instancia_de_cpu *cpu_aux = list_iterator_next(aux);
        if (cpu_aux->proceso_ejecutando->proxima_estimacion > proceso->proxima_estimacion) { // comparo al pid que estoy apuntando con el pid que busco.
            list_iterator_destroy(aux); //delete del iterador.
            return true;
        }
    }
    return false;
}

//LISTAS DE ESTADOS

struct pcb *obtener_copia_primer_proceso_de(Estado estado){
    if(list_is_empty(colaEstados[estado])){
        return NULL;
    }
    else{
    pthread_mutex_lock(&mx_usar_cola_estado[estado]); // es una variable global asi que la protegemos (mejor un mx)
        t_list *aux = colaEstados[estado];
        struct pcb *proceso = list_get(aux, 0);  // Obtener el primer elemento pero sin sacarlo de la lista todavia
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

void enviar_proceso_a_memoria(struct pcb* proceso){
    int socket = iniciar_conexion_kernel_memoria();
    t_buffer* buffer = crear_buffer_de_envio_de_proceso(proceso->pid,proceso->ruta_del_archivo_de_pseudocodigo,proceso->tamanio);
    crear_paquete(INICIALIZAR_PROCESO_DESDE_NEW, buffer, socket);
    cerrar_conexion(socket);
}

void transicionar_a_new(struct pcb *pcb){
 pthread_mutex_lock(&mx_usar_cola_estado[NEW]); 
    if(strcmp(ALGORITMO_INGRESO_A_READY,"FIFO")==0){
        list_add(colaEstados[NEW],pcb); // es una variable global asi que habria que poner un mutex
    }else{
        insertar_ordenado_segun(colaEstados[NEW],pcb,menor_por_tamanio);
        pthread_mutex_lock(&mx_proximo_a_consultar);
        proximo_a_consultar = pcb;
        pthread_mutex_unlock(&mx_proximo_a_consultar);
        sem_post(&INTENTAR_INICIAR_NEW);
    }
    pthread_mutex_unlock(&mx_usar_cola_estado[NEW]);
    sem_post(&CANTIDAD_DE_PROCESOS_EN[NEW]);
}

void transicionar_a_ready(struct pcb *pcb,Estado estadoInicial){
    pthread_mutex_lock(&mx_usar_cola_estado[READY]); 
    if(strcmp(ALGORITMO_INGRESO_A_READY,"FIFO")==0){
        cambiarEstado(pcb,estadoInicial,READY);
    }else{
        sem_post(&REPLANIFICAR);
        cambiarEstadoOrdenado(pcb,estadoInicial,READY,menor_por_estimacion);
     }
    pthread_mutex_unlock(&mx_usar_cola_estado[READY]);
    sem_post(&CANTIDAD_DE_PROCESOS_EN[READY]);
}

void transicionar_a_susp_ready(struct pcb *pcb){
    pthread_mutex_lock(&mx_usar_cola_estado[SUSP_READY]); 
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
    pthread_mutex_unlock(&mx_usar_cola_estado[SUSP_READY]);
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
    int socket = iniciar_conexion_kernel_memoria();
    enviar_op_code(socket,operacion);
    bool respuesta = solicitar_permiso_a_memoria(socket,proceso,operacion); //(Adentro de la funcion, vamos a manejar un op_code)
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

int manejar_dump(struct pcb *aux,struct instancia_de_cpu* cpu_en_la_que_ejecuta){
    temporal_stop(aux->duracion_ultima_rafaga);
    cambiarEstado(aux,EXEC,BLOCKED);
    sem_post(&CANTIDAD_DE_PROCESOS_EN[BLOCKED]);
    aux->tiempo_bloqueado = temporal_create();
    int socket = iniciar_conexion_kernel_memoria();
    t_buffer *buffer = mandar_pid_a_memoria(aux->pid);
    crear_paquete(DUMP_MEMORY,buffer,socket);
    sacar_proceso_de_cola_de_estado(aux,EXEC);
    liberar_cpu(cpu_en_la_que_ejecuta);
    int respuesta = recibir_entero(socket);
    return respuesta;
}

void crear_hilo_de_ejecucion(struct pcb *proceso, struct instancia_de_cpu *cpu_aux){
    pthread_t hilo_ejecucion;
    struct parametros_de_ejecucion *parametros = malloc(sizeof(struct parametros_de_ejecucion));
    parametros->proceso = proceso;
    parametros->cpu_aux = cpu_aux;
    pthread_create(&hilo_ejecucion,NULL,poner_a_ejecutar,(void *)parametros);
    pthread_detach(hilo_ejecucion);
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
        recibir_op_code(cpu_en_la_que_ejecuta->socket_para_comunicarse);
        mandar_paquete_a_cpu(proceso,cpu_en_la_que_ejecuta); //mando paquete a cpu, le mando pid y pc del proceso
        log_debug(kernel_debug_log,"Esperando syscall");
        t_paquete *paquete = recibir_paquete(cpu_en_la_que_ejecuta->socket_para_comunicarse); //cpu ejecuta una instruccion y nos devuelve el pid con una syscall
        log_debug(kernel_debug_log,"Paquete recibido");
        proceso->pc = deserializar_pc(paquete);
        log_debug(kernel_debug_log,"El PC ES: %i",proceso->pc);
        op_code motivo_de_devolucion = obtener_codigo_de_operacion(paquete); //deserializa el opcode del paquete
        log_debug(kernel_debug_log,"Antes del if");
        if(motivo_de_devolucion!=DESALOJO_ACEPTADO){
            char *syscall = cambiar_syscall_a_string(motivo_de_devolucion);
            log_info(kernel_logger,"## (<PID: %i>) - Solicitó syscall: <%s>",proceso->pid,syscall);
        }
        switch(motivo_de_devolucion){
            case DESALOJO_ACEPTADO:
                temporal_stop(proceso->duracion_ultima_rafaga);
                proceso->proxima_estimacion = calcular_proxima_estimacion(proceso);
                desalojar_proceso_de_cpu(proceso,cpu_en_la_que_ejecuta);
                log_info(kernel_logger,"## (<PID: %i>) - DESALOJADO POR ALGORITMO SJF/SRT",proceso->pid);
                bloqueante = true;
                break;
            case INIT_PROC:
                char *nombre_archivo = deserializar_nombre_archivo_init_proc(paquete);
                int tamanio = deserializar_tamanio (paquete);
                log_debug(kernel_debug_log,"El nombre del archivo es: %s",nombre_archivo);
                log_debug(kernel_debug_log,"El tamanio del archivo es: %i",tamanio);
                log_debug(kernel_debug_log,"Proceso creado");
                crear_proceso(tamanio,nombre_archivo);
                proceso->pc++;
                enviar_op_code(cpu_en_la_que_ejecuta->socket_para_comunicarse,SYSCALL_EJECUTADA);
                //avisar que termine
                break;
            case EXIT:
                log_debug(kernel_debug_log, "estoy en exit");
                //Hay que sacarlo de la lista de exit
                liberar_cpu(cpu_en_la_que_ejecuta);
                finalizar_proceso(proceso,EXEC);
                enviar_op_code(cpu_en_la_que_ejecuta->socket_para_comunicarse,SYSCALL_EJECUTADA);
                //tener en cuenta lo del mediano plazo
                bloqueante = true;
                break;
            case DUMP_MEMORY:
                proceso->pc++;
                sacar_proceso_de_cola_de_estado(proceso,EXEC);
                int respuesta = manejar_dump(proceso,cpu_en_la_que_ejecuta); //esta funcion manda el proceso a BLOCKED y tambien libera la cpu
                if(respuesta == DUMP_ACEPTADO){
                    proceso->proxima_estimacion = calcular_proxima_estimacion(proceso);
                    transicionar_a_ready(proceso,BLOCKED);
                }
                else{
                    finalizar_proceso(proceso,BLOCKED);
                }
                bloqueante = true; 
                break;   
            case IO:
                proceso->pc++;
                log_warning(kernel_debug_log,"Estoy en la syscall IO");
                int milisegundos = deserializar_cant_segundos(paquete);
                log_debug(kernel_debug_log,"La cantidad de milisegundos es: %i",milisegundos);
                proceso->proxima_rafaga_io = milisegundos;
                proceso->nombre_io_que_lo_bloqueo = deserializar_nombre_syscall_io(paquete);
                log_debug(kernel_debug_log,"EL nombre de la io a usar es: %s",proceso->nombre_io_que_lo_bloqueo);
                pthread_mutex_lock(&mx_usar_recurso[REC_IO]);
                int pos = buscar_IO_solicitada(ios_conectados,proceso->nombre_io_que_lo_bloqueo);
                pthread_mutex_unlock(&mx_usar_recurso[REC_IO]);
                log_warning(kernel_debug_log,"DEspues de retornal la posicion");
                if(pos == -1){ //quiere decir que no hay ninguna syscall con ese nombre
                    liberar_cpu(cpu_en_la_que_ejecuta);
                    log_warning(kernel_debug_log,"adentro del IF de io");
                    finalizar_proceso(proceso,EXEC);
                }
                else{
                    log_info(kernel_logger,"## (<PID: %i>) - Bloqueado por IO: <%s>",proceso->pid,proceso->nombre_io_que_lo_bloqueo);
                    temporal_stop(proceso->duracion_ultima_rafaga);
                    sacar_proceso_de_cola_de_estado(proceso,EXEC);
                    liberar_cpu(cpu_en_la_que_ejecuta);
                    cambiarEstado(proceso,EXEC,BLOCKED);
                    //creamos el hilo para que bloquee el proceso
                    pthread_create(&proceso->hilo_al_bloquearse,NULL,funcion_para_bloqueados,proceso);
                    pthread_detach(proceso->hilo_al_bloquearse);
                    //Inicializamos el cronometro de bloqueado
                    proceso->tiempo_bloqueado = temporal_create();
                    pthread_mutex_lock(&mx_usar_recurso[REC_IO]);
                    struct instancia_de_io *io_aux = list_get(ios_conectados,pos);
                    pthread_mutex_unlock(&mx_usar_recurso[REC_IO]);
                    sem_post(io_aux->hay_procesos_esperando);
                    log_warning(kernel_debug_log,"Hice el post del semaforo de la IO");
                }
                bloqueante = true;
                break;
            default:
                log_debug(kernel_debug_log,"SYSCALL DESCONOCIDA");
                break;
        }
        /*------*/
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
    return NULL;
}

void liberar_cpu(struct instancia_de_cpu *cpu){
    cpu->puede_usarse = true;
    cpu->proceso_ejecutando = NULL;
    sem_post(&CPUS_LIBRES);
    if(strcmp(ALGORITMO_INGRESO_A_READY,"SJF_CON_DESALOJO")==0){
        sem_post(&REPLANIFICAR);
    }
}

void desalojar_proceso_de_cpu(struct pcb *proceso_desalojado, struct instancia_de_cpu *cpu_en_la_que_ejecuta){
    sacar_proceso_de_cola_de_estado(proceso_desalojado,EXEC);
    liberar_cpu(cpu_en_la_que_ejecuta);
    log_debug(kernel_debug_log,"Se desaloja de la cola EXECUTE al proceso con id: %i",proceso_desalojado->pid);
    transicionar_a_ready(proceso_desalojado,EXEC);
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
    log_debug(kernel_debug_log,"Llego la finalizacion %i",confirmacion);
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
        log_info(kernel_logger,"## (<PID: %i>) - Metricas de estado: %s %i %ld",proceso->pid,estado_string,proceso->metricas_de_estado[i],temporal_gettime(proceso->metricas_de_tiempo[i]));
        }
    }
}

void liberar_proceso(struct pcb *proceso){
    log_debug(kernel_debug_log,"Adentro de liberar proceso");
    temporal_destroy(proceso->duracion_ultima_rafaga);
    log_debug(kernel_debug_log,"Ultima rafaga destruida");
    for (int i=0; i<7;i++){
        if(proceso->metricas_de_estado[i]!=0){
        temporal_destroy(proceso->metricas_de_tiempo[i]);
        }
    }
    //free(proceso->ruta_del_archivo_de_pseudocodigo);
    log_debug(kernel_debug_log,"Antes de lo del nombre de IO");
    if(proceso->nombre_io_que_lo_bloqueo){
    free(proceso->nombre_io_que_lo_bloqueo);
    }
    log_debug(kernel_debug_log,"DESPUES de lo del nombre de IO");
    free(proceso);
}

void intentar_iniciar(){
    if(!list_is_empty(colaEstados[SUSP_READY])){
        log_debug(kernel_debug_log,"ENTRE AL IF QUE NO DEBERIA HABER ENTRADO");
        sem_post(&INTENTAR_INICIAR_SUSP_READY);
        if(strcmp(ALGORITMO_INGRESO_A_READY,"PMCP")==0){
            actualizar_proximo_a_consultar(SUSP_READY);
        }
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
