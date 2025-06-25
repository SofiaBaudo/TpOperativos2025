#include <planificacion.h>
#include <readline/readline.h>
#include <string.h>

t_list* procesos = NULL;
t_list* colaEstados[7]={NULL};
int identificador_del_proceso = 0;
struct pcb *proximo_a_consultar;
int estimacion_de_prueba = 50;

void crear_proceso(int tamanio,char *ruta_archivo) { // tambien tiene que recibir el tamanio y el path
    struct pcb* pcb = malloc(sizeof(struct pcb));
    pcb = inicializar_un_proceso(pcb,tamanio,ruta_archivo);
    transicionar_a_new(pcb); //aca esta lo de ultimo proceso en entrar
    log_info(kernel_logger,"Se creo el proceso con el PID: %i",identificador_del_proceso);
    log_debug(kernel_debug_log,"Su proxima estimacion es: %f",pcb->proxima_estimacion);
    incrementar_var_global_id_proceso();
  return; 
}

void *planificador_largo_plazo_fifo(){
    esperar_enter_por_pantalla();
    log_debug(kernel_debug_log,"INICIANDO PLANIFICADOR DE LARGO PLAZO");
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[NEW]); // si no hay nada espera a que llegue un proceso
        sem_wait(&INTENTAR_INICIAR_NEW); //que los demas esperen a que uno entre
        sem_wait(&UNO_A_LA_VEZ); //este semaforo hace que el planificador maneje un proceso a la vez.
        //verificar_cola_de_suspendido_ready
        if(!list_is_empty(colaEstados[SUSP_READY])){
            sem_wait(&SUSP_READY_SIN_PROCESOS);
        }
        struct pcb* primer_proceso = obtener_copia_primer_proceso_de(NEW); //no lo sacamos de la lista todavia pero obtenemos una referencia
        bool respuesta = consultar_si_puede_entrar(primer_proceso);
        log_debug(kernel_debug_log,"Conexion con memoria cerrada");
        if (respuesta == true){
            primer_proceso = sacar_primero_de_la_lista(NEW); //Una vez que tenemos la confirmacion de memoria ahi si lo sacamos de la cola de NEW
            transicionar_a_ready(primer_proceso,NEW);
            sem_post(&INTENTAR_INICIAR_NEW); //incremento el semaforo para que pueda iniciar el planificador.
        }   
        else{
            log_debug(kernel_debug_log,"NO HAY ESPACIO SUFICIENTE EN MEMORIA");
            //sem_post(&CANTIDAD_DE_PROCESOS_EN_NEW); 
            sem_post(&CANTIDAD_DE_PROCESOS_EN[NEW]); //si no hay espacio en memoria, aviso que el proceso sigue estando en new.
        }
        sem_post(&UNO_A_LA_VEZ);//me aseguro que se siga tratando de a un proceso.
    } 
    return NULL;
}

void *planificador_largo_plazo_proceso_mas_chico_primero(){
    esperar_enter_por_pantalla();
    log_debug(kernel_debug_log,"INICIANDO PLANIFICADOR DE LARGO PLAZO TMCP");
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[NEW]);// si no hay nada espera a que llegue un proceso
        sem_wait(&INTENTAR_INICIAR_NEW); //que los demas esperen a que uno entre
        sem_wait(&UNO_A_LA_VEZ); //este semaforo hace que el planificador maneje un proceso a la vez.
        //verifico que la cola de SUSP_READY este vacía.
        if(!list_is_empty(colaEstados[SUSP_READY])){
            sem_wait(&SUSP_READY_SIN_PROCESOS); //espero a que se vacíe la lista de susp ready.
        }
        struct pcb* primer_proceso = obtener_copia_primer_proceso_de(NEW); 
        if(primer_proceso->pid == proximo_a_consultar->pid){ 
            bool respuesta = consultar_si_puede_entrar(primer_proceso);
            log_debug(kernel_debug_log,"Conexion con memoria cerrada");
            if(respuesta == true){
                primer_proceso = sacar_primero_de_la_lista(NEW); //saco el primer proceso de la lista de NEW
                transicionar_a_ready(primer_proceso,NEW); // lo transiciono a READY
                actualizar_proximo_a_consultar(NEW); //actualizo la lista de NEW
                sem_post(&INTENTAR_INICIAR_NEW); //intento iniciar nuevamente la lista de NEW
            }
            else{
                sem_post(&CANTIDAD_DE_PROCESOS_EN[NEW]); //aviso que el proceso sigue en new
            }
        }
        else{
            sem_post(&CANTIDAD_DE_PROCESOS_EN[NEW]);  //aviso que el proceso sigue en new
        }
        sem_post(&UNO_A_LA_VEZ); //me aseguro que se siga tratando de a un proceso a la vez.
    }
}

void *planificador_corto_plazo_fifo(){
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[READY]); //espero a que llegue un proceso a ready.
        sem_wait(&CPUS_LIBRES); //espero a que haya cpus libres.
        //sem_wait(&INGRESO_DEL_PRIMERO_READY); // a chequear
        usleep(3000000); 
        log_debug(kernel_debug_log,"PLANI DE CORTO PLAZO INICIADO");
        pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
        int pos_cpu = buscar_cpu_libre(cpus_conectadas); 
        struct instancia_de_cpu *cpu_aux = list_get(cpus_conectadas,pos_cpu);
        pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
        struct pcb* proceso = sacar_primero_de_la_lista(READY);
        cambiarEstado(proceso,READY,EXEC);
        log_debug(kernel_debug_log,"El proceso %i pasa a ejecutar en la cpu %i",proceso->pid,cpu_aux->id_cpu);
        //poner_a_ejecutar(proceso,cpu_aux); //estaria bueno mandar tambien la cpu que usa 
        //sem_post(&INGRESO_DEL_PRIMERO_READY);
    }
}   

void *planificador_corto_plazo_sjf_sin_desalojo(){
      while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[READY]);
        //sem_wait(&CANTIDAD_DE_PROCESOS_EN_READY);
        sem_wait(&CPUS_LIBRES);
        //sem_wait(&INGRESO_DEL_PRIMERO_READY); //Preguntar si es necesario pero creeriamos que con el de cantProcesos y el de las cpus ya esta
        usleep(3000000); 
        log_debug(kernel_debug_log,"PLANI DE CORTO PLAZO INICIADO");
        pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
        int pos_cpu = buscar_cpu_libre(cpus_conectadas);
        struct instancia_de_cpu *cpu_aux = list_get(cpus_conectadas,pos_cpu);
        pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
        struct pcb* proceso = sacar_primero_de_la_lista(READY);
        cambiarEstado(proceso,READY,EXEC);
        log_debug(kernel_debug_log,"El proceso %i pasa a ejecutar en la cpu %i",proceso->pid,cpu_aux->id_cpu);
        //poner_a_ejecutar(proceso,cpu_aux);
        //sem_post(&INGRESO_DEL_PRIMERO_READY);
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
            //asignarle un hilo al proceso
            poner_a_ejecutar(proceso,cpu_aux); //esta funcion tambien va a recibir la cpu.
        }
        else{
            bool desalojo = ver_si_hay_que_desalojar(proceso);
            if(desalojo){
                struct pcb* proceso_a_desalojar = buscar_el_mas_grande(EXEC);
                enviar_entero(cliente_interrupt,1); //Con esto aviso que quiero desalojar un proceso
                enviar_entero(cliente_dispatch,proceso_a_desalojar->pid); //Con esto le indico a la cpu cual quiero desalojar
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
            //sem_post(&CANTIDAD_DE_PROCESOS_EN_READY);
            sem_post(&CANTIDAD_DE_PROCESOS_EN[READY]);
            }
        }
        sem_post(&CPUS_LIBRES); // es para que no se bloquee, pero hay que preguntar en el soporte si las cpus se conectan primero
    }
}

void *planificador_mediano_plazo(){
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[SUSP_BLOCKED]);
        //avisar a memoria que lo pase a swap
        intentar_iniciar();
    }
}

void *planificador_mediano_plazo_fifo(){
    while(1){
    sem_wait(&CANTIDAD_DE_PROCESOS_EN[SUSP_READY]);
    sem_wait(&INTENTAR_INICIAR_SUSP_READY);
    sem_wait(&UNO_A_LA_VEZ_SUSP_READY);
    struct pcb* primer_proceso = obtener_copia_primer_proceso_de(SUSP_READY); //no lo sacamos de la lista todavia pero obtenemos una referencia
        bool respuesta = consultar_si_puede_entrar(primer_proceso);
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
        sem_post(&UNO_A_LA_VEZ_SUSP_READY);
    }
}

void *planificador_mediano_plazo_proceso_mas_chico_primero(){
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN[SUSP_READY]);// si no hay nada espera a que llegue un proceso
        sem_wait(&INTENTAR_INICIAR_SUSP_READY);
        sem_wait(&UNO_A_LA_VEZ_SUSP_READY);
        struct pcb* primer_proceso = obtener_copia_primer_proceso_de(SUSP_READY);
        if(primer_proceso->pid == proximo_a_consultar->pid){
            bool respuesta = consultar_si_puede_entrar(primer_proceso);
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
        sem_post(&UNO_A_LA_VEZ_SUSP_READY);
    }
}

//FUNCIONES PLANI MEDIANO PLAZO

void *funcion_para_bloqueados(struct pcb *proceso){

    usleep(atoi(TIEMPO_SUSPENSION)); //preguntar 
    pthread_mutex_lock(&mx_usar_cola_estado[BLOCKED]);
    int pos = buscar_en_lista(colaEstados[BLOCKED],proceso->pid);
    pthread_mutex_unlock(&mx_usar_cola_estado[BLOCKED]);
    if(pos!=-1){
        sacar_de_cola_de_estado(proceso,BLOCKED);
        cambiarEstado(proceso,BLOCKED,SUSP_BLOCKED); //A chequear
        sem_post(&CANTIDAD_DE_PROCESOS_EN[SUSP_BLOCKED]);
    }
    pthread_detach(proceso->hilo_al_bloquearse);//El hilo se desacopla del hilo principal.
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

struct pcb *buscar_el_mas_grande(){
    pthread_mutex_lock(&mx_usar_recurso[REC_CPU]);
    ordenar_lista_segun(cpus_conectadas,menor_por_estimacion_de_los_que_ya_estan_ejecutando);
    struct instancia_de_cpu *ultima_cpu = list_get(cpus_conectadas,list_size(cpus_conectadas)-1);
    pthread_mutex_unlock(&mx_usar_recurso[REC_CPU]);
    return ultima_cpu->proceso_ejecutando;
}

void frenar_y_restar_cronometros(t_list *lista){
    t_list_iterator *aux = list_iterator_create(lista); //arranca apuntando a NULL, no a donde apunta a lista
    log_debug(kernel_debug_log,"afuera del while");
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
    //pcb->proxima_estimacion = atoi(ESTIMACION_INICIAL);
    pcb->proxima_estimacion = estimacion_de_prueba;
    estimacion_de_prueba-=30;
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

void transicionar_a_new(struct pcb *pcb){
 pthread_mutex_lock(&mx_usar_cola_estado[NEW]); 
    //if(strcmp(ALGORITMO_INGRESO_A_READY,FIFO)==0){
        list_add(colaEstados[NEW],pcb); // es una variable global asi que habria que poner un mutex
    //}else{
        /*insertar_ordenado_segun(colaEstados[NEW],pcb,menor_por_tamanio);
        pthread_mutex_lock(&mx_proximo_a_consultar);
        proximo_a_consultar = pcb;
        pthread_mutex_unlock(&mx_proximo_a_consultar);
        sem_post(&INTENTAR_INICIAR_NEW);
        */
    //}
    pthread_mutex_unlock(&mx_usar_cola_estado[NEW]);
    sem_post(&CANTIDAD_DE_PROCESOS_EN[NEW]);
    //un signal de un semaforo que le avise al plani de largo plazo por menor tamanio que se creo un proceso
}

void transicionar_a_ready(struct pcb *pcb,Estado estadoInicial){
    pthread_mutex_lock(&mx_usar_cola_estado[READY]); 
    //if(strcmp(ALGORITMO_INGRESO_A_READY,FIFO)==0){
        //cambiarEstado(pcb,estadoInicial,READY);
    //}else{
        sem_post(&REPLANIFICAR);
        cambiarEstadoOrdenado(pcb,estadoInicial,READY,menor_por_estimacion);
     //}
    pthread_mutex_unlock(&mx_usar_cola_estado[READY]);
    sem_post(&CANTIDAD_DE_PROCESOS_EN[READY]);
    //un signal de un semaforo que le avise al plani de largo plazo por menor tamanio que se creo un proceso
}

void transicionar_a_susp_ready(struct pcb *pcb){
    pthread_mutex_lock(&mx_usar_cola_estado[SUSP_READY]); 
    //if(strcmp(ALGORITMO_INGRESO_A_READY,FIFO)==0){
        //cambiarEstado(pcb,SUSP_BLOCKED,SUSP_READY);
    //}
    // else{
        pthread_mutex_lock(&mx_proximo_a_consultar);
        proximo_a_consultar = pcb;
        pthread_mutex_unlock(&mx_proximo_a_consultar);
        cambiarEstadoOrdenado(pcb,SUSP_BLOCKED,SUSP_READY,menor_por_tamanio);
        sem_post(&INTENTAR_INICIAR_SUSP_READY);
     //}
    pthread_mutex_unlock(&mx_usar_cola_estado[SUSP_READY]);
    sem_post(&CANTIDAD_DE_PROCESOS_EN[SUSP_READY]);
    //un signal de un semaforo que le avise al plani de largo plazo por menor tamanio que se creo un proceso
}

void cambiarEstado (struct pcb* pcb,Estado estadoAnterior, Estado estadoNuevo){
    char *string_estado_nuevo = cambiar_a_string(estadoNuevo);
    char *string_estado_anterior = cambiar_a_string(estadoAnterior);
    log_info(kernel_logger,"El proceso %i cambia del estado %s a %s ",pcb->pid,string_estado_anterior,string_estado_nuevo);
    pthread_mutex_unlock(&mx_usar_cola_estado[estadoNuevo]);
    list_add(colaEstados[estadoNuevo],pcb); //preguntar si hay que usar mutex
    pthread_mutex_unlock(&mx_usar_cola_estado[estadoNuevo]);
}

void cambiarEstadoOrdenado(struct pcb* pcb,Estado estadoAnterior, Estado estadoNuevo,bool (*comparador)(void *, void *)){
    char *string_estado_nuevo = cambiar_a_string(estadoNuevo);
    char *string_estado_anterior = cambiar_a_string(estadoAnterior);
    log_info(kernel_logger,"El proceso %i cambia del estado %s a %s ",pcb->pid,string_estado_anterior,string_estado_nuevo);
    pthread_mutex_unlock(&mx_usar_cola_estado[estadoNuevo]);
    list_add_sorted(colaEstados[estadoNuevo],pcb,comparador); //preguntar si hay que usar mutex
    pthread_mutex_unlock(&mx_usar_cola_estado[estadoNuevo]);
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

    if (estado < 0 || estado > SUSP_BLOCKED) {
        return "ESTADO_DESCONOCIDO";
    }

    return nombres_estados[estado];
}


bool consultar_si_puede_entrar(struct pcb *proceso){
    int socket = iniciar_conexion_kernel_memoria();
    bool respuesta = solicitar_permiso_a_memoria(socket,proceso->tamanio); //(Adentro de la funcion, vamos a manejar un op_code)
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
    //capaz agregar que proceso->ultima_estimacion sea esta
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

void sacar_de_cola_de_estado(struct pcb *proceso,Estado estado){
    pthread_mutex_lock(&mx_usar_cola_estado[estado]);
    int pos = buscar_en_lista(colaEstados[estado],proceso->pid);
    list_remove(colaEstados[estado],pos);
    pthread_mutex_unlock(&mx_usar_cola_estado[estado]);
}

void mandar_paquete_a_cpu(struct pcb *proceso){
        t_buffer *buffer = crear_buffer_cpu(proceso->pid,proceso->pc);
        log_debug(kernel_debug_log,"Se creo el buffer con el pid %i y el pc %i", proceso->pid, proceso->pc);
        crear_paquete(ENVIO_PID_Y_PC,buffer,cliente_dispatch); //esta funcion crea el paquete y tambien lo envia
}

int manejar_dump(struct pcb *aux,struct instancia_de_cpu* cpu_en_la_que_ejecuta){
    temporal_stop(aux->duracion_ultima_rafaga);
    cambiarEstado(aux,EXEC,BLOCKED);
    //pthread_create(&aux->hilo_al_bloquearse,NULL,funcion_para_bloqueados,aux); preguntar en el soporte
    sem_post(&CANTIDAD_DE_PROCESOS_EN[BLOCKED]);
    aux->tiempo_bloqueado = temporal_create();
    int socket = iniciar_conexion_kernel_memoria();
    t_buffer *buffer = mandar_pid_a_memoria(aux->pid);
    crear_paquete(DUMP_MEMORY,buffer,socket);
    sacar_de_cola_de_estado(aux,EXEC);
    liberar_cpu(cpu_en_la_que_ejecuta);
    int respuesta = recibir_entero(socket);
    return respuesta;
}

void poner_a_ejecutar(struct pcb* proceso, struct instancia_de_cpu *cpu_en_la_que_ejecuta){
    cpu_en_la_que_ejecuta->proceso_ejecutando = proceso;
    proceso->duracion_ultima_rafaga = temporal_create();
    bool bloqueante = false;
    while(!bloqueante){
        log_debug(kernel_debug_log,"Entre al while");
        mandar_paquete_a_cpu(proceso);
        log_debug(kernel_debug_log,"Paquete mandado a cpu");
        t_paquete *paquete = recibir_paquete(cliente_dispatch); //cpu ejecuta una instruccion y nos devuelve el pid con una syscall
        //deserializar el pc
        //actualizarlo
        op_code motivo_de_devolucion = obtener_codigo_de_operacion(paquete); //deserializa el opcode del paquete
        switch(motivo_de_devolucion){
            case DESALOJO_ACEPTADO:
                temporal_stop(proceso->duracion_ultima_rafaga);
                proceso->proxima_estimacion = calcular_proxima_estimacion(proceso);
                desalojar_proceso_de_cpu(proceso,cpu_en_la_que_ejecuta);
                bloqueante = true;
            case INIT_PROC:
                char *nombre_archivo = deserializar_nombre_archivo(paquete);
                int tamanio = deserializar_tamanio (paquete);
                crear_proceso(tamanio,nombre_archivo);
                //avisar que termine
                break;
            case EXIT:
                //Hay que sacarlo de la lista de exit
                finalizar_proceso(proceso,EXEC);
                liberar_cpu(cpu_en_la_que_ejecuta);
                //tener en cuenta lo del mediano plazo
                bloqueante = true;
                break;
            case DUMP_MEMORY:
                sacar_de_cola_de_estado(proceso,EXEC);
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
                int milisegundos = deserializar_cant_segundos(paquete);
                proceso->proxima_rafaga_io = milisegundos;
                char *nombre_io_a_usar = deserializar_nombre_syscall_io(paquete);
                int posicionIO = buscar_IO_solicitada(ios_conectados,nombre_io_a_usar);
                if(posicionIO == -1){ //quiere decir que no hay ninguna syscall con ese nombre
                    finalizar_proceso(proceso,EXEC);
                    liberar_cpu(cpu_en_la_que_ejecuta);
                }
                else{
                    temporal_stop(proceso->duracion_ultima_rafaga);
                    sacar_de_cola_de_estado(proceso,EXEC);
                    cambiarEstado(proceso,EXEC,BLOCKED);
                    //pthread_create(&aux->hilo_al_bloquearse,NULL,funcion_para_bloqueados,aux);
                    sem_post(&CANTIDAD_DE_PROCESOS_EN[BLOCKED]);
                    proceso->tiempo_bloqueado = temporal_create();
                    liberar_cpu(cpu_en_la_que_ejecuta);
                    pthread_mutex_lock(&mx_usar_recurso[IO]);
                    struct instancia_de_io *io_aux = list_get(ios_conectados,posicionIO);
                    pthread_mutex_unlock(&mx_usar_recurso[REC_IO]);
                    list_add(io_aux->procesos_esperando,proceso);
                    sem_post(&io_aux->hay_procesos_esperando);
                }
                bloqueante = true;
                break;
            default:
                log_debug(kernel_debug_log,"SYSCALL DESCONOCIDA");
                break;
        }
    }
    
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
    sacar_de_cola_de_estado(proceso_desalojado,EXEC);
    liberar_cpu(cpu_en_la_que_ejecuta);
    log_debug(kernel_debug_log,"Se desaloja de la cola EXECUTE al proceso con id: %i",proceso_desalojado->pid);
    transicionar_a_ready(proceso_desalojado,EXEC);
}

void finalizar_proceso(struct pcb *aux, Estado estadoInicial){
    sacar_de_cola_de_estado(aux,estadoInicial);
    cambiarEstado(aux,estadoInicial,EXIT_ESTADO);
    int socket = iniciar_conexion_kernel_memoria();
    t_buffer *buffer = mandar_pid_a_memoria(aux->pid);
    crear_paquete(EXIT,buffer,socket);
    cerrar_conexion(socket);
    sacar_de_cola_de_estado(aux,EXIT_ESTADO);    
    liberar_proceso(aux); //free de todos los punteros, lo demas se va con el free (aux)
    intentar_iniciar();

    //int confirmacion = recibir_entero(socket);
    //return confirmacion;
}

void liberar_proceso(struct pcb *aux){
    temporal_destroy(aux->duracion_ultima_rafaga);
    free(aux->ruta_del_archivo_de_pseudocodigo);
    free(aux);
}

void intentar_iniciar(){
    if(!list_is_empty(colaEstados[SUSP_READY])){
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
