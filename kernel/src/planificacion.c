#include <planificacion.h>
#include <readline/readline.h>
#include <string.h>


t_list* procesos = NULL;
t_list* colaEstados[7]={NULL};
int identificador_del_proceso = 0;
struct pcb *pcb;

void crear_proceso(int tamanio,char *ruta_archivo) { // tambien tiene que recibir el tamanio y el path
  struct pcb* pcb = malloc(sizeof(struct pcb));
    pcb -> pid = identificador_del_proceso; //ver lo de la variable global
    pcb -> pc = 0;
    pcb -> tamanio = tamanio;
    pcb -> ruta_del_archivo_de_pseudocodigo = ruta_archivo;
  //pcb -> estado = NEW; // seguramente no sirva mucho
    pcb -> lista_de_rafagas = list_create(); // crea la lista como vacia
  pthread_mutex_lock(&mx_usar_cola_new); //CONSULTAR EN SOPORTE. 
  list_add(colaEstados[NEW],pcb); // es una variable global asi que habria que poner un mutex
  pthread_mutex_unlock(&mx_usar_cola_new);
  sem_post(&CANTIDAD_DE_PROCESOS_EN_NEW);
  //un signal de un semaforo que le avise al plani de largo plazo por menor tamanio que se creo un proceso
  log_info(kernel_logger,"Se creo el proceso con el PID: %i",identificador_del_proceso);
  pthread_mutex_lock(&mx_identificador_del_proceso);
    identificador_del_proceso++; 
  pthread_mutex_unlock(&mx_identificador_del_proceso);
  return; 
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

bool menor_por_tamanio(void* a, void* b) {
    
    struct pcb* p1 = (struct pcb*) a;
    struct pcb* p2 = (struct pcb*) b;
    return p1->tamanio < p2->tamanio;
}

void insertar_ordenado_segun(t_list *lista, struct pcb *proceso, bool (*comparador)(void *, void *)) {
    list_add_sorted(lista, proceso, comparador);
}

/*
funcion planificador de largo plazo ()


    if(ALGORITMO_INGRESO_A_READY == FIFO){
        planificador_largo_plazo_fifo()
    }
    else{
        if(){
            el de mas chico primero es por tamanio
        }
        else{
            exit
        }
    }


void planificador_proceso_mas_chico_primero(){
    otro readline aca
    el proceso ingresa
    semaforo que espera a que se cree un proceso nuevo
    se consulta a la memoria si puede inicializarse o no (es agarrar la respuesta y despues un if)
    caso positivo
        cambiar de estado
    caso negativo
        se ingresa el proceso a una lista de procesos esperando, que a su vez esta lista se 
        este ordenando por tamanio (utilizar list_add_sorted)
    }*/
       
void esperar_enter_por_pantalla(){
    char *line;
printf("Se esta esperando un enter por pantalla");
    do {
        line = readline("");
    } while (strlen(line) != 0); // si la longitud es mayor a 0 quiere decir que no se ingreso solo un enter
    free(line);  
}

/*
struct pcb * finalizar proceso (proceso,estado anterior)
cambio de estado (proceso,estado anterior, exit)
*/


void *planificador_largo_plazo_fifo(){
    esperar_enter_por_pantalla();
    log_debug(kernel_debug_log,"INICIANDO PLANIFICADOR DE LARGO PLAZO");
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN_NEW); // si no hay nada espera a que llegue un proceso
        sem_wait(&INGRESO_DEL_PRIMERO); //que los demas esperen a que uno entre
        pthread_mutex_lock(&mx_usar_cola_new); // es una variable global asi que la protegemos (mejor un mx)
        t_list *aux = colaEstados[NEW];
        struct pcb *proceso = list_get(aux, 0);  // Obtener el primer elemento pero sin sacarlo de la lista todavia
        int tamanio = proceso->tamanio;
        pthread_mutex_unlock(&mx_usar_cola_new);
        int socket = iniciar_conexion_kernel_memoria();
        bool respuesta = solicitar_permiso_a_memoria(socket,tamanio); //(Adentro de la funcion, vamos a manejar un op_code)
        cerrar_conexion(socket);
        log_debug(kernel_debug_log,"Conexion con memoria cerrada");
        if (respuesta == true){
             pthread_mutex_lock(&mx_usar_cola_new);   //productor-consumidor
            struct pcb *pcb_aux = agarrar_el_primer_proceso(colaEstados[NEW]); //una vez que tenemos la confirmacion de memoria lo sacamos de la lista
                cambiarEstado(pcb_aux,NEW,READY);
                pthread_mutex_unlock(&mx_usar_cola_new);
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

struct pcb *agarrar_el_primer_proceso(t_list *lista){
    struct pcb *aux;
    aux =list_remove(lista,0); //como el list add agrerga al final, sacamos del principio para no romper el comportamiento de la cola
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
    list_add(colaEstados[estadoNuevo],pcb);
}

/* En este algoritmo se va a priorizar a los procesos mas chicos, por lo tanto, 
al llegar un nuevo proceso a NEW, independientemente de si hay o no procesos esperando, 
vamos a consultarle a la memoria si dispone del espacio requerido para iniciar el nuevo proceso y 
en caso afirmativo, se transiciona el proceso a READY. 
Caso contrario, se encolara para seguir esperando junto al resto de los procesos que no pueden ingresar 
por tamanio.
Al liberarse espacio en la memoria, 
se debera validar si se pueden ingresar procesos a READY ordenandolos por tamanio de manera ascendente.
 */

//int seleccionar_proceso_segun_tamanio_mas_chico_en_memoria(t_list *lista){
 
/*
(EJECUCION)
Una vez seleccionado el proceso a ejecutar, se lo transicionara al estado EXEC y 
se enviara a uno de los modulos CPU (conectados y libres) el PID y el PC a ejecutar a traves del 
puerto de dispatch, quedando a la espera de recibir dicho PID despues de la ejecucion 
junto con un motivo por el cual fue devuelto.
En caso que el algoritmo requiera desalojar al proceso en ejecucion, 
se enviara una interrupcion a traves de la conexion de interrupt para forzar el desalojo del mismo.
Al recibir el PID del proceso en ejecucion, en caso de que el motivo de devolucion implique replanificar, 
se seleccionara el siguiente proceso a ejecutar segun indique el algoritmo. 
Durante este período la CPU se quedara esperando.


(MEMORIA)
En este apartado solamente se tendra la instruccion DUMP_MEMORY. 
Esta syscall le solicita a la memoria, junto al PID que lo solicita, que haga un Dump del proceso.
Esta syscall bloqueara al proceso que la invoca hasta que el modulo memoria confirme la finalizacion de la operacion, 
en caso de error, el proceso se enviara a EXIT. Caso contrario, se desbloquea normalmente pasando a READY.


(ENTRADA/SALIDA)
El modulo Kernel administra los distintos dispositivos de IO que se conecten a el. 
momento de que un modulo de IO se conecte al Kernel, este ultimo debera identificarlo por medio de su nombre, 
en este punto el Kernel debera conocer todos los madulos de IO conectados, que procesos estan ejecutando IO en cada modulo y 
todos los procesos que estan esperando una IO determinada.
Para la utilizacian de estos madulos existira la syscall “IO” que recibira dos parametros: 
el nombre de la entrada/salida a usar y la cantidad de milisegundos que debera usarla. 
Primero se debera validar que la IO solicitada exista en el sistema, si no existe ninguna IO en el sistema con el nombre solicitado, el proceso se debera enviar a EXIT. 
En caso de que si exista al menos una instancia de IO, aun si la misma se encuentre ocupada, el kernel debera pasar el proceso al estado BLOCKED y agregarlo a la cola de bloqueados por la IO solicitada, 
en caso de que el kernel tenga una instancia de esta IO libre, debera enviarle al modulo correspondiente el PID y el tiempo por el cual debe bloquearse. 
Al momento que se conecte una nueva IO o se reciba el desbloqueo por medio de una de ellas, se debera verificar si hay procesos encolados para dicha IO y enviarlo a la misma. 
Al momento de recibir un mensaje de una IO se debera verificar que el mismo sea una confirmacian de fin de IO, en caso afirmativo, se debera validar si hay mas procesos esperando realizar dicha IO. En caso de que el mensaje corresponda a una desconexian de la IO, el proceso que estaba ejecutando en dicha IO, se debera pasar al estado EXIT.

*/

void planificador_corto_plazo_fifo(){
  
    while(1){
        sem_wait(&CANTIDAD_DE_PROCESOS_EN_READY);
        //semaforo de cpus libres
        sem_wait(&INGRESO_DEL_PRIMERO_READY); // a chequear
        log_debug(kernel_debug_log,"PLANI DE CORTO PLAZO INICIADO");
        pthread_mutex_lock(&mx_usar_cola_ready);
        struct pcb *aux = agarrar_el_primer_proceso(colaEstados[READY]);
        cambiarEstado(aux,READY,EXEC);
        //mejor hacer un list add para cada estado con sus respectivos mutex
        pthread_mutex_unlock(&mx_usar_cola_ready);
        t_buffer *buffer = crear_buffer_cpu(aux->pc,aux->pid);
        crear_paquete(ENVIO_PID_Y_PC,buffer,cliente_dispatch); //esta funcion crea el paquete y tambien lo envia
        //funcion enviar donde le mande el socket y el id de la cpu
        t_paquete *paquete = recibir_paquete(cliente_dispatch);
        op_code syscall = obtener_codigo_de_operacion(paquete);
        switch(syscall){
            case INIT_PROC:
                char *nombre_archivo = deserializar_nombre_archivo(paquete);
                int tamanio = deserializar_tamanio (paquete);
                crear_proceso(tamanio,nombre_archivo);
                //avisar que termine
                break;
            case EXIT:
                /*
                cambiar_estado(aux,EXEC,EXIT);
                crear_conexion_con_memoria()
                solicitar finalizacion de proceso
                liberar pcb
                intentar inicializar otro
                */
                //Hablar con Cami.
                break;
            case DUMP_MEMORY:
                //Hablar con Cami
                break;
            case IO:
                /*
                recorrer la lista
                preguntar por cada IO si es el nombre que buscamos
                    CASO AFIRMATIVO
                    - agregar a estado Bloqueado
                    - agregar a la cola de bloqueados por esa IO.
                    CASO NEGATIVO
                    -enviar a exit
                */
                break;
            default:
                break;
        }
    }


    /*si no hay nada en ready, semaforo para que espere
    una vez que llega algo, agarra el primer proceso que haya.
    una vez que agarra el proceso, lo transiciona a la cola de execute.
    mandar PID y PC a CPU, ya sea que este libre o conectada.
    
    
    */

}   







