//Incluimos las librerias

#include <instrucciones.h>

//Fase fecth (Buscar proxima instruccion a realizar)(Primer Fase del Ciclo de Instruccion)

char* fetch(int pc){
    //Mando confirmacion de cpu a memoria, espero la instruccion a realizar de memoria
    send(fd_conexion_dispatch_memoria,&pc,sizeof(int),0);
    char* instruccion_recibida;
    recv(fd_conexion_dispatch_memoria,&instruccion_recibida,sizeof(instruccion),0);
    return instruccion_recibida;
}

//Fase Decode (Interpretar proxima ejecucion a ejecutar)(Segunda Fase del Ciclo de Instruccion)

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
    //Llamar a la MMMU para que lo traduzca
    }   
}

//Fase Execute (Ejecutar la instruccion)(Tercera Fase del Ciclo de Instruccion)

void execute(instruccion instruccion_recibida){
    char *nombre_instruccion = instruccion_recibida.nombreIns;
    char *syscalls[] = {"IO","INIT_PROC","DUMP_MEMORY","EXIT"};
    if(estaContenido(syscalls, nombre_instruccion ,4)){
    //Hacer funcion que las envie al kernel
    }
    //No se puede hacer un switch con valores no enteros. 
    if(strcmp(nombre_instruccion , "NOOP") == 0){
        //Ejecuto noop
        instruccion_noop();
    }
    else if(strcmp(nombre_instruccion , "WRITE") == 0){
        //ejecutar write
    }
    else if(strcmp(nombre_instruccion , "READ") == 0){
        //ejecutar read
    }
    else if(strcmp(nombre_instruccion , "GOTO") == 0){
        //ejecutar goto
    }

}

void instruccion_noop(void){
    //No hace nada, no se debe poner nada aca (Solo tiempo para dirrecionar memoria)
}

void instruccion_write(){
    //Acceder a memoria y escribir
}

void instruccion_read(){
    //Acceder a memoria y leer
}

void instruccion_goto(){
    //Actualizar por parametro el IP/PC n go to
}

int obtenerLongitud(char **obtenerInsPartes){
    int contador = 0;
    for(int i = 0; i < 4; i++){ //asumo 4 --> [opcode, param1, param2, NULL]
        contador = contador +1;
    }
    return contador;
}

bool estaContenido(char **array, char* valor, int tam){
    for(int i = 0; i < tam; i++){
        if(array[i] == valor){
            return true;
        }
    }
    return false;
}

void obtenerDelKernelPcPid(t_log* log, int pid, int pc){
    recv(fd_conexion_kernel_dispatch, &pid, sizeof(pid),0);
    recv(fd_conexion_kernel_dispatch, &pc, sizeof(pc),0);
    if(pc < 0){
        log_error(log, "El program Counter no puede ser negativo");
    }
    log_info(log, "## PID: <PID> - FETCH - Program Counter: <%d>", pc );
}