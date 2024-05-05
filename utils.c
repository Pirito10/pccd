#include <sys/msg.h>

#include "utils.h" // Archivo de cabecera con la definición de las funciones y estructura de los mensajes

#define TOKEN 1
#define REQUEST 2

// Variables globales en nodo.c
extern int id, quiere[3];                                   // ID del nodo y vector de procesos que quieren SC por cada prioridad
extern int vector_peticiones[3][N], vector_atendidas[3][N]; // Cola de solicitudes por atender y cola de solicitudes atendidas

/**
 * Envía el token a otro nodo especificado
 * @param id_nodo ID del nodo al que se envía el token
 */
void enviar_token(int id_nodo)
{
    // Creamos el mensaje
    struct msg_nodo msg_nodo = (const struct msg_nodo){0};
    msg_nodo.mtype = TOKEN;
    msg_nodo.id_nodo_origen = id;
    for (int j = 0; j < 3; j++)
    {
        for (int i = 0; i < N; i++)
        {
            // Introducimos el vector de atendidas en el mensaje
            msg_nodo.vector_atendidas[j][i] = vector_atendidas[j][i];
        }
    }

    // Enviamos el mensaje con el testigo
    int msgid = msgget(1000 + id_nodo, 0666);
    msgsnd(msgid, &msg_nodo, sizeof(msg_nodo), 0);
}

/**
 * Broadcastea una request de la prioridad aportada a todos los otros nodos del SD
 * @param prioridad prioridad de la request a broadcastear
 */
void broadcast(int prioridad)
{
    // Creamos el mensaje de solicitud
    struct msg_nodo msg_nodo = (const struct msg_nodo){0};
    msg_nodo.mtype = REQUEST;
    msg_nodo.id_nodo_origen = id;
    vector_peticiones[prioridad][id]++;
    msg_nodo.num_peticion_nodo_origen = vector_peticiones[prioridad][id];
    msg_nodo.prioridad_origen = prioridad;

    // Lo enviamos a cada nodo
    for (int i = (id + 1) % N; i != id; i = (i + 1) % N)
    {
        int msgid = msgget(1000 + i, 0666);
        msgsnd(msgid, &msg_nodo, sizeof(msg_nodo), 0);
    }
}

/**
 * Sustituye los valores del vector de atendidas por los del vector aportado como parámetro
 * @param vector_atendidas_nuevo nuevo vector de atendidas para el nodo
 */
void actualizar_atendidas(int vector_atendidas_nuevo[3][N])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < N; j++)
        {
            vector_atendidas[i][j] = vector_atendidas_nuevo[i][j];
        }
    }
}

/**
 * Determina el ID del nodo siguiente teniendo en cuenta el vector de peticiones, atendidas y quiere.
 * En caso de existir una petición más prioritaria de otro nodo que las que esperan en su nodo devuelve el ID del nodo que ha hecho la petición prioritaria
 *
 * @return ID del nodo con la petición prioritaria, o -1 en caso de no existir
 */
int buscar_nodo_siguiente()
{
    int prioridad_este_nodo = 3;
    for (int i = 0; i < 3; i++)
    {
        if (quiere[i] > 0)
        {
            prioridad_este_nodo = i;
        }
    }
    for (int i = 0; i < prioridad_este_nodo; i++)
    {
        for (int j = (id + 1) % N; j != id; j = (j + 1) % N)
        {
            if (vector_peticiones[i][j] > vector_atendidas[i][j])
            {
                return j;
            }
        }
    }
    return -1;
}

/**
 * Comprueba si hay una petición activa para la prioridad dada en el nodo
 * @param prioridad prioridad de la petición
 * @return 1 si hay una petición activa, 0 en caso contrario
 */
int peticion_activa(int prioridad)
{
    return vector_peticiones[prioridad][id] > vector_atendidas[prioridad][id];
}

/**
 * Comprueba el vector quiere del nodo para determinar si hay procesos más prioritarios que el parámetro a la espera
 * @param prioridad prioridad a comprobar
 * @return devuelve 1 si la prioridad dada es más prioritaria que las que esperan en el nodo, y 0 de lo contrario
 */
int prioridad_superior(int prioridad)
{
    for (int i = 0; i < prioridad; i++)
    {
        if (quiere[prioridad] != 0)
            return 0;
    }
    return 1;
}