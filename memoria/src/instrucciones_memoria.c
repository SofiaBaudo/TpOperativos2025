#include <instrucciones_memoria.h>

t_list* generar_instrucciones_proceso(int pid, const char* path_pseudocodigo) {
    FILE* archivo = fopen(path_pseudocodigo, "r");
    if (!archivo) 
        return NULL;

    t_list* instrucciones = list_create();
    if (!instrucciones) {
        fclose(archivo);
        return NULL;
    }

    char* linea = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&linea, &len, archivo)) != -1) {
        // Eliminar salto de línea final si existe
        if (read > 0 && linea[read-1] == '\n')
            linea[read-1] = '\0';
        list_add(instrucciones, strdup(linea));
    }
    free(linea);
    fclose(archivo);
    return instrucciones;
}

// Devuelve una copia de la instrucción (char) para el proceso y PC dado, o NULL si no existe
char* obtener_instruccion_proceso(int pid, int pc) {
    char* copia = NULL;
    struct t_proceso_memoria* proc = buscar_proceso_en_memoria(pid);
    pthread_mutex_lock(&mutex_procesos_en_memoria);
    if (proc && proc->instrucciones && pc >= 0 && pc < list_size(proc->instrucciones)) {
        char* original = list_get(proc->instrucciones, pc);
        if (original)
            copia = strdup(original);
    }
    pthread_mutex_unlock(&mutex_procesos_en_memoria);
    return copia;
}