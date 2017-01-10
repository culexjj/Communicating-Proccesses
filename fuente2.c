


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
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
    struct sembuf sem_op; //estructura para almacenar operacion del semaforo

    sem_op.sem_num = 0;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    semop(sem_set_id, &sem_op, 1);
}



int main()
{
    /** declaracion de variables **/
    char bufferLectura[100]; //Buffer de lectura
    int P2; //Pid de P2
    int P3; //Resultado creacion P3
    int fd; //descriptor fichero para tuberia FIFO
    char* tuberiaFIFO = "fichero1"; //Nombre fichero FIFO
    int memoriaID; //Identificador del area de memoria compartida
    char *vc1; //Identificador para añadir el segmento de memoria compartida
    int sem1;//ID semaforo
    union semun
    {
        int val;
        struct semid_ds *buf;
        ushort * array;
    } sem_val;//Union usada para inicializar el semaforo
    key_t claveShaMen; //Clave del area de memoria compartida
    key_t claveSemaforo; //Clave del semaforo
    int controlSemaforo; //Control inicializacion semaforo
    int llamadaExec; //Resultado llamada al sistema EXEC





	/*************************************************/
	/**            AREA DE CODIGO                   **/
	/*************************************************/


    P2 = getpid(); //pid P2

    /** leer de la tuberia **/
    fd = open(tuberiaFIFO, O_RDONLY); //apertura tuberia
    read(fd,bufferLectura, sizeof(bufferLectura));//lectura
    printf("\nEl proceso P2 (PID=%d, Ej2) recibe el mensaje del proceso P2 por una tuberia FIFO - mensaje: %s", P2, bufferLectura);
    fflush(stdout);
    close(fd);//cierre tuberia


    /** preparacion del area de memoria compartida **/
    claveShaMen = ftok("fichero1",'B');
    if (claveShaMen == -1)
    {
        printf("Error en la creacion de la  clave del area de memoria compartida");
        exit(-1);
    }

    memoriaID = shmget(claveShaMen, 100, IPC_CREAT | 0600); //obtenemos identificador segmento memoria
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

    sem_val.val = 1; //inicializamos el semaforo en verde
    controlSemaforo = semctl (sem1, 0, SETVAL, sem_val);
    if (controlSemaforo == -1)
    {
        printf("Error en la inicializacion del semaforo");
        exit(-1);
    }

    /** Proceso hijo P3 **/
    P3 = fork();
    if (P3 == -1)
    {
        printf("Error en la creacion de P3");
        exit(-1);
    }

     /** operaciones P2 y P3 **/

    if(P3 == 0) //esta parte solo la puede ejecutar el hijo
    {
        llamadaExec = execl("./Ej3", "Ej3", (char*) NULL); //ejecutar Ej3
        if (llamadaExec == -1)
        {
            printf("Error en la llamada EXEC");
            exit(-1);
        }
    }
    else //esta parte solo la puede ejecutar el padre
    {
        /** Operaciones area critica **/
        P(sem1);//Peticion acceso al area critica
        sleep (1);
        strcpy(vc1, bufferLectura);//copia datos al segmento de memoria compartida
        V(sem1);//Liberacion acceso al area critica

        printf("\nEl proceso P2 (PID=%d, Ej2) escribe el mensaje en area memoria compartida - mensaje: %s", P2, vc1);
        fflush(stdout);

        pause();
    }
}
