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
  log_info(kernel_logger,"Se creo el proceso con el PID: %i",identificador_del_proceso);
  identificador_del_proceso++;
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

     
            
*/ 

/*void planificador_proceso_mas_chico_primero(){
    otro readline aca
    el proceso ingresa
    se consulta a la memoria si puede inicializarse o no (es agarrar la respuesta y despues un if)
    caso positivo
        cambiar de estado
    caso negativo
        se ingresa el proceso a una lista de procesos esperando, que a su vez esta lista se 
        este ordenando por tamaño (utilizar list_add_sorted)
    
    }*/
    
void planificador_largo_plazo_fifo(){
    /*readline conviene hacerlo aca 
    si la cola de new esta vacia agarro el primer elemento y se lo mando a memoria para ver si lo puedo inicializar*/ 
    /*Si la respuesta es positiva, se pasará el proceso al estado READY y se sigue la misma lógica con el proceso que sigue. 
    Si la respuesta es negativa (ya que la Memoria no tiene espacio suficiente para inicializarlo) 
    se deberá esperar la finalización de otro proceso para volver a intentar inicializarlo. 
    
    En cambio, si un proceso llega a esta cola y 
    ya hay otros esperando se debe tener en cuenta el algoritmo definido y 
    verificar si corresponde o no su ingreso. 

    */


char *line;
printf("Se esta esperando un enter por pantalla");
    do {
        line = readline("");
    } while (strlen(line) != 0); // si la longitud es mayor a 0 quiere decir que no se ingreso solo un enter
    free(line);  
    log_debug(kernel_debug_log,"INICIANDO PLANIFICADOR DE LARGO PLAZO");
    
    /*if(list_size(colaEstados[NEW])==1)){ que el proceso que se creo sea el unico que esta en new
        struct pcb *pcb_aux = agarrar_el_primer_proceso(); 
       bool respuesta = consultarMemoria (pcb_aux) //(Adentro de la funcion, vamos a manejar un op_code)
       if (respuesta == true){
            cambiarEstado(pcb_aux,NEW,READY);
            SIGNAL(INGRESO_DEL_PRIMERO)
       }
       else{
        aca va un semaforo, esperando para entrar a memoria
        
       }     
    } else{ //la cola no esta vacia
    wait(INGRESO_DEL_PRIMERO)
    }
     */   
    
}

struct pcb *agarrar_el_primer_proceso(){
    struct pcb *aux;
    
    aux =list_remove(colaEstados[NEW],0); //como el list add agrerga al final, sacamos del principio para no romper el comportamiento de la cola
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

/* En este algoritmo se va a priorizar a los procesos más chicos, por lo tanto, 
al llegar un nuevo proceso a NEW, independientemente de si hay o no procesos esperando, 
vamos a consultarle a la memoria si dispone del espacio requerido para iniciar el nuevo proceso y 
en caso afirmativo, se transiciona el proceso a READY. 
Caso contrario, se encolará para seguir esperando junto al resto de los procesos que no pueden ingresar 
por tamaño.
Al liberarse espacio en la memoria, 
se deberá validar si se pueden ingresar procesos a READY ordenándolos por tamaño de manera ascendente.
 */

//int seleccionar_proceso_segun_tamanio_mas_chico_en_memoria(t_list *lista){
 


    







