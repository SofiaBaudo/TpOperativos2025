//SWAP = Manda un Proceso a Estado Suspendido
//Guardas el contenido de las paginas de los procesos cuando son suspendidos (Estado SUSP.BLOCKED)

#include <SWAP.h>

#define PATH_SWAPFILE "/home/utnso/tp-2025-1c-Sinergia-SO-13/memoria/swapfile.bin"

static FILE* swapfile = NULL;
static t_list* paginas_en_swap = NULL;

int tamanio_total;
int tamanio_disponible_en_memoria;

//Funcion Iniciar SWAP
void inicializar_swap(){
    swapfile = fopen(PATH_SWAPFILE, "rb+");
    if (!swapfile) {
        swapfile = fopen(PATH_SWAPFILE, "wb+");
    }
    if (!swapfile) {
        perror("No se pudo abrir o crear el archivo swapfile.bin");
        exit(EXIT_FAILURE);
    }
    paginas_en_swap = list_create();
}
void escribir_pagina_en_swap(int pid, int nro_pagina, void* contenido,int tamanio_proceso,int cliente){
    int offset = ftell(swapfile);
    fseek(swapfile, 0, SEEK_END);
    fwrite(contenido, 1, memoria_config.TAM_PAGINA, swapfile); 
    //fwrite(contenido, 1, tamanio_marco, swapfile); 
    fflush(swapfile);
    t_pagina_en_swap* entry = malloc(sizeof(t_pagina_en_swap));
    entry->pid = pid;
    entry->nro_pagina = nro_pagina;
    entry->offset_en_archivo = offset;
    list_add(paginas_en_swap, entry);
    tamanio_disponible_en_memoria = tamanio_disponible_en_memoria - tamanio_proceso;
    enviar_op_code(cliente,SUSPENSION_CONFIRMADA);
}
void* leer_pagina_de_swap(int pid, int nro_pagina, int tamanio_proceso,int cliente) {
    if(TAM_MEMORIA >= (TAM_MEMORIA + tamanio_proceso))
    {
    for (int i = 0; i < list_size(paginas_en_swap); i++) {
        t_pagina_en_swap* entry = list_get(paginas_en_swap, i);
        if (entry->pid == pid && entry->nro_pagina == nro_pagina) {
            void* buffer = malloc(memoria_config.TAM_PAGINA);
            //void* buffer = malloc(tamanio_marco);
            fseek(swapfile, entry->offset_en_archivo, SEEK_SET);
            fread(buffer, 1, memoria_config.TAM_PAGINA, swapfile);
            //fread(buffer, 1, tamanio_marco, swapfile);
            return buffer;
        }
    }
    tamanio_disponible_en_memoria = tamanio_disponible_en_memoria + tamanio_proceso;
    enviar_op_code(cliente,ACEPTAR_PROCESO);
    return NULL;
    }
    enviar_op_code(cliente,NO_HAY_ESPACIO_EN_MEMORIA);
    return NULL;
}
//Funcion Eliminar Pagina de SWAP
void eliminar_paginas_de_proceso(int pid){
    for (int i = list_size(paginas_en_swap) - 1; i >= 0; i--) {
        t_pagina_en_swap* entry = list_get(paginas_en_swap, i);
        if (entry->pid == pid) {
            list_remove(paginas_en_swap, i);
            free(entry);
        }
    }
}
//Funcion Cerrar SWAP
void cerrar_swap(){
    if (swapfile != NULL) {
        fclose(swapfile);
        swapfile = NULL;
    }
    if (paginas_en_swap != NULL) {
        list_destroy_and_destroy_elements(paginas_en_swap, free);
        paginas_en_swap = NULL;
    }
}