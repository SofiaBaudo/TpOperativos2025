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

t_buffer * crear_buffer_de_envio_de_proceso(int pid ,char *ruta_del_archivo, int tamanio){
	t_buffer * buffer_aux = crear_buffer();
	int longitud = strlen(ruta_del_archivo);
	buffer_aux->size = 3*sizeof(int) + longitud;
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); 
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &tamanio, sizeof(int)); 
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &longitud, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, ruta_del_archivo,longitud);
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux->stream;
	return buffer_aux;
}

char *deserializar_nombre_archivo_proceso(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
	stream+= sizeof(int);
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
	buffer_aux -> stream = buffer_aux->stream;
	
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
	buffer_aux -> stream = buffer_aux->stream;
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
	buffer_aux -> stream = buffer_aux->stream;
	return buffer_aux;
}

t_buffer *mandar_pid_a_memoria(int pid){
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux->stream;
	return buffer_aux;
}

t_buffer * devolver_pid_a_kernel(int pid){
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size); //guarda el tamaño del buffer en stream.
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int)); //como un fwrite.
	buffer_aux->offset += sizeof(int);
	buffer_aux -> stream = buffer_aux->stream;
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
	buffer_aux -> stream = buffer_aux->stream;
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
	buffer_aux -> stream = buffer_aux->stream;
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
	buffer_aux -> stream = buffer_aux->stream;
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
	buffer_aux -> stream = buffer_aux->stream;
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

void *deserializar_contenido(t_paquete *paquete) {
    void *stream = paquete->buffer->stream;
    int offset = 0;

    offset += sizeof(int); 
    offset += sizeof(int);

    int longitud;
    memcpy(&longitud, stream + offset, sizeof(int));
    offset += sizeof(int);

    void *contenido = malloc(longitud);
    memcpy(contenido, stream + offset, longitud);
    offset += longitud;

    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

    return contenido;
}

char *deserializar_nombre_io(t_paquete *paquete){
	void *stream = paquete->buffer->stream;
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

//FUNCIONES PARA NUEVAS OPERACIONES

// Crear buffer para solicitud de acceso a tabla de páginas
t_buffer *crear_buffer_acceso_tabla_paginas(int pid, int pagina_logica) {
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = 2*sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pagina_logica, sizeof(int));
	buffer_aux->offset += sizeof(int);
	return buffer_aux;
}

// Crear buffer para respuesta de acceso a tabla de páginas
t_buffer *crear_buffer_respuesta_acceso_tabla_paginas(int marco) {
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &marco, sizeof(int));
	buffer_aux->offset += sizeof(int);
	return buffer_aux;
}

// Crear buffer para solicitud de lectura de página completa
t_buffer *crear_buffer_leer_pagina_completa(int pid, int marco) {
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = 2*sizeof(int);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &marco, sizeof(int));
	buffer_aux->offset += sizeof(int);
	return buffer_aux;
}

// Crear buffer para respuesta de lectura de página completa
t_buffer *crear_buffer_respuesta_leer_pagina_completa(void* contenido, int tam_pagina) {
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = sizeof(int) + tam_pagina;
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &tam_pagina, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, contenido, tam_pagina);
	buffer_aux->offset += tam_pagina;
	return buffer_aux;
}

// Crear buffer para solicitud de actualización de página completa
t_buffer *crear_buffer_actualizar_pagina_completa(int pid, int marco, void* contenido, int tam_pagina) {
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = 3*sizeof(int) + tam_pagina;
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &pid, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &marco, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, &tam_pagina, sizeof(int));
	buffer_aux->offset += sizeof(int);
	memcpy(buffer_aux->stream + buffer_aux->offset, contenido, tam_pagina);
	buffer_aux->offset += tam_pagina;
	return buffer_aux;
}

// Crear buffer para respuesta de actualización de página completa
t_buffer *crear_buffer_respuesta_actualizar_pagina_completa(bool exito) {
	t_buffer *buffer_aux = crear_buffer();
	buffer_aux->size = sizeof(bool);
	buffer_aux->offset = 0;
	buffer_aux->stream = malloc(buffer_aux->size);
	memcpy(buffer_aux->stream + buffer_aux->offset, &exito, sizeof(bool));
	buffer_aux->offset += sizeof(bool);
	return buffer_aux;
}

// Deserializar solicitud de acceso a tabla de páginas
void deserializar_acceso_tabla_paginas(t_paquete *paquete, int *pid, int *pagina_logica) {
	void *stream = paquete->buffer->stream;
	memcpy(pid, stream, sizeof(int));
	stream += sizeof(int);
	memcpy(pagina_logica, stream, sizeof(int));
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

// Deserializar respuesta de acceso a tabla de páginas
int deserializar_respuesta_acceso_tabla_paginas(t_paquete *paquete) {
	void *stream = paquete->buffer->stream;
	int marco;
	memcpy(&marco, stream, sizeof(int));
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	return marco;
}

// Deserializar solicitud de lectura de página completa
void deserializar_leer_pagina_completa(t_paquete *paquete, int *pid, int *marco) {
	void *stream = paquete->buffer->stream;
	memcpy(pid, stream, sizeof(int));
	stream += sizeof(int);
	memcpy(marco, stream, sizeof(int));
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

// Deserializar respuesta de lectura de página completa
void* deserializar_respuesta_leer_pagina_completa(t_paquete *paquete, int *tam_pagina) {
	void *stream = paquete->buffer->stream;
	memcpy(tam_pagina, stream, sizeof(int));
	stream += sizeof(int);
	
	void* contenido = malloc(*tam_pagina);
	memcpy(contenido, stream, *tam_pagina);
	
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	return contenido;
}

// Deserializar solicitud de actualización de página completa
void deserializar_actualizar_pagina_completa(t_paquete *paquete, int *pid, int *marco, void** contenido, int *tam_pagina) {
	void *stream = paquete->buffer->stream;
	memcpy(pid, stream, sizeof(int));
	stream += sizeof(int);
	memcpy(marco, stream, sizeof(int));
	stream += sizeof(int);
	memcpy(tam_pagina, stream, sizeof(int));
	stream += sizeof(int);
	
	*contenido = malloc(*tam_pagina);
	memcpy(*contenido, stream, *tam_pagina);
	
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

// Deserializar respuesta de actualización de página completa
bool deserializar_respuesta_actualizar_pagina_completa(t_paquete *paquete) {
	void *stream = paquete->buffer->stream;
	bool exito;
	memcpy(&exito, stream, sizeof(bool));
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	return exito;
}

//SIN CLASIFICACION

char* instruccion_a_string(op_code codigo) {
    switch(codigo) {
        // HANDSHAKE
        case HANDSHAKE_KERNEL:
            return "HANDSHAKE_KERNEL";
        case HANDSHAKE_CPU:
            return "HANDSHAKE_CPU";
        case HANDSHAKE_MEMORIA:
            return "HANDSHAKE_MEMORIA";
        case HANDSHAKE_IO:
            return "HANDSHAKE_IO";
        case HANDSHAKE_CPU_DISPATCH:
            return "HANDSHAKE_CPU_DISPATCH";
        case HANDSHAKE_CPU_INTERRUPT:
            return "HANDSHAKE_CPU_INTERRUPT";
        case HANDSHAKE_ACCEPTED:
            return "HANDSHAKE_ACCEPTED";
        case HANDSHAKE_DENIED:
            return "HANDSHAKE_DENIED";
        case GET_INSTRUCCION:
            return "GET_INSTRUCCION";
        case SOLICITAR_ESPACIO:
            return "SOLICITAR_ESPACIO";
        case MENSAJE:
            return "MENSAJE";
        case PAQUETE:
            return "PAQUETE";
        case HANDSHAKE:
            return "HANDSHAKE";
        case CREAR_PROCESO:
            return "CREAR_PROCESO";
        case RTA_CREAR_PROCESO:
            return "RTA_CREAR_PROCESO";
        case ENVIO_DE_PID_Y_RUTA_ARCHIVO:
            return "ENVIO_DE_PID_Y_RUTA_ARCHIVO";
        
        // enums para kernel
        case EJECUTAR_RAFAGA_IO:
            return "EJECUTAR_RAFAGA_IO";
        case RAFAGA_ACEPTADA:
            return "RAFAGA_ACEPTADA";
        case IO_NOMBRE:
            return "IO_NOMBRE";
        
        // enums para conexion kernel_memoria
        case INICIALIZAR_PROCESO_DESDE_NEW:
            return "INICIALIZAR_PROCESO_DESDE_NEW";
        case INICIALIZAR_PROCESO_SUSPENDIDO:
            return "INICIALIZAR_PROCESO_SUSPENDIDO";
        case ACEPTAR_PROCESO:
            return "ACEPTAR_PROCESO";
        case RECHAZO_PROCESO:
            return "RECHAZO_PROCESO";
        case FINALIZAR_PROCESO:
            return "FINALIZAR_PROCESO";
        case FINALIZACION_CONFIRMADA:
            return "FINALIZACION_CONFIRMADA";
        case SUSPENDER_PROCESO:
            return "SUSPENDER_PROCESO";
        case SUSPENSION_CONFIRMADA:
            return "SUSPENSION_CONFIRMADA";
        case SOLICITAR_ESPACIO_MEMORIA:
            return "SOLICITAR_ESPACIO_MEMORIA";
        case HAY_ESPACIO_EN_MEMORIA:
            return "HAY_ESPACIO_EN_MEMORIA";
        case NO_HAY_ESPACIO_EN_MEMORIA:
            return "NO_HAY_ESPACIO_EN_MEMORIA";
        
        // enums memoria con cpu
        case SOLICITAR_INSTRUCCION:
            return "SOLICITAR_INSTRUCCION";
        case RESPUESTA_INSTRUCCION:
            return "RESPUESTA_INSTRUCCION";
        case ENVIO_PID_Y_PC:
            return "ENVIO_PID_Y_PC";
        case ENVIO_PID_Y_ENTRADANIVEL:
            return "ENVIO_PID_Y_ENTRADANIVEL";
        case ENVIO_TAMPAG_ENTRADASTABLA_CANTIDADNIVELES:
            return "ENVIO_TAMPAG_ENTRADASTABLA_CANTIDADNIVELES";
        
        // enums instrucciones
        case NOOP:
            return "NOOP";
        case WRITE:
            return "WRITE";
        case READ:
            return "READ";
        case GOTO:
            return "GOTO";
        case IO:
            return "IO";
        case DESALOJO_ACEPTADO:
            return "DESALOJO_ACEPTADO";
        case INIT_PROC:
            return "INIT_PROC";
        case DUMP_MEMORY:
            return "DUMP_MEMORY";
        case EXIT:
            return "EXIT";
        
        // RESPUESTAS
        case DUMP_ACEPTADO:
            return "DUMP_ACEPTADO";
        case DUMP_RECHAZADO:
            return "DUMP_RECHAZADO";
        
        // INTERACCION ENTRE KERNEL E IO
        case RAFAGA_DE_IO:
            return "RAFAGA_DE_IO";
        case FIN_DE_IO:
            return "FIN_DE_IO";
        
        // memoria
        case FETCH_INSTRUCCION:
            return "FETCH_INSTRUCCION";
        case READ_MEMORIA:
            return "READ_MEMORIA";
        case WRITE_MEMORIA:
            return "WRITE_MEMORIA";
        case ENVIO_INSTRUCCION:
            return "ENVIO_INSTRUCCION";
        case ENVIO_PID_DIRFIS_DAT:
            return "ENVIO_PID_DIRFIS_DAT";
        case ENVIO_PID_NROPAG:
            return "ENVIO_PID_NROPAG";
        case ENVIO_PID_NROPAG_CONTENIDO_MARCO:
            return "ENVIO_PID_NROPAG_CONTENIDO_MARCO";
        case ACCESO_TABLA_PAGINAS:
            return "ACCESO_TABLA_PAGINAS";
        case RESPUESTA_ACCESO_TABLA_PAGINAS:
            return "RESPUESTA_ACCESO_TABLA_PAGINAS";
        case LEER_PAGINA_COMPLETA:
            return "LEER_PAGINA_COMPLETA";
        case RESPUESTA_LEER_PAGINA_COMPLETA:
            return "RESPUESTA_LEER_PAGINA_COMPLETA";
        case ACTUALIZAR_PAGINA_COMPLETA:
            return "ACTUALIZAR_PAGINA_COMPLETA";
        case RESPUESTA_ACTUALIZAR_PAGINA_COMPLETA:
            return "RESPUESTA_ACTUALIZAR_PAGINA_COMPLETA";
        
        default:
            return "CODIGO_DESCONOCIDO";
    }
}

op_code obtener_codigo_de_operacion (t_paquete * paquete){
	op_code codigo;
	memcpy(&codigo, &(paquete->codigo_operacion), sizeof(op_code));
	return codigo;
}

