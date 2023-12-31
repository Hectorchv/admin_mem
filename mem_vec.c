/*
    * Autores:
    * Chávez Mejía Luis Héctor
    * Escobar Flores Daniel
    * 
    * Objetivo: 
    * Realizar un programa en C que efectúe la administración
    * de memoria por paginación, utilizando un vector “de áreas
    * libres” (el que usa Linux) tanto para la asignación como 
    * para liberación de marcos de página de memoria
    * 
    * 
    * PASOS DE COMPILACION Y EJECUCIÓN:
    * 
    * Se utiliza la biblioteca math.h
    * gcc -o mem_vec -lm  mem_vec.c
    * 
    * El nombre archivo se pasa como parámetro al programa
    * ./mem_vec procesos.txt
    * 
    * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#ifndef MEM
#define MARCOS_MEM 16
#define ENT_VECTOR 5
#endif

// Defninición de la lista que utilizará
// el vector de áreas libres
typedef struct lista
{
    struct lista *next;
    int marcLibre;
} mem_vec;

// Operación insertar en la lista
void insertarList(mem_vec **lista, int dato)
{
    mem_vec *tmp = malloc(sizeof(mem_vec));
    if (tmp != NULL)
    {
        tmp->marcLibre = dato;
        if (*lista != NULL)
            tmp->next = *lista;
        else
            tmp->next = NULL;
        *lista = tmp;
    }
}

// Operación de eliminar en la lista
// (primer elemento encontrado)
void eliminarList(mem_vec **lista)
{
    mem_vec *tmp = *lista;
    *lista = (*lista)->next;
    free(tmp);
}

// Estrae cualquier elemeto de la lista
// identificado por su valor en marco
int extraeList(mem_vec **lista, int marco)
{
    int dev = 0;
    mem_vec *anterior, *tmp;
    tmp = anterior = (*lista);
    if (tmp != NULL)
    {
        if (tmp->marcLibre == marco)
        {
            dev = 1;
            *lista = (*lista)->next;
            free(tmp);
            tmp = anterior = NULL;
        }
        else
        {
            tmp = tmp->next;
            while (tmp != NULL)
            {
                if (tmp->marcLibre == marco)
                {
                    anterior->next = tmp->next;
                    tmp->next = NULL;
                    free(tmp);
                    dev = 1;
                    tmp = anterior = NULL;
                    break;
                }
                else
                {
                    tmp = tmp->next;
                    anterior = anterior->next;
                }
            }
        }
    }
    return dev;
}

// Agrega el pid al marco de página señalado
// a la memoria
void cargarMem(int memoria[MARCOS_MEM], int marcoLibre, int tam, int pid)
{
    for (int i = 0; i < tam; i++)
    {
        memoria[marcoLibre + i] = pid;
    }
}

// Función que busca un espacio en el vector de
// areas libre. Si no lo encuentra, intenta crearlo
// dividiendo areas de memoria en el tamaño requerido
int buscEspacio(mem_vec *vectorMLibres[ENT_VECTOR], int tam)
{
    int encontrado = 0;
    int indiMarc = (int)log2(tam);

    // Encontramos una area libre del mismo
    // tamaño que requiere el proceso
    if (vectorMLibres[indiMarc] != NULL)
        encontrado = 1;

    // No encontramos uns area libre del mismo
    // tamaño requerido por el proceso
    else
    {
        for (int i = indiMarc + 1; i < ENT_VECTOR; i++)
        {
            // Encontramos una area del mayor tamaño
            // requerida por el proceso
            if (vectorMLibres[i] != NULL)
            {
                // Dividimos el area cada vez en dos partes
                // hasta que nos quede una del tamaño requerido
                int tamArea = i;
                mem_vec *tmp = vectorMLibres[tamArea];
                while (tamArea != indiMarc)
                {
                    int marcoLibreA = tmp->marcLibre;
                    int marcoLibreB = tmp->marcLibre + (pow(2, tamArea - 1));
                    eliminarList(&(vectorMLibres[tamArea]));
                    tamArea--;
                    insertarList(&(vectorMLibres[tamArea]), marcoLibreA);
                    insertarList(&(vectorMLibres[tamArea]), marcoLibreB);
                    tmp = vectorMLibres[tamArea];
                }
                encontrado = 1;
                break;
            }
        }
    }
    return encontrado;
}

// Función encargada de unir espación libres de memoria
// contiguos que cumplan con la condición de unión
int reunificarMem(int indiMarc, int tam, mem_vec *vectorMlibres[ENT_VECTOR])
{
    int entrada = (int)log2(tam);
    int marcoAUnir = 0;

    if (((indiMarc / tam) % 2) == 0)
        marcoAUnir = indiMarc + tam;
    else
        marcoAUnir = indiMarc - tam;

    if (tam == MARCOS_MEM)
    {
        extraeList(&(vectorMlibres[entrada]), indiMarc);
        insertarList(&(vectorMlibres[entrada]), indiMarc);
    }
    else
    {
        if (extraeList(&(vectorMlibres[entrada]), marcoAUnir) != 0)
        {
            extraeList(&(vectorMlibres[entrada]), indiMarc);
            insertarList(&(vectorMlibres[entrada + 1]), ((marcoAUnir < indiMarc) ? marcoAUnir : indiMarc));
            reunificarMem(((marcoAUnir < indiMarc) ? marcoAUnir : indiMarc), tam * 2, vectorMlibres);
        }
        else
            insertarList(&(vectorMlibres[entrada]), indiMarc);
    }
    return 0;
}

// Función que deasigna de memoria un proceso
// recibe el pid del que se quiere eliminar de la memoria
int desasignarMem(int pid, int memoria[MARCOS_MEM], mem_vec *vectorMlibres[ENT_VECTOR])
{
    int indiMarc = -1;
    int tam = 0;
    int flag = 0;
    for (int i = 0; i < MARCOS_MEM; i++)
    {
        if (memoria[i] == pid)
        {
            if (flag == 0)
                indiMarc = i;
            tam++;
            flag = 1;
            memoria[i] = -1;
        }
        else if (flag == 1)
        {
            reunificarMem(indiMarc, tam, vectorMlibres);
            flag = 0;
            tam = 0;
        }
    }
    if (flag == 1)
        reunificarMem(indiMarc, tam, vectorMlibres);
    return 1;
}

// Asigna espacio de memoria a un proceso
// recibe el pid del proceso, así como el tamaño
// que este ocupará
int asignarMem(int pid, int tam, mem_vec *vectorMLibres[ENT_VECTOR], int memoria[MARCOS_MEM])
{
    int asignado = 0;
    int marcoLibre, a, b;
    int indiVector = (int)log2(tam);

    int espacio = buscEspacio(vectorMLibres, tam);
    if (espacio == 1)
    {
        marcoLibre = vectorMLibres[indiVector]->marcLibre;
        eliminarList(&(vectorMLibres[indiVector]));
        cargarMem(memoria, marcoLibre, tam, pid);
        asignado = 1;
    }
    else if (tam > 1)
    {
        a = asignarMem(pid, tam / 2, vectorMLibres, memoria);
        b = asignarMem(pid, tam / 2, vectorMLibres, memoria);
    }
    else if (a == 0 || b == 0)
    {
        fprintf(stderr, "Sin espacio en memoria");
    }

    return asignado;
}


//Función principal
int main(int argc, char *argv[])
{
    int pid, tam;
    FILE *archivo;
    archivo = fopen(argv[1], "r");
    assert(archivo != NULL);

    //Inicializamos la memoria
    int memoria[MARCOS_MEM];
    for (int i = 0; i < MARCOS_MEM; i++)
    {
        memoria[i] = -1;
    }

    // Inicializamos el vector de areas libres
    mem_vec *vectorMLibres[ENT_VECTOR];
    for (int i = 0; i < ENT_VECTOR; i++)
        vectorMLibres[i] = NULL;
    insertarList(&(vectorMLibres[4]), 0);

    while (!feof(archivo))
    {

        fscanf(archivo, "%d %d", &pid, &tam);
        fgetc(archivo);

        if (tam == -1)
            desasignarMem(pid, memoria, vectorMLibres);
        else
            asignarMem(pid, tam, vectorMLibres, memoria);

        printf("*****************************************\n");
        printf("Situación de la memoria\n\n");
        printf("N marc\tpid\n");
        for (int i = 0; i < MARCOS_MEM; i++)
        {
            printf("|%d\t|", i);
            if(memoria[i] == -1)
                printf(" LIBRE|");
            else
                printf(" %5d|", memoria[i]);
            printf("\n");
        }
        printf("----------------------------------------\n");
        printf("Situacion del vector de areas libres\n\n");
        printf("Entrada\tMarcos libres\n");
        for (int i = 0; i < ENT_VECTOR; i++)
        {
            mem_vec *tmp = vectorMLibres[i];
            printf("|%d\t|",i);
            while (tmp != NULL)
            {
                printf("->%d ", tmp->marcLibre);
                tmp = tmp->next;
            }
            printf("\n");
        }
    }
    fclose(archivo);
}

/**
 * 
 * Conclusiones:
 * 
 * Luis Héctor Chávez Mejía:
 * Con el desarrollo de este programa fuí capaz de implementar
 * mis conocimientos del tema de administración de memoria, ya
 * que implementé la asignación de memoria con ayuda de un ve-
 * tor de áreas libres, además de segmentar el proceso con el
 * objetivo de que este entrara en memoría. Además implementé
 * el concepto de marcos de página
 * Apesar de que fué un programa dificil de implementar, logré
 * aprender todo lo necesario del teme de administración de me-
 * moria, con lo que puedo decir que he logrado mi objetivo 
 * principal.
 * 
 * Escobar Flores Daniel:
 * Finalmente se realizó correctamente el funcionamiento del
 * programa que se tenia como objetivo, se realizó con diferentes
 * funciones para la función de liberar marcos de página de 
 * memoria, de este modo nos ayuda a que se ejecute completamente,
 * incluyendo algunas librerías extras.
*/