#define FIFO1_NAME "server_to_client"
#define FIFO_NAME "client_to_server"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int main()
{
    mknod(FIFO1_NAME, S_IFIFO | 0666, 0);
    while(1)
    {
        int fd;
        char s[1024];
        fd = open(FIFO_NAME, O_CREAT | O_TRUNC | O_WRONLY);
        int pid = getpid(), lungime_s;
        printf("Introduceti comanda: ");
        gets(s);
        lungime_s = strlen(s);
        if(write(fd, &pid, sizeof(pid)) == -1)
        {
            perror("Problema la scriere in FIFO, linia 26.\n");
            exit(2);
        }
        if((write(fd, &lungime_s, sizeof(lungime_s))) == -1)
        {
            perror("Problema la scriere in FIFO, linia 31.\n");
            exit(2);
        }
        if((write(fd, s, strlen(s))) == -1)
        {
            perror("Problema la scriere in FIFO, linia 36.\n");
            exit(2);
        }
        close(fd);
        int fd1 = open(FIFO1_NAME, O_RDONLY);
        struct stat stat_record;
        stat(FIFO1_NAME, &stat_record);
        while(stat_record.st_size == 0) //Prin asta il fortez sa nu citeasca date dintr-un fisier gol.
        {
            stat(FIFO1_NAME, &stat_record);
            sleep(1);
        }
        char aux[1024] = "";
        int nrcaux;
        if(read(fd1, &nrcaux, sizeof(nrcaux)) < 0)
        {
            perror("Eroare la read, linia 52.\n");
            exit(1);
        }
        if(read(fd1, aux, nrcaux) < 0)
        {
            perror("Eroare la read, linia 57.\n");
            exit(1);
        }
        if(write(1, aux, nrcaux) < 0)
        {
            perror("Eroare la write, linia 62.\n");
            exit(2);
        }
        close(fd1);
        fd1 = open(FIFO1_NAME, O_WRONLY | O_TRUNC); //Dupa ce citesc, vreau sa sterg continutul fifo-ului.
        close(fd1);
    }
    exit(0);
    return 0;
}

/* 
    Aici se citesc comenzile (siruri de caractere delimitate prin new line).
    Va afisa rezultatul obtinut in urma executiei oricarei comenzi.
*/