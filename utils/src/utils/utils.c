#include <utils/utils.h>

// CONFIG
t_config *crear_config(char* direccion){
    t_config *nuevo_config = config_create(direccion); // se pasa la ruta del archivo .config
    if(!nuevo_config){
        perror("No se pudo crear el config");
        exit(EXIT_FAILURE);
    }
    return nuevo_config;
}

void destruir_config(t_config *config){
	config_destroy(config);
}

// LOGS

t_log *iniciar_logger(char* archivoLog, char* nombreLog){
    t_log *nuevo_logger = log_create(archivoLog,nombreLog,1,LOG_LEVEL_INFO);
    if(!nuevo_logger){
        perror("No se pudo crear el logger");
        exit(EXIT_FAILURE);
    }
    return nuevo_logger;
} //EN REALIDAD LOS LOGS ES MEJOR IR DEFINIENDOLOS A CONVENIENCIA PORQUE TIENEN DIFERENTES NIVELES

void destruir_logger(t_log *logger){
    log_destroy(logger);
}

//CONEXIONES

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket (server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);


	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_servidor(char *puerto, t_log *un_log, char *mensaje)
{
	int socket_servidor;
	struct addrinfo hints, *servinfo; //*p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
 	socket_servidor = socket (servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	// Asociamos el socket a un puerto
		bind(socket_servidor,servinfo->ai_addr,servinfo->ai_addrlen);
	// Escuchamos las conexiones entrantes
	listen(socket_servidor,SOMAXCONN);

	freeaddrinfo(servinfo);
	log_info(un_log, "SERVER: %s",mensaje);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log *un_log, char *mensaje)
{
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(un_log,"Se conecto el cliente: %s",mensaje);
	return socket_cliente;
}

//ENVIO DE DATOS
void enviar_entero(int socket_cliente, int numero)
{
	send(socket_cliente, &numero, sizeof(numero), MSG_WAITALL);
}

void enviar_mensaje(int socket_cliente, char* mensaje) {
	int longitud = strlen(mensaje) + 1; //obtener log
	send(socket_cliente, &longitud, sizeof(int), 0); // no se si esta bien pero es para que el receptor sepa cuantos bytes debe leer la longitud del mensaje
	send(socket_cliente, mensaje, longitud, 0); 
}

void enviar_op_code(int socket_cliente, op_code codigo_operacion)
{
	enviar_entero(socket_cliente, codigo_operacion);
}

void crear_paquete(op_code codigo, t_buffer *buffer, int socket){
	
	//llenamos el paquete con el buffer
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo; // Podemos usar una constante por operación
	paquete->buffer = buffer; // Nuestro buffer de antes.
	//armamos el stream a enviar
	void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code)+sizeof(int)); // el tamanio que hay que reservar segun el buffer + el tamanio del op_code + el tamanio del buffer
	paquete->buffer->offset = 0;
	
	memcpy((char *)a_enviar + paquete->buffer->offset, &(paquete->codigo_operacion), sizeof(op_code));
	paquete->buffer->offset += sizeof(op_code);
	memcpy((char *)a_enviar + paquete->buffer->offset, &(paquete->buffer->size), sizeof(int)); // el tamanio del buffer
	paquete->buffer->offset += sizeof(int);
	memcpy((char *)a_enviar + paquete->buffer->offset, paquete->buffer->stream, paquete->buffer->size);
	//enviamos con send()
	send(socket,(char*)a_enviar, buffer-> size + sizeof(op_code) + sizeof(int),0);
	
	//liberamos la memoria
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

//RECIBIR DATOS
int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}


int recibir_entero(int fd_conexion){
	int entero;
	recv(fd_conexion, &entero, sizeof(entero), MSG_WAITALL);
	return entero;
}


op_code recibir_op_code (int socket)
{
	op_code codigo_operacion;
	recv(socket, &codigo_operacion, sizeof(codigo_operacion), 0);
	return codigo_operacion;
}

t_paquete* recibir_paquete(int socket){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	
	// Primero recibimos el codigo de operacion
	recv(socket, &paquete->codigo_operacion, sizeof(paquete->codigo_operacion), 0);
	
	// Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
	recv(socket, &(paquete->buffer->size), sizeof(int), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);
	return paquete;
}
//CREACION DE BUFFERS
t_buffer *crear_buffer(){
	t_buffer *buffer_aux = malloc (sizeof(t_buffer));
	return buffer_aux;
}

t_buffer *crear_buffer_vacio(){
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = 0;
	buffer_aux->offset = 0;
	buffer_aux->stream = NULL;
	return buffer_aux;	
}

t_buffer * crear_buffer_para_ejecucion_de_io(int pid, int milisegundos){ //esto se lo manda kernel a cpu
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = 2*sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &milisegundos, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux-> stream;
	return buffer_aux;
}

t_buffer * crear_buffer_MarcoMem(int pid, int entradaNivel){
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = 2*sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &entradaNivel, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux-> stream;
	return buffer_aux;
}

t_buffer *mandar_pid_a_memoria(int pid){
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux-> stream;
	return buffer_aux;
}

t_buffer * devolver_pid_a_kernel(int pid){
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux-> stream;
	return buffer_aux;
}
t_buffer *crear_buffer_tamPag_entradasTabla_cantNiveles(int tamPag, int entradasTabla, int cantNiveles){
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = 3*sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &tamPag, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &entradasTabla, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &cantNiveles, sizeof(int));
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux-> stream;
	return buffer_aux;
}

t_buffer *crear_buffer_pid_entradaNivel(int pid, int entradaNivel){
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = 2*sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &entradaNivel, sizeof(int));
	buffer_aux->offset += sizeof(int);
	return buffer_aux;
}

t_buffer * crear_buffer_instruccion_init_proc(char* ruta_del_archivo, int tamanio_en_memoria, int *pid, int *pc){
	t_buffer *buffer_aux = crear_buffer();
	int longitud = strlen(ruta_del_archivo);
	buffer_aux->size = 4*sizeof(int) + longitud; 
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pc, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &tamanio_en_memoria, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &longitud, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, ruta_del_archivo,longitud);
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux-> stream;
	
	return buffer_aux;
}

t_buffer * crear_buffer_instruccion_io (char* nombre, int milisegundos, int *pid, int *pc){
	t_buffer *buffer_aux = crear_buffer();
	int longitud = strlen(nombre);
	buffer_aux->size = 4*sizeof(int) + longitud;
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.
	
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pc, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &milisegundos, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &longitud, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, nombre,longitud);
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux-> stream;
	return buffer_aux;
}

t_buffer *crear_buffer_io_nombre(char *nombre){
	t_buffer *buffer_aux = crear_buffer();
	int longitud = strlen(nombre);
	buffer_aux->size = sizeof(int) + longitud;
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.
	
	memcpy(buffer_aux->stream + buffer_aux->offset, &longitud, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, nombre,longitud);
	
	buffer_aux -> stream = buffer_aux-> stream;
	return buffer_aux;
}

t_buffer *crear_buffer_pid_dirFis_datos(int pid, int dirFis, char* datos){
	t_buffer *buffer_aux = crear_buffer();
	int valor1 = 2*sizeof(int);
	int longitud = strlen(datos);
	buffer_aux->size = valor1 + longitud;
	buffer_aux->offset = 0;
	memcpy(buffer_aux->stream + buffer_aux->offset, &longitud, sizeof(int)); 
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, datos,longitud);
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); 
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &dirFis, sizeof(int)); 
	buffer_aux->offset += sizeof(int);
	return buffer_aux;
}
t_buffer *crear_buffer_pid_numPag(int pid, int nroPag){
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = 2*sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &nroPag, sizeof(int));
	buffer_aux->offset += sizeof(int);
	return buffer_aux;
}
t_buffer *crear_buffer_pid_numPag_contenido_marco(int pid, int nroPag, void* contenido, int marco, int tamPag){
	t_buffer *buffer_aux = crear_buffer();
	int longitud = tamPag;
	buffer_aux->size = 3*sizeof(int) + longitud + sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &nroPag, sizeof(int));
	buffer_aux->offset += sizeof(int);
	
	memcpy(buffer_aux->stream + buffer_aux->offset, &longitud, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, contenido, longitud);
	buffer_aux->offset += longitud;	
	memcpy(buffer_aux->stream + buffer_aux->offset, &marco, sizeof(int));
	buffer_aux->offset += sizeof(int);
	return buffer_aux;
	// el contendio tiene que entrar en tampag entonces podemos ponerlo como tampag para sacar el size?
}

t_buffer * crear_buffer_cpu(int pid, int pc){ //esto se lo manda kernel a cpu
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = 2*sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pc, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux-> stream;
	return buffer_aux;
}

/*t_buffer* crear_buffer_instruccion(t_instruccion* instr){
	t_buffer* buffer = crear_buffer();
	int size_total = sizeof(op_code) + sizeof(int);  // codigo + cantidad de parametros
	//calculo el tamaño
	for (int i = 0; i < instr->cantidad_parametros; i++) {
		size_total += sizeof(int);
		size_total += strlen(instr->parametros[i]) + 1;
	}
	buffer->size = size_total;
	buffer->offset = 0;
	buffer->stream = malloc(size_total);
	memcpy(buffer->stream + buffer->offset, &(instr->codigo), sizeof(op_code));
	buffer->offset += sizeof(op_code);
	memcpy(buffer->stream + buffer->offset, &(instr->cantidad_parametros), sizeof(int));
	buffer->offset += sizeof(int);
	for (int i = 0; i < instr->cantidad_parametros; i++) {
		int len = strlen(instr->parametros[i]) + 1;
		memcpy(buffer->stream + buffer->offset, &len, sizeof(int));
		buffer->offset += sizeof(int);
		memcpy(buffer->stream + buffer->offset, instr->parametros[i], len);
		buffer->offset += len;
	}
	return buffer;
}
*/
//DESERIALIZACIONES

/*void *deserializar_contenido(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	int longitud;
	int offset = 0;
	offset += sizeof(int);
    offset += sizeof(int);
    int data;
	stream+=sizeof(int);
    memcpy(&data,stream + offset,sizeof(int));
    offset+=sizeof(int);
   void *contenido = malloc(longitud);
    memcpy(contenido, stream + offset, longitud);
    offset += longitud;
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	return NULL;
}*/

char *deserializar_nombre_io(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
    int longitud;
	stream+=sizeof(int);
    memcpy(&longitud,stream,sizeof(int));
    stream+=sizeof(int);
    char *nombre = malloc(longitud+1);
    memcpy(nombre,stream,longitud);
	free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	return nombre;
}

char *deserializar_nombre_syscall_io(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
    int longitud;
	stream+=3*sizeof(int); //para "saltear" los milisegundos, el pid y el pc que vienen primero
    memcpy(&longitud,stream,sizeof(int));
    stream+=sizeof(int);
    char *nombre = malloc(longitud+1);
    memcpy(nombre,stream,longitud);
	free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	return nombre;
}

void deserializar_pid_y_pc(t_paquete *paquete, int *pid, int *pc){
	void *stream = paquete->buffer->stream;
    memcpy(pid,stream,sizeof(int));
	stream+=sizeof(int);
	memcpy(pc,stream,sizeof(int));
	free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

int deserializar_pid(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	int pid;
    memcpy(&pid,stream,sizeof(int));
	free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	return pid;
}

void deserializar_config_memoria(t_paquete *paquete, int* tamPag, int* entradasTabla, int* cantNiveles) {
	void* stream = paquete->buffer->stream;
	
	memcpy(tamPag, stream, sizeof(int));
	stream += sizeof(int);
	
	memcpy(entradasTabla, stream, sizeof(int));
	stream += sizeof(int);
	
	memcpy(cantNiveles, stream, sizeof(int));
	
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

int deserializar_pc(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	stream+=sizeof(int);
	int pc;
	fprintf(stderr, "Puntero nulo en deserializar_pc\n");
	memcpy(&pc,stream,sizeof(int));
	fprintf(stderr, "Ya pase el memcpy");
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	return pc;
}

int deserializar_entradaNivel(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	stream+=sizeof(int);
	int entradaNivel;
	memcpy(&entradaNivel, stream, sizeof(int));
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	return entradaNivel;
}

/*
int deserializar_pc(t_paquete *paquete) {
    if (!paquete || !paquete->buffer || !paquete->buffer->stream) {
        fprintf(stderr, "Puntero nulo en deserializar_pc\n");
        return -1;
		}
		
		int *stream = (int *)paquete->buffer->stream;
		
		int pc = stream[1];  // asumimos que el primer int es otra cosa, el segundo es el pc
		
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
		
		return pc;
		}
*/
		
		
int deserializar_nroPag(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	int nroPag;
	memcpy(&nroPag,stream,sizeof(int));
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	return nroPag;
}
		
int deserializar_marco(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	int marco;
	memcpy(&marco,stream,sizeof(int));
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	return marco;
}

int deserializar_dirFis(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	int dirFis;
	memcpy(&dirFis,stream,sizeof(int));
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	return dirFis;
}

char *deserializar_dataIns(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
    int data;
	stream+=sizeof(int);
    memcpy(&data,stream,sizeof(int));
    stream+=sizeof(int);
    char *nombre = malloc(data+1);
    memcpy(nombre,stream,data);
	free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	return nombre;
}

int deserializar_tamanio(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	stream+=2*sizeof(int);
    int tamanio;
    memcpy(&tamanio,stream,sizeof(int));
	free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	return tamanio;
}
int deserializar_cant_segundos(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	stream+=2*sizeof(int);
    int tamanio;
    memcpy(&tamanio,stream,sizeof(int));
	free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	return tamanio;
}
char *deserializar_nombre_archivo(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	stream+=3*sizeof(int);
    int longitud;
    memcpy(&longitud,stream,sizeof(int));
    stream+=sizeof(int);
    char *nombre = malloc(longitud+1);
    memcpy(nombre,stream,longitud);
	free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	return nombre;
}
//para lo de deserializar init proc viene primero el tamanio en memoria

int deserializar_entero_desde_stream(t_paquete* paquete){ //lee un int desde el buffer de un paquete recibido por socket, y avanza el offset interno del buffer para que puedas seguir leyendo otros datos
	int valor;
	memcpy(&valor, paquete->buffer->stream + paquete->buffer->offset, sizeof(int));
	paquete->buffer->offset += sizeof(int);
	return valor;
}

//SIN CLASIFICACION

char* instruccion_a_string(op_code codigo){
	switch (codigo) {
        case NOOP:
		return strdup("NOOP");
        case READ:
		return strdup("READ");
        case WRITE:
		return strdup("WRITE");
        case IO:
		return strdup("IO");
        case EXIT:
		return strdup("EXIT");
        case INIT_PROC:
		return strdup("INIT_PROC");
        case DUMP_MEMORY:
		return strdup("DUMP_MEMORY");
        default:
		return strdup("INSTRUCCION_DESCONOCIDA");
    }
}

op_code obtener_codigo_de_operacion (t_paquete * paquete){
	op_code codigo;
	memcpy(&codigo, &(paquete->codigo_operacion), sizeof(op_code));
	return codigo;
}
