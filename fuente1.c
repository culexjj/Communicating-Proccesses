


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <sys/times.h>

int main()
{
    /** declaracion de variables **/
    char mensaje[100]; //Mensaje
    char bufferLectura[100]; //Buffer de lectura tuberia
    int tuberia[2]; //Tuberia sin nombre
    int P1; //PID de P1
    int P2; //PID de P2 padre
    int pidP2; //Pid de P2 hijo
    //int P3; //PID de P3 padre
    int resultadoTuberia; //Resultado creacion tuberia
    int resultadoFIFO; //Resultado creacion tuberia FIFO
    int resultadoEscrituraFIFO; //Resultado escritura tuberia FIFO
    int resultadoLecturaCola;//Resultado lectura de la cola de mensajes
    int llamadaExec; //Resultado llamada al sistema EXEC
    int colaID; //Identificador de la cola de mensajes
    int fd; //descriptor fichero para tuberia FIFO
    key_t claveColaMensajes; //Clave de la cola de mensajes
    char* tuberiaFIFO = "fichero1"; //Nombre fichero FIFO
    struct //Estructura usada por la cola de mensajes
    {
        long tipo;
        int valor; //pid
    } mensaje1;
   	struct //Estructura usada para el calculo de tiempo de uso
	{
		clock_t tms_utime;
		clock_t tms_stime;
		clock_t tms_cutime;
		clock_t tms_cstime;
	} pb1, pb2;
	clock_t t1,t2;//tiempo inicio y fin
    int CLK_TCK;//tics de reloj
    float ti1;//Tiempo ejecucion
    float ti2;//Tiempor modo usuario
    float cpu;//% uso CPU



	/*************************************************/
	/**            AREA DE CODIGO                   **/
	/*************************************************/

	t1=times(&pb1);//tiempo inicio ejecucion

    P1 = getpid(); //pid P1

    /** lectura mensaje **/
    printf("Escriba el mensaje : ");
	fflush(stdout);
    fgets(mensaje, 100, stdin);
    printf("\nEl mensaje es: %s", mensaje);
    fflush(stdout);

    /** Tuberia sin nombre **/
    resultadoTuberia = pipe(tuberia);
    if (resultadoTuberia == -1)
    {
        printf("Error en la creacion de la tuberia");
        exit(-1);
    }

    /** Proceso hijo P2 **/
    P2 = fork();
    if (P2 == -1)
    {
        printf("Error en la creacion de P2");
        exit(-1);
    }

    /** operaciones P1 y P2 **/
    if(P2 == 0) //esta parte solo la puede ejecutar el hijo
    {
        pidP2 = getpid(); //pid P2

        /** preparacion de la tuberia sin nombre y lectura **/
        close(tuberia[1]); //Hijo cierra la entrada de la tuberia
        read(tuberia[0], bufferLectura, sizeof(bufferLectura)); //hijo lee el mensaje
        printf("\nEl proceso P2 (PID=%d, Ej1) recibe el mensaje del proceso P1 por una tuberia sin nombre - mensaje: %s", pidP2, bufferLectura);
        fflush(stdout);

        /** preparacion de la tuberia con nombre **/
        unlink(tuberiaFIFO); //eliminamos el link a fichero1 en caso de que una ejecucion anterior no acabe correctamente
        resultadoFIFO = mknod(tuberiaFIFO, S_IFIFO|0666, 0);//creacion tuberia FIFO
        if (resultadoFIFO == -1)
        {
            printf("Error en la creacion de la tuberia FIFO");
            exit(-1);
        }

        /** escribir mensaje en tuberia FIFO **/
        fd = open("fichero1",O_RDWR);
        if (fd == -1)
        {
            printf("Error en la apertura de la tuberia FIFO");
            exit(-1);
        }

        resultadoEscrituraFIFO = write(fd,bufferLectura,(strlen(bufferLectura)+1));
        if (resultadoEscrituraFIFO == -1)
        {
            printf("Error en la escritura de la tuberia FIFO");
            exit(-1);
        }

         printf("\nEl proceso P2 (PID=%d, Ej1) escribe el mensaje del proceso P1 en la tuberia FIFO - mensaje: %s", pidP2, bufferLectura);
         fflush(stdout);

        /** ejecutar Ej2 **/
		llamadaExec = execl("./Ej2", "Ej2", (char*) NULL);
        if (llamadaExec == -1)
        {
            printf("Error en la llamada EXEC");
            exit(-1);
        }
    }
    else //esta parte solo la puede ejecutar el padre
    {
        /** preparacion de la tuberia sin nombre y escritura **/
        close(tuberia[0]); //Padre cierra la salida de la tuberia
        write(tuberia[1], mensaje, (strlen(mensaje)+1)); //padre escribe el mensaje
        printf("\nEl proceso P1 (PID=%d, Ej1) transmite un mensaje al proceso P2 por una tuberia sin nombre\n", P1);
        fflush(stdout);

        /** preparacion de la cola de mensajes **/
        claveColaMensajes = ftok("Ej1",'A');
        colaID = msgget(claveColaMensajes, IPC_CREAT | 0600);
        if (colaID == -1)
        {
            printf("Error en la creacion de la cola de mensajes");
            exit(-1);
        }

        resultadoLecturaCola = msgrcv(colaID, (struct msgbuf *)&mensaje1, sizeof(mensaje1.valor), 1, 0);//lectura cola mensajes
        if (resultadoLecturaCola == -1)
        {
            printf("Error en el recepcion de la cola de mensajes");
            exit(-1);
        }

        printf("\nEl proceso P1 (PID=%d, Ej1) recibe el PID del proceso P3 por la cola de mensajes - PID P3: %d\n", P1, mensaje1.valor);
        fflush(stdout);

        msgctl (colaID, IPC_RMID, (struct msqid_ds *)NULL); //borramos la cola de mensajes

        kill(P2, SIGKILL); //kill P2
        kill(mensaje1.valor, SIGKILL); //kill P3

        unlink(tuberiaFIFO); //eliminamos el link a fichero1


        // Bucle para pruebas informacion estadistica
        /*int h;
		for(h==1;h<=1000000000;h++)
        {
            h++;
        };*/


        t2=times(&pb2); //tiempo fin ejecucion

		CLK_TCK=sysconf(_SC_CLK_TCK); //tics de reloj

        /** calculo tiempo ejecucion y tiempo uso CPU **/
        ti1 =(float) (t2-t1)/ CLK_TCK;
        ti2 =(float) (pb2.tms_utime-pb1.tms_utime)/ CLK_TCK;
        cpu = ((ti2 * 100) / ti1) ;

        printf("\n");
        printf("\nUso de CPU = %.2f %%", cpu);
        printf("\nTiempo de ejecucion = %.2f segundos", ti1);
        printf("\nTiempo de uso de la CPU en modo usuario = %.2f segundos\n", ti2);
        printf("\n");

        return 0;
    }
}
