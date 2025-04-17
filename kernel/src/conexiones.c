#include <conexiones.h>

void atender_kernel_dispatch(){
bool estaTodoBien = 1;
    while (estaTodoBien) { // sigue ciclando mientras no se desconecte el cliente
		int cod_op = recibir_operacion(fd_cpu_dispatch);
		switch (cod_op) {
		case MENSAJE:
	
			break;
		case PAQUETE:
	
			break;
		case -1:
			log_error(kernel_logger, "Desconexion de CPU - KERNEL"); // para cuando el cliente se desconecta
            estaTodoBien = 0;
			//return EXIT_FAILURE;
            break;
		default:
			//log_warning(kernel_logger,"Operacion desconocida de ...."); // en .. van los modulos
			break;
		}
	}
	}

void atender_kernel_interrupt(){
	bool estaTodoBien = 1;
		while (estaTodoBien) { // sigue ciclando mientras no se desconecte el cliente
			int cod_op = recibir_operacion(fd_cpu_interrupt);
			switch (cod_op) {
			case MENSAJE:
			
				break;
			case PAQUETE:
			
				break;
			case -1:
				log_error(kernel_logger, "Desconexion de CPU - KERNEL"); // para cuando el cliente se desconecta
				estaTodoBien = 0;
				//return EXIT_FAILURE;
				break;
			default:
				//log_warning(kernel_logger,"Operacion desconocida de ...."); // en .. van los modulos
				break;
			}
			}
			}


void atender_kernel_io(){
	bool estaTodoBien = 1;
		while (estaTodoBien) { // sigue ciclando mientras no se desconecte el cliente
			int cod_op = recibir_operacion(fd_io);
			switch (cod_op) {
			case MENSAJE:
			
				break;
			case PAQUETE:
			
				break;
			case -1:
				log_error(kernel_logger, "Desconexion de CPU - KERNEL"); // para cuando el cliente se desconecta
				estaTodoBien = 0;
				//return EXIT_FAILURE;
				break;
			default:
				log_warning(kernel_logger,"Operacion desconocida de ...."); // en .. van los modulos
				break;
				}
			}
}

