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
  list_add(colaEstados[NEW],pcb);
  //un signal de un semaforo que le avise al plani de largo plazo por menor tamanio que se creo un proceso
  log_info(kernel_logger,"Se creo el proceso con el PID: %i",identificador_del_proceso);
  pthread_mutex_lock(&mx_identificador_del_proceso);
  identificador_del_proceso++; 
  pthread_mutex_unlock(&mx_identificador_del_proceso);
  return; 
}

int buscar_en_lista(t_list *lista, int pid) {
   
    if (!lista) {
        printf("La lista no tiene ningún proceso\n");
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
Funcion: Recibir Proceso ()
    recibo informacion de CPU.
    creo proceso con informacion recibida
    envio proceso a planificador de largo plazo.
*/


void *planificador_largo_plazo_fifo(){
    esperar_enter_por_pantalla();
    log_debug(kernel_debug_log,"INICIANDO PLANIFICADOR DE LARGO PLAZO");
    
    if(list_size(colaEstados[NEW])==1){ //que el proceso que se creo sea el unico que esta en new
        struct pcb *pcb_aux = agarrar_el_primer_proceso(colaEstados[NEW]); 
        int tamanio = pcb_aux->tamanio;
        int socket = iniciar_conexion_kernel_memoria();
        bool respuesta = solicitar_permiso_a_memoria(socket,tamanio); //(Adentro de la funcion, vamos a manejar un op_code)
        cerrar_conexion(socket);
        log_debug(kernel_debug_log,"Conexion con memoria cerrada");
        if (respuesta == true){
                cambiarEstado(pcb_aux,NEW,READY);
                //sem_post(&INGRESO_DEL_PRIMERO); //avisar que el proceso que estaba segundo puede solicitar su ingreso
        }
        else{
            log_debug(kernel_debug_log,"NO HAY ESPACIO SUFICIENTE EN MEMORIA");
            //aca va un semaforo, esperando para entrar a memoria una vez que termine otro proceso.
            //sem_post(&INGRESO_DEL_PRIMERO);
       }     
    } else{ //hay mas de un proceso
    //sem_wait(&INGRESO_DEL_PRIMERO);
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

Una vez seleccionado el proceso a ejecutar, se lo transicionara al estado EXEC y 
se enviara a uno de los modulos CPU (conectados y libres) el PID y el PC a ejecutar a traves del 
puerto de dispatch, quedando a la espera de recibir dicho PID despues de la ejecucion 
junto con un motivo por el cual fue devuelto.
En caso que el algoritmo requiera desalojar al proceso en ejecucion, 
se enviará una interrupcion a traves de la conexión de interrupt para forzar el desalojo del mismo.
Al recibir el PID del proceso en ejecución, en caso de que el motivo de devolución implique replanificar, 
se seleccionará el siguiente proceso a ejecutar según indique el algoritmo. 
Durante este período la CPU se quedará esperando.
.

*/

void planificador_corto_plazo_fifo(){
    
    /*si no hay nada en ready, semaforo para que espere
    una vez que llega algo, agarra el primer proceso que haya.
    una vez que agarra el proceso, lo transiciona a la cola de execute.
    mandar PID y PC a CPU, ya sea que este libre o conectada.
    
    
    */

}   







