#include <inicializacionLogs.h>

t_log *iniciar_logger(void){
    t_log *nuevo_logger_CPU = log_create("archivoCPULog.log","LOGGER_CPU",1,LOG_LEVEL_INFO);
    if(!nuevo_logger_CPU){
        perror("No se pudo crear el logger");
        EXIT(EXIT_FAILURE);
    }
    return nuevo_logger_CPU;
}

void destruir_logger(t_log *logger){
    log_destroy(logger);
}

/*void leer_consola(t_log* logger)
{
	char* leido;

	// La primera te la dejo de yapa
	leido = readline("> ");
	log_info(logger,">> %s",leido);
	// El resto, las vamos leyendo y logueando hasta recibir un string vacío
while(strcmp(leido,"")!=0){
	free(leido);
leido = readline("> ");
	log_info(logger,">> %s",leido);
	
}
*/