//Incluimos las librerias.

#include <instrucciones.h>

//Fase fecth (Buscar proxima instruccion a realizar)(Primer Fase del Ciclo de Instruccion).

char* fetch(int pc){
    //Mando confirmacion de cpu a memoria, espero la instruccion a realizar de memoria.
    send(fd_conexion_dispatch_memoria,&pc,sizeof(int),0);
    recv(fd_conexion_dispatch_memoria,&instruccion_recibida,sizeof(instruccion),0);
    return instruccion_recibida;
}

//Fase Decode (Interpretar proxima ejecucion a ejecutar)(Segunda Fase del Ciclo de Instruccion)

void decode(char* instruccion_recibida){
    //Decodifico las instrucciones
    instruccion instruccion;
    obtenerInsPartes = string_split(instruccion_recibida, " "); //te recibe el string tal como es si no lo encuentra
    longitudIns = obtenerLongitud(obtenerInsPartes);
    parametros[longitudIns-1]; //Agarro el string de obtenerIns que tiene los parametros. 
    for(int i = 1; i< longitudIns; i++){ // hardcodear el tema de las instrucciones 
        parametros[i-1] = obtenerInsPartes[i]; // hacer un t_instruccion
    }
    instruccion.nombreIns = obtenerInsPartes[0];
    instruccion.parametros[0] = parametros[0];
    instruccion.parametros[1] = parametros[1];
    instruccion.parametros[2] = parametros[2];
    if(estaContenido(traduccionNecesaria, instruccion.nombreIns, 2)){ //hacer un if en vez array 
    //Llamar a la MMMU para que lo traduzca.
    }   
}

//Fase Execute (Ejecutar la instruccion)(Tercera Fase del Ciclo de Instruccion).

void execute(instruccion instruccion_recibida){
    nombre_instruccion = instruccion_recibida.nombreIns;
    if(estaContenido(syscalls, nombre_instruccion ,4)){ //hacer un if en vez de array
    //Hacer funcion que las envie al kernel.
    }
    //No se puede hacer un switch con valores no enteros. 
    if(strcmp(nombre_instruccion , "NOOP") == 0){
        //Ejecuto Noop.
        instruccion_noop();
    }
    else if(strcmp(nombre_instruccion , "WRITE") == 0){
        //Ejecuto Write.
        instruccion_write();
    }
    else if(strcmp(nombre_instruccion , "READ") == 0){
        //Ejecuto Read.
        instruccion_read();
    }
    else if(strcmp(nombre_instruccion , "GOTO") == 0){
        //Ejecuto Go To.
        instruccion_goto();
    }
}

//MMU la canti de niveles es variable
//hay un loop infinito con los go to
// /Ejecucion Noop.

void instruccion_noop(void){
    //No hace nada, no se debe poner nada aca (Solo tiempo para dirrecionar memoria).
}

//Ejecucion Write.

void instruccion_write(){
    //Acceder a memoria y escribir.
}

//Ejecucion Read.

void instruccion_read(){
    //Acceder a memoria y leer.
}

//Ejecucion Go to.

void instruccion_goto(){
    //Actualizar por parametro el IP/PC n go to.
}

//Chequear Interrupcion

void check_interrupt(){
    //
}

//Obtengo la Longitud de la Instruccion.

int obtenerLongitud(char **obtenerInsPartes){
    contador = 0;
    //Hago bucle para ver Longitud.
    for(int i = 0; i < 4; i++){ //asumo 4 --> [opcode, param1, param2, NULL]
        contador = contador +1;
    }
    return contador;
}

//Reviso si esta contenido el valor en el Vector.

bool estaContenido(char* array, char* valor, int tam){
    //Hago bucle para revisar si el valor esta en el vector.
    for(int i = 0; i < tam; i++){
        if(array[i] == valor){
            return true;
        }
    }
    return false;
}

//Me comunico con el Kernel para obtenerr el PC/IP y el PID.

void obtenerDelKernelPcPid(t_log* log, int pid, int pc){
    recv(fd_conexion_kernel_dispatch, &pid, sizeof(pid),0);
    recv(fd_conexion_kernel_dispatch, &pc, sizeof(pc),0);
    if(pc < 0){
        log_error(log, "El program Counter no puede ser negativo");
    }
    log_info(log, "## PID: <PID> - FETCH - Program Counter: <%d>", pc);
}