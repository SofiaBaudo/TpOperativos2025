#include "instrucciones.h"

t_list* procesos;

void iniciar_lista_procesos() { //guarda a todos los procesos que están en memoria con sus instrucciones.
    procesos = list_create();
}

/* que hace cargar_instrucciones_proceso:
lee el archivo de pseudocódigo
guarda cada instrucción) en una lista (t_list* instrucciones) asociada al PID.
esto es para que cuando CPU  pida instrucción (PID, PC), memoria se la pueda devolver.*/

void cargar_instrucciones_proceso(int pid, char* path) {
    FILE* archivo = fopen(path, "r");
    if (!archivo) {
        log_error(logger_memoria, "No se pudo abrir el pseudocódigo: %s", path);
        return;
    }

    t_proceso* proc = malloc(sizeof(t_proceso));
    proc->pid = pid;
    proc->instrucciones = list_create();

    char* linea = NULL;
    size_t len = 0;
    while (getline(&linea, &len, archivo) != -1) {
        linea[strcspn(linea, "\n")] = 0;
        list_add(proc->instrucciones, strdup(linea));
    }
    free(linea);
    fclose(archivo);

    list_add(procesos, proc);
}

/*que hace obtener_instruccion
busca el proceso correspondiente al PID.
devuelve la instrucción de la posición PC dentro de la lista de instrucciones.
se usa para responderle a la CPU cuando  pide una instrucción.*/

char* obtener_instruccion(int pid, int pc) {
    for (int i = 0; i < list_size(procesos); i++) {
        t_proceso* proc = list_get(procesos, i);
        if (proc->pid == pid && pc < list_size(proc->instrucciones)) {
            return list_get(proc->instrucciones, pc);
        }
    }
    return "";
}