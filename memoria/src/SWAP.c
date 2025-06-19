//SWAP = Manda un Proceso a Estado Suspendido
//Guardas el contenido de las páginas de los procesos cuando son suspendidos (Estado SUSP.BLOCKED)

//Incluir las librerias

#include <SWAP.h>

//Define Localizacion PATH_SWAPFILE

#define PATH_SWAPFILE "/home/utnso/tp-2025-1c-Sinergia-SO-13/memoria/swapfile.bin"

//Declaracion de Variables de SWAP

static FILE* swapfile = NULL;
static t_list* paginas_en_swap = NULL;

//Funcion Iniciar SWAP

void inicializar_swap() {
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

//Funcion Escribir Pagina en SWAP

void escribir_pagina_en_swap(int pid, int nro_pagina, void* contenido) {
    int offset = ftell(swapfile);
    fseek(swapfile, 0, SEEK_END);
    fwrite(contenido, 1, memoria_config.TAM_PAGINA, swapfile);
    fflush(swapfile);

    t_pagina_en_swap* entry = malloc(sizeof(t_pagina_en_swap));
    entry->pid = pid;
    entry->nro_pagina = nro_pagina;
    entry->offset_en_archivo = offset;
    list_add(paginas_en_swap, entry);
}

//Funcion Leer Pagina en SWAP

void* leer_pagina_de_swap(int pid, int nro_pagina) {
    for (int i = 0; i < list_size(paginas_en_swap); i++) {
        t_pagina_en_swap* entry = list_get(paginas_en_swap, i);
        if (entry->pid == pid && entry->nro_pagina == nro_pagina) {
            void* buffer = malloc(memoria_config.TAM_PAGINA);
            fseek(swapfile, entry->offset_en_archivo, SEEK_SET);
            fread(buffer, 1, memoria_config.TAM_PAGINA, swapfile);
            return buffer;
        }
    }
    return NULL;
}

//Funcion Eliminar Pagina de SWAP

void eliminar_paginas_de_proceso(int pid) {
    for (int i = list_size(paginas_en_swap) - 1; i >= 0; i--) {
        t_pagina_en_swap* entry = list_get(paginas_en_swap, i);
        if (entry->pid == pid) {
            list_remove(paginas_en_swap, i);
            free(entry);
        }
    }
}

//Funcion Cerrar SWAP

void cerrar_swap() {
    if (swapfile != NULL) {
        fclose(swapfile);
        swapfile = NULL;
    }
    if (paginas_en_swap != NULL) {
        list_destroy_and_destroy_elements(paginas_en_swap, free);
        paginas_en_swap = NULL;
    }
}