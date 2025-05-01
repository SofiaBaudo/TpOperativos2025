#include <planificacion.h>

t_list* procesos = NULL;
t_list* colaEstados[7]={NULL};

struct pcb *pcb;

void crear_proceso(int pid) {
  struct pcb* pcb = malloc(sizeof(struct pcb));
  pcb -> pid = pid;
  pcb -> pc = 0;
  pcb -> estado = NEW;
  pcb -> lista_de_rafagas = list_create(); // crea la lista como vacia
  list_add(colaEstados[NEW],pcb);
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
funcion planificador de largo plazo (recibe lista de procesos (¿ o un proceso?) , el algoritmo a usar(?), )
    
    si la cola new esta vacia
        inicializar conexion con kernel meoria ( pedir a memoria permiso para inicializar el proceso) y devuelve un op_code
            si la respuesta de memoria es true
                quito el proceso de la cola new.
                coloco el proceso en la cola ready (pasar_a_ready)   
            si la respuesta es false
            wait(FINALIZACION_DE_PROCESO)
            El signal se haria cuando el mismo planificador de largo plazo manda otro proceso a EXIT

    si la cola new no esta vacia
        mientras 
            
*/ 

//funcion para pasar a ready que recibe un proceso
void manejar_proceso(struct pcb *proceso){
    //crear_proceso();
    //planificador de largo plazo
}