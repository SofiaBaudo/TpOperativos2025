#ifndef INSTRUCCIONES_MEMORIA_H
#define INSTRUCCIONES_MEMORIA_H

#include "bibliotecas.h"
#include "inicializar_memoria.h"
#include "procesos.h"


void enviar_instruccion(int socket_destino,char* instruccion);
// Genera una lista de instrucciones (cada elemento es un char) leyendo el archivo indicado
t_list* generar_instrucciones_proceso(int pid, char* path_pseudocodigo);
char* obtener_instruccion_proceso(int pid, int pc);

#endif