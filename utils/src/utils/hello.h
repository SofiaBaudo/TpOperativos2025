#ifndef UTILS_HELLO_H_
#define UTILS_HELLO_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/config.h>


/**
* @brief Imprime un saludo por consola
* @param quien Módulo desde donde se llama a la función
* @return No devuelve nada
*/
void saludar(char* quien);
t_config *crear_config(char* direccion);
void destruir_config(t_config *config);
t_log *iniciar_logger(char* archivoLog, char* nombreLog);
void destruir_logger(t_log *logger);
#endif
