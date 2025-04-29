#include <instrucciones.h>


void execute(instruccion instruccion_recibida){
    char *nombre = instruccion_recibida.nombreIns;
    char *syscalls[] = {"IO","INIT_PROC","DUMP_MEMORY","EXIT"};
    if(estaContenido(syscalls, nombre,4)){
        //hacer funcion que las envie al kernel
    }
    //no se puede hacer un switch con valores no enteros. 
    if(strcmp(nombre, "NOOP") == 0){
        //ejecutar noop
    }
    else if(strcmp(nombre, "WRITE") == 0){
        //ejecutar write
    }
    else if(strcmp(nombre, "READ") == 0){
        //ejecutar read
    }
    else if(strcmp(nombre, "GOTO") == 0){
        //ejecutar goto
    }

}

void instruccion_noop(){

}

void instruccion_write(){

}

void instruccion_read(){

}

void instruccion_goto(){

}
int obtenerLongitud(char **obtenerInsPartes){
    int contador = 0;
    for(int i = 0; i < 4; i++){ //asumo 4 --> [opcode, param1, param2, NULL]
        contador = contador +1;
    }
    return contador;
}

void decode(char* instruccion_recibida){
    instruccion instruccion;
    char **obtenerInsPartes = string_split(instruccion_recibida, " ");
    int longitudIns = obtenerLongitud(obtenerInsPartes);
    char *parametros[longitudIns-1]; //agarro la cola de obtenerIns que tiene los parametros. 
    for(int i = 1; i< longitudIns; i++){
        parametros[i-1] = obtenerInsPartes[i];
    }
    instruccion.nombreIns = obtenerInsPartes[0];
    instruccion.parametros[0] = parametros[0];
    instruccion.parametros[1] = parametros[1];
    instruccion.parametros[2] = parametros[2];

    char* traduccionNecesaria[] = {"WRITE", "READ"};
    if(estaContenido(traduccionNecesaria, instruccion.nombreIns, 2)){
        //llamar a la mmu para que lo traduzca
    }   
}
bool estaContenido(char **array, char* valor, int tam){
    for(int i = 0; i < tam; i++){
        if(array[i] == valor){
            return true;
        }
    }
    return false;
}

char* fetch(int pc){
    send(fd_conexion_dispatch_memoria,&pc,sizeof(int),0);
    char* instruccion_recibida;
    recv(fd_conexion_dispatch_memoria,&instruccion_recibida,sizeof(instruccion),0);
    return instruccion_recibida;  

}
void obtenerDelKernelPcPid(t_log* log, int pid, int pc){
    recv(fd_conexion_kernel_dispatch, &pid, sizeof(pid),0);
    recv(fd_conexion_kernel_dispatch, &pc, sizeof(pc),0);
    if(pc < 0){
        log_error(log, "El program Counter no puede ser negativo");
    }
    log_info(log, "## PID: <PID> - FETCH - Program Counter: <%d>", pc );
}