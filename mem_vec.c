#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<assert.h>

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

// Eliminar un elemento en la lista de los
// vectores de áreas libre
void eliminarList(mem_vec **lista)
{
    mem_vec *tmp = *lista;
    *lista = (*lista)->next;
    free(tmp);
}

int extraeList(mem_vec **lista, int marco)
{
    int dev = 0;
    mem_vec *anterior, *tmp;
    tmp = anterior = (*lista);
    if (tmp->marcLibre == marco)
    {
        dev = 1;
        *lista = (*lista)->next;
        free(tmp);
    }
    else
    {
        tmp = tmp->next;
        while (tmp != NULL)
        {
            if(tmp->marcLibre == marco)
            {
                anterior->next = tmp->next;
                tmp->next = NULL;
                free(tmp);
                dev = 1;
                break;
            }
            else
            {
                tmp = tmp->next;
                anterior = anterior->next;
            }
        }
    }
    return dev;
}

// Agrega el pid al marco de página señalado
// a la memoria
void cargarMem(int memoria[MARCOS_MEM],int marcoLibre,int tam, int pid)
{
    for (int i = 0; i < tam; i++)
    {
        memoria[marcoLibre+i] = pid;
    }
}

// Función que busca un espacio en el vector de
// areas libre.
//
// devuelve 1 si lo encontró
// devuelve 0 si no lo encontró
// 
// @param vectorMLibres vector de areas libres
// @param tam marcos requeridos por el proceso
int buscEspacio(mem_vec *vectorMLibres[ENT_VECTOR], int tam)
{
    int encontrado = 0;
    int indiMarc = (int) log2(tam);

    // Encontramos una area libre del mismo
    // tamaño que requiere el proceso
    if(vectorMLibres[indiMarc] != NULL)
        encontrado = 1;
    
    // No encontramos uns area libre del mismo
    // tamaño requerido por el proceso
    else
    {
        for(int i=indiMarc+1;i<ENT_VECTOR;i++)
        {
            // Encontramos una area del mayor tamaño
            // requerida por el proceso
            if(vectorMLibres[i]!=NULL)
            {
                // Dividimos el area cada vez en dos partes
                // hasta que nos quede una del tamaño requerido
                int tamArea = i;
                mem_vec *tmp = vectorMLibres[tamArea];
                while (tamArea != indiMarc)
                {
                    int marcoLibreA = tmp->marcLibre;
                    int marcoLibreB = tmp->marcLibre+(pow(2,tamArea-1));
                    eliminarList(&(vectorMLibres[tamArea]));
                    tamArea --;
                    insertarList(&(vectorMLibres[tamArea]),marcoLibreA);
                    insertarList(&(vectorMLibres[tamArea]),marcoLibreB);
                    tmp=vectorMLibres[tamArea];
                }
                encontrado = 1;
                break;
            }
        }
    }
    return encontrado;
}

int reunificarMem(int indiMarc, int tam, mem_vec *vectorMlibres[ENT_VECTOR])
{
    int entrada = (int) log2(tam);

    if(tam == MARCOS_MEM)
    {
        extraeList(&(vectorMlibres[entrada]), indiMarc);
        insertarList(vectorMlibres, indiMarc);
    }
    else
    {
        int marcoAUnir = ((indiMarc/(tam*2))*(tam*2)+(tam));
        if (extraeList(&(vectorMlibres[entrada]),marcoAUnir) != 0)
        {
            insertarList(&(vectorMlibres[entrada+1]), (marcoAUnir<indiMarc ? marcoAUnir : indiMarc));
            reunificarMem((marcoAUnir<indiMarc ? marcoAUnir : indiMarc), tam * 2, vectorMlibres);
        }
    }
    return 0;
}

int desasignarMem(int pid, int memoria[MARCOS_MEM], mem_vec *vectorMlibres[ENT_VECTOR])
{
    int indiMarc = -1;
    int tam = 0;
    int flag = 0;
    for (int i = 0; i < MARCOS_MEM; i++)
    {
        if(memoria[i] == pid)
        {
            if(flag == 0)
                indiMarc = i;
            tam ++;
            flag = 1;
            memoria[i] = -1;
        }
        else if(flag == 1)
        {
            reunificarMem(indiMarc, tam, vectorMlibres);
            flag = 0;
            tam = 0;
        }
    }
    return 1;
}

int asignarMem(int pid, int tam, mem_vec *vectorMLibres[ENT_VECTOR], int memoria[MARCOS_MEM])
{
    int asignado = 0;
    int marcoLibre, a, b;
    int indiVector = (int) log2(tam);
    
    int espacio = buscEspacio(vectorMLibres, tam);
    if (espacio == 1)
    {
        marcoLibre = vectorMLibres[indiVector]->marcLibre;
        eliminarList(&(vectorMLibres[indiVector]));
        cargarMem(memoria,marcoLibre,tam,pid);
        asignado = 1;
    }
    else if (tam > 1)
    {
        a = asignarMem(pid, tam/2, vectorMLibres, memoria);
        b = asignarMem(pid, tam/2, vectorMLibres, memoria);
    }
    else if(a == 0 || b ==0)
    {
        fprintf(stderr, "Sin espacio en memoria");
    }

    return asignado;
}

int main(int argc, char *argv[])
{
    //int pid, tam;
    FILE *archivo;
    archivo = fopen(argv[1], "r");
    assert(archivo != NULL);

    int memoria[MARCOS_MEM];
    for (int i = 0; i < MARCOS_MEM; i++)
    {
        memoria[i] = -1;
    }
    
    //Vector de areas libre
    mem_vec *vectorMLibres[ENT_VECTOR];

    // Inicializamos el vector de areas libre
    // Con todos los apuntadores a NULL
    for (int i = 0; i < ENT_VECTOR; i++)
    {
        vectorMLibres[i] = NULL;
    }
    // Colocamos el primer elemento de tamaño 16
    // al vector de áreas libres para inicializar
    /* insertarList(&(vectorMLibres[0]), 1);
    insertarList(&(vectorMLibres[0]), 0);
    insertarList(&(vectorMLibres[1]), 2);
    insertarList(&(vectorMLibres[2]), 4);
    insertarList(&(vectorMLibres[3]), 8); */ 
    insertarList(&(vectorMLibres[1]), 2);
    insertarList(&(vectorMLibres[1]), 4);

    /* while (!feof(archivo))
    {

        fscanf(archivo, "%d %d", &pid, &tam);
        fgetc(archivo);

        printf("pid: %d tam: %d\n", pid , tam);
    } */

    //asignarMem(150, 1, vectorMLibres, memoria);
    printf("hola mundo\n");
    
    //desasignarMem(150, memoria, vectorMLibres);
    //asignarMem(150, 8, vectorMLibres, memoria);
    //asignarMem(120, 8, vectorMLibres, memoria);
    //desasignarMem(150, memoria, vectorMLibres);
    //reunificarMem(8,4,vectorMLibres);

    fclose(archivo);
}