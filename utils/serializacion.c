#include "serializacion.h"

void* serializar_paquete(t_paquete* paquete, int *bytes)
{
	*bytes = sizeof(op_code) + sizeof(paquete->id_correlativo) + sizeof(paquete->id_mensaje)
			+ sizeof(paquete->buffer->size) + paquete->buffer->size;

	void * aEnviar = malloc(*bytes);
	int desplazamiento = 0;

	memcpy(aEnviar + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(aEnviar + desplazamiento, &(paquete->id_correlativo), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(aEnviar + desplazamiento, &(paquete->id_mensaje), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(aEnviar + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(aEnviar + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return aEnviar;
}




/********** serializaciones mas especificas ************/
void serializar_new_pokemon(t_paquete** paquete, void* mensaje){

	int offset = 0;

	t_new_pokemon* new_pokemon = mensaje;
	(*paquete)->buffer->size = sizeof(int)*4 + new_pokemon->size;
	(*paquete)->buffer->stream = malloc((*paquete)->buffer->size);
	memcpy((*paquete)->buffer->stream + offset, &(new_pokemon->size), sizeof(int));
	offset += sizeof(int);
	memcpy((*paquete)->buffer->stream + offset, new_pokemon->nombre, new_pokemon->size);
	offset += new_pokemon->size;
	memcpy((*paquete)->buffer->stream + offset, &(new_pokemon->coordenadas.posx), sizeof(int));
	offset += sizeof(int);
	memcpy((*paquete)->buffer->stream + offset, &(new_pokemon->coordenadas.posy), sizeof(int));
	offset += sizeof(int);
	memcpy((*paquete)->buffer->stream + offset, &(new_pokemon->cantidad), sizeof(int));
}

void serializar_appeared_pokemon(t_paquete** paquete, void* mensaje){

	int offset = 0;

	t_appeared_pokemon* appeared_pokemon = mensaje;
	(*paquete)->buffer->size = sizeof(int)*3 + appeared_pokemon->size;
	(*paquete)->buffer->stream = malloc((*paquete)->buffer->size);
	memcpy((*paquete)->buffer->stream + offset, &(appeared_pokemon->size), sizeof(int));
	offset += sizeof(int);
	memcpy((*paquete)->buffer->stream + offset, appeared_pokemon->nombre, appeared_pokemon->size);
	offset += appeared_pokemon->size;
	memcpy((*paquete)->buffer->stream + offset, &(appeared_pokemon->coordenadas.posx), sizeof(int));
	offset += sizeof(int);
	memcpy((*paquete)->buffer->stream + offset, &(appeared_pokemon->coordenadas.posy), sizeof(int));
}

void serializar_catch_pokemon(t_paquete** paquete, void* mensaje){

	int offset = 0;

	t_catch_pokemon* catch_pokemon = mensaje;
	(*paquete)->buffer->size = sizeof(int)*3 + catch_pokemon->size;
	(*paquete)->buffer->stream = malloc((*paquete)->buffer->size);
	memcpy((*paquete)->buffer->stream + offset, &(catch_pokemon->size), sizeof(int));
	offset += sizeof(int);
	memcpy((*paquete)->buffer->stream + offset, catch_pokemon->nombre, catch_pokemon->size);
	offset += catch_pokemon->size;
	memcpy((*paquete)->buffer->stream + offset, &(catch_pokemon->coordenadas.posx), sizeof(int));
	offset += sizeof(int);
	memcpy((*paquete)->buffer->stream + offset, &(catch_pokemon->coordenadas.posy), sizeof(int));
}

void serializar_caught_pokemon(t_paquete** paquete, void* mensaje){

	t_caught_pokemon* caught_pokemon = mensaje;
	(*paquete)->buffer->size = sizeof(caught_pokemon->resultado);
	(*paquete)->buffer->stream = malloc((*paquete)->buffer->size);
	memcpy((*paquete)->buffer->stream, &(caught_pokemon->resultado), (*paquete)->buffer->size);
}

void serializar_get_pokemon(t_paquete** paquete, void* mensaje){

	int offset = 0;

	t_get_pokemon* get_pokemon = mensaje;
	(*paquete)->buffer->size = sizeof(int) + get_pokemon->size;
	(*paquete)->buffer->stream = malloc((*paquete)->buffer->size);
	memcpy((*paquete)->buffer->stream + offset, &(get_pokemon->size), sizeof(int));
	offset += sizeof(int);
	memcpy((*paquete)->buffer->stream + offset, get_pokemon->nombre, get_pokemon->size);
}

void serializar_localized_pokemon(t_paquete** paquete, void* mensaje){		//NO PODRIA HACER LOS FREE() ACA?? TODO de t_localized_pokemon y t_coordenadas

	int offset = 0;

	t_localized_pokemon* localized_pokemon = mensaje;
	(*paquete)->buffer->size = sizeof(int) + localized_pokemon->size + sizeof(int) +
			sizeof(int) * 2 * (localized_pokemon->coordenadas->elements_count);
	(*paquete)->buffer->stream = malloc((*paquete)->buffer->size);
	memcpy((*paquete)->buffer->stream + offset, &(localized_pokemon->size), sizeof(int));
	offset += sizeof(int);
	memcpy((*paquete)->buffer->stream + offset, localized_pokemon->nombre, localized_pokemon->size);
	offset += localized_pokemon->size;
	memcpy((*paquete)->buffer->stream + offset, &(localized_pokemon->coordenadas->elements_count), sizeof(int));
	offset += sizeof(int);

	int i;
	for(i = 0; i < localized_pokemon->coordenadas->elements_count; i++)
	{
		t_coordenadas* coordenadas = list_get(localized_pokemon->coordenadas, i);
		memcpy((*paquete)->buffer->stream + offset, &(coordenadas->posx), sizeof(int));
		offset += sizeof(int);
		memcpy((*paquete)->buffer->stream + offset, &(coordenadas->posy), sizeof(int));
		offset += sizeof(int);
	}
}




/********************* funciones para deserializar *******************/

int deserializar_identificacion(void* stream){

	int identificacion;

    memcpy(&identificacion, stream, sizeof(int));

	return identificacion;
}

char* deserializar_mensaje(void* stream, int size){

	char* mensaje;

    memcpy(&mensaje, stream, size);

	return mensaje;
}

t_new_pokemon* deserializar_new_pokemon(void *stream){ 	//DESPUES DE USAR ESTA FUNCION
														//NO OLVIDAR HACER FREE() PARA
														//POKEMON Y POKEMON->NOMBRE

	t_new_pokemon* pokemon = malloc(sizeof(t_new_pokemon));

    memcpy(&(pokemon->size), stream, sizeof(int));
    stream += sizeof(int);
    pokemon->nombre = malloc(pokemon->size);
    memcpy(pokemon->nombre, stream, pokemon->size);
    stream += sizeof(pokemon->size);
    memcpy(&(pokemon->coordenadas.posx), stream, sizeof(int));
    stream += sizeof(int);
    memcpy(&(pokemon->coordenadas.posy), stream, sizeof(int));
    stream += sizeof(int);
    memcpy(&(pokemon->cantidad), stream, sizeof(int));

	return pokemon;
}

t_appeared_pokemon* deserializar_appeared_pokemon(void *stream){	//DESPUES DE USAR ESTA FUNCION
																	//NO OLVIDAR HACER FREE() PARA
																	//POKEMON Y POKEMON->NOMBRE

	t_appeared_pokemon* pokemon = malloc(sizeof(t_appeared_pokemon));

    memcpy(&(pokemon->size), stream, sizeof(int));
    stream += sizeof(int);
    pokemon->nombre = malloc(pokemon->size);
    memcpy(pokemon->nombre, stream, pokemon->size);
    stream += sizeof(pokemon->size);
    memcpy(&(pokemon->coordenadas.posx), stream, sizeof(int));
    stream += sizeof(int);
    memcpy(&(pokemon->coordenadas.posy), stream, sizeof(int));
    printf("aca otra cosa %s \n", (char*)(*pokemon).nombre);
	return pokemon;
}

t_catch_pokemon* deserializar_catch_pokemon(void *stream){	//DESPUES DE USAR ESTA FUNCION
															//NO OLVIDAR HACER FREE() PARA
															//POKEMON Y POKEMON->NOMBRE

	t_catch_pokemon* pokemon = malloc(sizeof(t_catch_pokemon));

    memcpy(&(pokemon->size), stream, sizeof(int));
    stream += sizeof(int);
    pokemon->nombre = malloc(pokemon->size);
    memcpy(pokemon->nombre, stream, pokemon->size);
    stream += sizeof(pokemon->size);
    memcpy(&(pokemon->coordenadas.posx), stream, sizeof(int));
    stream += sizeof(int);
    memcpy(&(pokemon->coordenadas.posy), stream, sizeof(int));

	return pokemon;
}

t_caught_pokemon* deserializar_caught_pokemon(void *stream){	//DESPUES DE USAR ESTA FUNCION
																//NO OLVIDAR HACER FREE() PARA
																//POKEMON

	t_caught_pokemon* pokemon = malloc(sizeof(t_caught_pokemon));

    memcpy(&(pokemon->resultado), stream, sizeof(int));

	return pokemon;
}

t_get_pokemon* deserializar_get_pokemon(void *stream){	//DESPUES DE USAR ESTA FUNCION
														//NO OLVIDAR HACER FREE() PARA
														//POKEMON Y POKEMON->NOMBRE

	t_get_pokemon* pokemon = malloc(sizeof(t_get_pokemon));

    memcpy(&(pokemon->size), stream, sizeof(int));
    stream += sizeof(int);
    pokemon->nombre = malloc(pokemon->size);
    memcpy(pokemon->nombre, stream, pokemon->size);

	return pokemon;
}

t_localized_pokemon* deserializar_localized_pokemon(void *stream){ 	//DESPUES DE USAR ESTA FUNCION
																	//NO OLVIDAR HACER FREE() PARA
																	//POKEMON Y POKEMON->NOMBRE
																	//Y list_destroy_and_destroy_elements()
																	//PARA LA LISTA

	t_localized_pokemon* pokemon = malloc(sizeof(t_localized_pokemon));
	int cantidad_pares_de_coordenadas = 0;

    memcpy(&(pokemon->size), stream, sizeof(int));
    stream += sizeof(int);
    pokemon->nombre = malloc(pokemon->size);
    memcpy(pokemon->nombre, stream, pokemon->size);
    stream += sizeof(pokemon->size);
    memcpy(&(cantidad_pares_de_coordenadas), stream, sizeof(int));
    stream += sizeof(int);

    pokemon->coordenadas = list_create();

    int i;
    for(i = 0; i < cantidad_pares_de_coordenadas ; i++)
    {
        t_coordenadas* coordenadas = malloc(sizeof(t_coordenadas));

    	memcpy(&(coordenadas->posx), stream, sizeof(int));
    	stream += sizeof(int);
   	    memcpy(&(coordenadas->posy), stream, sizeof(int));
   	    stream += sizeof(int);
   	    list_add(pokemon->coordenadas, coordenadas);
    }

	return pokemon;
}
