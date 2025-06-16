#ifndef ESPACIO_DE_USUARIO_H
#define ESPACIO_DE_USUARIO_H

#include <stdbool.h>
#include <variables_globales_memoria.h>
#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <servidor_memoria.h>
#include <math.h>

// Inicialización de memoria de usuario
void inicializar_espacio_usuario();
// Funciones de página completa
char* leer_pagina_completa(unsigned int direccion_fisica);
char* actualizar_pagina_completa(unsigned int direccion_fisica, char* contenido);
// Funciones de acceso por direcciones físicas
char* funcion_lectura_fisica(unsigned int direccion_fisica, int tamanio);
char* funcion_escritura_fisica(unsigned int direccion_fisica, char* valor, int tamanio);

#endif