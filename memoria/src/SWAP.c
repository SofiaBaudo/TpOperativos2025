#include "SWAP.h"

t_list* procesos_swap;
t_list* huecos_swap;

// Inicializa el archivo swapfile.bin y las estructuras administrativas necesarias
void inicializar_swap(){
    // Abrir el archivo swapfile.bin existente en modo lectura/escritura binario
    FILE* swapfile = fopen(memoria_config.PATH_SWAPFILE, "r+b");
    if (swapfile == NULL) {
        perror("No se pudo abrir el archivo swapfile.bin");
        exit(EXIT_FAILURE);
    }
    fclose(swapfile);   

    // Inicializar la lista global de procesos en swap
    procesos_swap = list_create();
    if (procesos_swap == NULL) {
        perror("No se pudo crear la lista de procesos en swap");
        exit(EXIT_FAILURE);
    }
    huecos_swap = list_create();
    if (huecos_swap == NULL) {
        perror("No se pudo crear la lista de huecos en swap");
        exit(EXIT_FAILURE);
    }
}

int suspender_proceso(int pid){
    log_info(logger_memoria, "## PID: <%d> - Iniciando suspensión del proceso", pid);
    
    // 1. Obtener la lista de marcos ocupados por el proceso en memoria principal
    t_list* marcos_proceso = obtener_marcos_proceso(pid);
    if (marcos_proceso == NULL || list_size(marcos_proceso) == 0) {
        // No hay marcos para este proceso
        return -1;
    }

    // 2. Calcular el tamaño total a guardar
    size_t tamanio_total = list_size(marcos_proceso) * memoria_config.TAM_PAGINA; // Porque el tamaño de cada marco es igual al de una pagina
    void* buffer = malloc(tamanio_total);
    if (buffer == NULL) {
        // Error de memoria
        list_destroy(marcos_proceso);
        return -1;
    }

    // 3. Leer los datos de cada marco y copiarlos al buffer
    for (int i = 0; i < list_size(marcos_proceso); i++) {
        int nro_marco = *(int*)list_get(marcos_proceso, i);
        void* datos_marco = leer_marco_memoria(nro_marco);
        memcpy(buffer + i * memoria_config.TAM_PAGINA, datos_marco, memoria_config.TAM_PAGINA);
        free(datos_marco);
    }

    // 4. Escribir el buffer en swap
    size_t offset_swap = buscar_espacio_libre_swap(tamanio_total); 
    if (offset_swap == (size_t)-1) {
        // No hay espacio suficiente en swap
        free(buffer);
        list_destroy(marcos_proceso);
        return -1;
    }
    if (escribir_en_swap(buffer, tamanio_total, offset_swap) != 0) {
        free(buffer);
        list_destroy(marcos_proceso);
        return -1;
    }

    // 5. Registrar el proceso en la lista de procesos en swap
    ProcesoSwap* pswap = malloc(sizeof(ProcesoSwap));
    pswap->pid = pid;
    pswap->offset_swap = offset_swap;
    pswap->tamanio = tamanio_total;
    // Guardar el path del pseudocódigo (deberás obtenerlo del proceso en memoria antes de finalizarlo)
    t_proceso_memoria* proc = buscar_proceso_en_memoria(pid); // función auxiliar externa
    if (proc && proc->path_pseudocodigo) {
        pswap->path_pseudocodigo = strdup(proc->path_pseudocodigo);
    } else {
        log_error(logger_memoria, "No se encontró el proceso %d en memoria", pid);
        pswap->path_pseudocodigo = NULL;
    }
    list_add(procesos_swap, pswap); // Usar semaforo

    // 6. Finalizo el proceso en memoria
    finalizar_proceso(pid);

    // 7. Liberar recursos
    free(buffer);
    list_destroy(marcos_proceso);

    return 0;
}

size_t buscar_espacio_libre_swap(size_t tamanio) {
    // 1. Buscar en la lista de huecos libres
    for (int i = 0; i < list_size(huecos_swap); i++) {
        HuecoSwap* hueco = list_get(huecos_swap, i);
        if (hueco->tamanio >= tamanio) {
            size_t offset = hueco->offset;
            // Si el hueco es más grande que lo necesario, ajusta el hueco
            if (hueco->tamanio > tamanio) {
                hueco->offset += tamanio;
                hueco->tamanio -= tamanio;
            } else {
                // Si el hueco es exacto, lo elimina de la lista
                list_remove(huecos_swap, i);
                free(hueco);
            }
            return offset;
        }
    }

    // 2. Si no hay hueco, escribir al final del archivo
    FILE* swapfile = fopen(memoria_config.PATH_SWAPFILE, "r+b");
    if (swapfile == NULL) {
        perror("No se pudo abrir el archivo swapfile.bin");
        return (size_t)-1;
    }
    fseek(swapfile, 0, SEEK_END);
    size_t tam_actual = ftell(swapfile);
    fclose(swapfile);

    return tam_actual;
}

int escribir_en_swap(void* buffer, size_t tamanio, size_t offset) {
    // Abre el archivo swapfile.bin en modo lectura/escritura
    FILE* swapfile = fopen(memoria_config.PATH_SWAPFILE, "r+b");
    if (swapfile == NULL) {
        perror("No se pudo abrir el archivo swapfile.bin");
        return -1;
    }

    // Mueve el cursor al offset especificado
    fseek(swapfile, offset, SEEK_SET);

    // Escribe los datos del buffer en el archivo
    size_t bytes_escritos = fwrite(buffer, 1, tamanio, swapfile);
    fclose(swapfile);

    if (bytes_escritos != tamanio) {
        perror("Error al escribir en el archivo swapfile.bin");
        return -1;
    }

    return 0; // Éxito
}

// Desuspende un proceso: lo restaura desde swap a memoria principal
int desuspender_proceso(int pid) {
    // 1. Buscar el proceso en la lista de procesos en swap => USAR SEMAFOROS
    ProcesoSwap* pswap = NULL; 
    for (int i = 0; i < list_size(procesos_swap); i++) {
        ProcesoSwap* p = list_get(procesos_swap, i);
        if (p->pid == pid) {
            pswap = p;
            break;
        }
    }
    if (!pswap) {
        log_error(logger_memoria, "No se encontró el proceso %d en swap", pid);
        return -1;
    }

    // 2. Leer el contenido del proceso desde swap
    void* buffer = malloc(pswap->tamanio);
    if (!buffer) {
        log_error(logger_memoria, "No se pudo reservar buffer para desuspender PID %d", pid);
        return -1;
    }
    if (leer_de_swap(buffer, pswap->tamanio, pswap->offset_swap) != 0) { 
        free(buffer);
        log_error(logger_memoria, "Error al leer de swap para PID %d", pid);
        return -1;
    }

    // 3. Inicializar estructuras administrativas del proceso en memoria usando el path guardado
    if (!inicializar_proceso(pid, pswap->tamanio, pswap->path_pseudocodigo)) {
        free(buffer);
        log_error(logger_memoria, "No se pudo inicializar el proceso %d en memoria", pid);
        return -1;
    }

    // 4. Asignar los datos leídos de swap a los marcos físicos del proceso
    t_list* marcos_proceso = obtener_marcos_proceso(pid);
    //marcos_proceso -> es la lista de marcos que tiene un proceso
    for (int i = 0; i < list_size(marcos_proceso); i++) {
       // int nro_marco = *(int*)list_get(marcos_proceso, i);
        t_marco* marco_aux = list_get(marcos_proceso, i);
        int nro_marco = marco_aux->numero_marco;
        void* datos_pagina = buffer + i * memoria_config.TAM_PAGINA;
        escribir_marco_memoria(nro_marco, datos_pagina);
    }
    list_destroy(marcos_proceso);

    // 5. Liberar el espacio ocupado en swap y eliminar el proceso de la lista de swap
    HuecoSwap* hueco = malloc(sizeof(HuecoSwap));
    hueco->offset = pswap->offset_swap;
    hueco->tamanio = pswap->tamanio;
    list_add(huecos_swap, hueco);
    list_remove_element(procesos_swap, pswap);
    if (pswap->path_pseudocodigo) free(pswap->path_pseudocodigo);
    free(pswap);

    // 6. Liberar buffer
    free(buffer);

    log_info(logger_memoria, "PID: %d desuspendido correctamente", pid);
    return 0;
}

// Lee datos desde swapfile.bin al buffer indicado
int leer_de_swap(void* buffer, size_t tamanio, size_t offset) {
    FILE* swapfile = fopen(memoria_config.PATH_SWAPFILE, "rb");
    if (!swapfile) {
        perror("No se pudo abrir el archivo swapfile.bin para leer");
        return -1;
    }
    fseek(swapfile, offset, SEEK_SET);
    size_t bytes_leidos = fread(buffer, 1, tamanio, swapfile);
    fclose(swapfile);
    if (bytes_leidos != tamanio) {
        perror("Error al leer de swapfile.bin");
        return -1;
    }
    return 0;
}