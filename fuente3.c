


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>


/*
 * funcion: P. Bloquea el semaforo y así obtener acceso exclusivo al recurso.
 * input:    semaforo  ID.
 */

void P(int sem_set_id)
{
    struct sembuf sem_op; //estructura para almacenar operacion del semaforo

    sem_op.sem_num = 0;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    semop(sem_set_id, &sem_op, 1);
}


/*
 * funcion: V. desbloquea el semaforo.
 * input:    semaforo  ID.
 */

void V(int sem_set_id)
{
    struct sembuf sem_op;//estructura para almacenar operacion del semaforo

    sem_op.sem_num = 0;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    semop(sem_set_id, &sem_op, 1);
}



int main()
{
    /** declaracion de variables **/

    int colaID; //Identificador de la cola de mensajes
    int P3; //Pid de P3
    int sem1; //ID semaforo
    union semun
    {
        int val;
        struct semid_ds *buf;
        ushort * array;
    } sem_val; //Union usada para inicializar el semaforo
    key_t claveShaMen; //Clave del area de memoria compartida
    key_t claveSemaforo; //Clave del semaforo
    int memoriaID; //Identificador del area de memoria compartida
	char *vc1; //Identificador para añadir el segmento de memoria compartida
    key_t claveColaMensajes; //Clave de la cola de mensajes
    struct //Estructura usada por la cola de mensajes
    {
        long tipo;
        int valor; // pid
    } mensaje1;

    int resultadoEscrituraCola; //Resultado escritura cola


    /*************************************************/
	/**            AREA DE CODIGO                   **/
	/*************************************************/


    P3 = getpid(); //pid P3

    /** preparacion del area de memoria compartida **/
    claveShaMen = ftok("fichero1",'B');
    if (claveShaMen == -1)
    {
       	printf("Error en la creacion de la  clave del area de memoria compartida");
       	exit(-1);
    }

    memoriaID = shmget(claveShaMen, 100, IPC_CREAT | 0600);//obtenemos identificador segmento memoria
    if (memoriaID == -1)
    {
       	printf("Error en la creacion del area de memoria compartida");
       	exit(-1);
    }

    vc1 = shmat(memoriaID,0,0); //añadimos el area de memoria
    if (vc1 == -1)
    {
        printf("Error en la vinculacion del area de memoria compartida");
        exit(-1);
    }


    /** preparacion del semaforo **/
    claveSemaforo = ftok("fichero1",'C');
    if (claveSemaforo == -1)
    {
        printf("Error en la creacion de la  clave del semaforo");
        exit(-1);
    }

    sem1 = semget(claveSemaforo, 1, IPC_CREAT | 0600); //obtenemos identificador del semaforo
    if (sem1 == -1)
    {
        printf("Error en la creacion del semaforo");
        exit(-1);
    }


    /** Operaciones area critica **/
    P(sem1);//Peticion acceso al area critica
    vc1 = shmat(memoriaID,0,0); //añadimos el area de memoria
    V(sem1);//Liberacion acceso al area critica

    printf("\nEl proceso P3 (PID=%d, Ej3) lee el mensaje del area memoria compartida - mensaje: %s", P3, vc1);
    fflush(stdout);


    /** preparacion de la cola de mensajes **/
    claveColaMensajes = ftok("Ej1",'A');
    colaID = msgget(claveColaMensajes, IPC_CREAT | 0600);
    if (colaID == -1)
    {
        printf("Error en la creacion de la cola de mensajes");
        exit(-1);
    }

    //Mensaje a enviar
    mensaje1.tipo=1;
    mensaje1.valor = getpid();

    resultadoEscrituraCola = msgsnd(colaID,(struct msgbuf *)&mensaje1,sizeof(mensaje1.valor),0); //escritura cola mensajes
    if (resultadoEscrituraCola == -1)
    {
        printf("Error en el envio de la cola de mensajes");
        exit(-1);
    }

    printf("\nEl proceso P3 (PID=%d, Ej3) envia su PID al proceso P1 por la cola de mensajes - PID P3: %d\n", P3, mensaje1.valor);
    fflush(stdout);

    pause();
}
