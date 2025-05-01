#include <planificacion.h>

t_list* procesos = NULL;
t_list* colaEstados[7]={NULL};

struct pcb *pcb;

void crear_proceso(int pid) { // tambien tiene que recibir el tamanio y el path
  struct pcb* pcb = malloc(sizeof(struct pcb));
  pcb -> pid = pid; //ver lo de la variable global
  pcb -> pc = 0;
  pcb -> estado = NEW; // seguramente no sirva mucho
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
funcion planificador de largo plazo (recibe lista de procesos ()


    if(ALGORITMO_INGRESO_A_READY == FIFO){
        planificador_largo_plazo_fifo()
    }
    else{
        if(){
            el de mas chico primero
        }
        else{
            exit
        }
    }

     
            
*/ 

/*
planificador_largo_plazo_fifo(){
   readline conviene hacerlo aca 

   si la cola de new NO esta vacia agarro el primer elemento y se lo mando a memoria para ver si lo puedo inicializar 
}


*/

/*
planificador_proceso_mas_chico_primero(){
    
}
//funcion para pasar a ready que recibe un proceso
