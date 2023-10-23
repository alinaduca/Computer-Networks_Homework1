#define FIFO1_NAME "server_to_client"
#define FIFO_NAME "client_to_server"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <utmp.h>
bool logat = false;

char *int_to_char(int n)
{
    char *aux;
    aux = malloc(20);
    int nr = 0;
    if(n == 0)
        return "0";
    while(n)
    {
        aux[nr++] = (char)(n % 10 + '0');
        n /= 10;
    }
    for(int i = 0; i < nr / 2; i++)
    {
        char a = aux[i];
        aux[i] = aux[nr - i - 1];
        aux[nr - i - 1] = a;
    }
    aux[nr] = '\0';
    return aux;
}

void Login(char *s, int fd)
{
    int nrc;
    s[0] = ';';
    strcat(s, ";");
    FILE *users = fopen("users.txt", "r");
    char sir[500];
    fscanf(users, "%s", sir);
    if(strstr(sir, s) != NULL)
    {
        char msg[] = "Utilizator gasit.\n";
        nrc = strlen(msg);
        if((write(fd, &nrc, sizeof(nrc))) == -1)
        {
            perror("Problema la scriere in SOCKET, linia 53.\n");
            exit(2);
        }
        if((write(fd, msg, nrc)) == -1)
        {
            perror("Problema la scriere in SOCKET, linia 58.\n");
            exit(2);
        }
    }
    else
    {
        char msg[] = "Utilizator negasit.\n";
        nrc = strlen(msg);
        if((write(fd, &nrc, sizeof(nrc))) == -1)
        {
            perror("Problema la scriere in SOCKET, linia 68.\n");
            exit(2);
        }
        if((write(fd, msg, nrc)) == -1)
        {
            perror("Problema la scriere in SOCKET, linia 73.\n");
            exit(2);
        }
    }
}

void get_logged_users(int fd)
{
    int len = 0;
    char aux[500];
    if(!logat)
    {
        strcpy(aux, "Utilizatorul nu este logat.\n");
        len = strlen(aux);
        if((write(fd, &len, sizeof(len))) == -1)
        {
            perror("Problema la scriere in PIPE, linia 89.\n");
            exit(2);
        }
        if((write(fd, aux, len)) == -1)
        {
            perror("Problema la scriere in PIPE, linia 94.\n");
            exit(2);
        }
    }
    else
    {
        //username, hostname for remote login, time entry was made
        strcpy(aux, "");
        struct utmp* data;
        data = getutent();
        while(data != NULL)
        {
            strcat(aux, "Username: ");
            strcat(aux, data->ut_user);
            strcat(aux, ", hostname for remote login: ");
            strcat(aux, data->ut_host);
            strcat(aux, ", time entry was made: ");
            strcat(aux, int_to_char(data->ut_tv.tv_sec));
            strcat(aux, " s, ");
            strcat(aux, int_to_char(data->ut_tv.tv_usec));
            strcat(aux, " ms.\n");
            len = len + 71 + strlen(data->ut_user) + strlen(data->ut_host) + strlen(int_to_char(data->ut_tv.tv_sec)) + strlen(int_to_char(data->ut_tv.tv_usec));
            data = getutent();
        }
        if((write(fd, &len, sizeof(len))) == -1)
        {
            perror("Problema la scriere in PIPE, linia 120.\n");
            exit(2);
        }
        if((write(fd, aux, len)) == -1)
        {
            perror("Problema la scriere in PIPE, linia 125.\n");
            exit(2);
        }
    }
}

void get_proc_info(char *pid, int fdd)
{
    char sir[500];
    if(!logat)
    {
        strcpy(sir, "Utilizatorul nu este logat.\n");
        int nr = strlen(sir);
        if((write(fdd, &nr, sizeof(nr))) == -1)
        {
            perror("Problema la scriere in SOCKET, linia 140.\n");
            exit(2);
        }
        if((write(fdd, sir, nr)) == -1)
        {
            perror("Problema la scriere in SOCKET, linia 145.\n");
            exit(2);
        }
    }
    else
    {
        //name, state, ppid, uid, vmsize
        char *pid1, sir1[500];
        int lungimea_finala = 0;
        pid1 = malloc(strlen(pid));
        strcpy(pid1, pid + 1);
        strcpy(pid, pid1);
        char path[50] = "/proc/";
        strcat(path, pid);
        strcat(path, "/status");
        FILE *fd;
        fd = fopen(path, "r");
        int i = 1;
        while(!feof(fd))
        {
            fgets(sir, 500, fd);
            if(i == 1 || i == 3 || i == 7 || i == 9 || i == 18)
            {
                if(i == 1)
                    strcpy(sir1, sir);
                else
                    strcat(sir1, sir);
            }
            i++;
        }
        lungimea_finala = strlen(sir1);
        if((write(fdd, &lungimea_finala, sizeof(lungimea_finala))) == -1)
        {
            perror("Problema la scriere in SOCKET, linia 178.\n");
            exit(2);
        }
        if((write(fdd, sir1, lungimea_finala)) == -1)
        {
            perror("Problema la scriere in SOCKET, linia 183.\n");
            exit(2);
        }
    }
}

void Logout(int fd)
{
    logat = false;
    char msg[] = "Utilizator delogat.\n";
    int nr = strlen(msg);
    if((write(fd, &nr, sizeof(nr))) == -1)
    {
        perror("Problema la scriere in FIFO, linia 196.\n");
        exit(2);
    }
    if((write(fd, msg, strlen(msg))) == -1)
    {
        perror("Problema la scriere in FIFO, linia 201.\n");
        exit(2);
    }
}

void quit(int pid, int fd)
{
    kill(pid, SIGINT);
    // raise(SIGINT);
}

void comanda_necunoscuta(int fd)
{
    char msg[] = "Nu s-a gasit nicio comanda recunoscuta.\n";
    int nrc = strlen(msg);
    if((write(fd, &nrc, sizeof(nrc))) == -1)
    {
        perror("Problema la scriere in FIFO, linia 218.\n");
        exit(2);
    }
    if((write(fd, msg, nrc)) == -1)
    {
        perror("Problema la scriere in FIFO, linia 223.\n");
        exit(2);
    }
}

void server()
{
    mknod(FIFO_NAME, S_IFIFO | 0666, 0);
    while(1)
    {
        char s[1024], comanda[50];
        int fd, pid, lungime_s;
        fd = open(FIFO_NAME, O_RDONLY);
        struct stat stat_record;
        stat(FIFO_NAME, &stat_record);
        while(stat_record.st_size == 0)
        {
            stat(FIFO_NAME, &stat_record);
            sleep(1);
        }
        if(read(fd, &pid, sizeof(pid)) == -1)
        {
            perror("Eroare la read, linia 245.\n");
            exit(1);
        }
        if(read(fd, &lungime_s, sizeof(lungime_s)) == -1)
        {
            perror("Eroare la read, linia 250.\n");
            exit(1);
        }
        if(read(fd, s, lungime_s) == -1)
        {
            perror("Eroare la read, linia 255.\n");
            exit(1);
        }
        s[lungime_s] = '\0';
        close(fd);
        fd = open(FIFO_NAME, O_WRONLY | O_TRUNC);
        int fd1;
        fd1 = open(FIFO1_NAME, O_CREAT | O_TRUNC | O_WRONLY);
        strcpy(comanda, "");
        if(s[3] == 'i')
            strncpy(comanda, s, strlen(s) - strlen(strchr(s, ' ')));
        else if(s[4] == 'p')
        {
            int cate_caractere_iau = strlen(s) - strlen(strchr(s, ' '));
            unsigned int i;
            for(i = 0; i < cate_caractere_iau; i++)
                comanda[i] = s[i];
            comanda[cate_caractere_iau] = '\0';
        }
        else
            strcpy(comanda, s);
        int len = strlen(comanda);
        if(strstr(comanda, "login") && comanda[5] != '!')
            comanda[5] = '\0';
        if(!strcmp(comanda, "login") || !strcmp(comanda, "login:"))
        {
            logat = true;
            int sockp[2], pidd;
            if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
            { 
                perror("Eroare la socketpair, linia 285.\n"); 
                exit(3);
            }
            if((pidd = fork()) == -1)
            {
                perror("Eroare la fork, linia 290.\n");
                exit(4);
            }
            else
                if(pidd) //parinte
                {
                    close(sockp[0]);
                    int nrc;
                    char msg[500];
                    strcpy(msg, strrchr(s, ' '));
                    nrc = strlen(msg);
                    if(write(sockp[1], &nrc, sizeof(nrc)) < 0)
                    {
                        perror("Eroare la scrierea in socket, linia 303.\n");
                        exit(2);
                    }
                    if(write(sockp[1], msg, nrc) < 0)
                    {
                        perror("Eroare la scrierea in socket, linia 308.\n");
                        exit(2);
                    } 
                    wait(NULL);
                    if(read(sockp[1], &nrc, sizeof(nrc)) < 0)
                    {
                        perror("Eroare la citirea din socket, linia 314.\n");
                        exit(1);
                    }
                    if(read(sockp[1], msg, nrc) < 0)
                    {
                        perror("Eroare la citirea din socket, linia 319.\n");
                        exit(1);
                    }
                    msg[nrc] = '\0';
                    if(write(fd1, &nrc, sizeof(nrc)) < 0)
                    {
                        perror("Eroare la scrierea in fifo, linia 325.\n");
                        exit(2);
                    }
                    if(write(fd1, msg, nrc) < 0)
                    {
                        perror("Eroare la scrierea in fifo, linia 330.\n");
                        exit(2);
                    }
                    close(sockp[1]);
                }
                else //copil
                {
                    close(sockp[1]);
                    char msg[500];
                    int nrc;
                    if(read(sockp[0], &nrc, sizeof(nrc)) < 0)
                    {
                        perror("Eroare la citirea din socket, linia 342.\n");
                        exit(1);
                    }
                    if(read(sockp[0], msg, nrc) < 0)
                    {
                        perror("Eroare la citirea din socket, linia 347.\n");
                        exit(1);
                    }
                    msg[nrc] = '\0';
                    Login(msg, sockp[0]);
                    close(sockp[0]);
                    exit(7);
                }
        }
        else
            if(!strcmp(comanda, "get-logged-users"))
            {
                int pp[2], pidd;
                if(pipe(pp) == -1)
                {
                    perror("Eroare la pipe, linia 362.\n");
                    exit(5);
                }
                char buff[500];
                if((pidd = fork()) == -1)
                {
                    perror("Eroare la fork, linia 368.\n");
                    exit(4);
                }
                else
                    if(pidd) //parinte
                    {
                        close(pp[1]);
                        char msg[500];
                        wait(NULL);
                        int len;
                        if(read(pp[0], &len, sizeof(len)) < 0)
                        {
                            perror("Eroare la citirea din pipe, linia 380.\n");
                            exit(1);
                        }
                        if(read(pp[0], msg, len) < 0)
                        {
                            perror("Eroare la citirea din pipe, linia 385.\n");
                            exit(1);
                        }
                        if(write(fd1, &len, sizeof(len)) < 0)
                        {
                            perror("Eroare la scrierea in fifo, linia 390.\n");
                            exit(2);
                        } 
                        if(write(fd1, msg, len) < 0)
                        {
                            perror("Eroare la scrierea in fifo, linia 395.\n");
                            exit(2);
                        }
                        close(pp[0]);
                    }
                    else //copil
                    {
                        close(pp[0]);
                        get_logged_users(pp[1]);
                        close(pp[1]);
                        exit(9);
                    }
            }
            else
                if(!strcmp(comanda, "get-proc-info") || !strcmp(comanda, "get-proc-info:"))
                {
                    int sockp[2], pidd;
                    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
                    { 
                        perror("Eroare la socketpair, linia 414.\n"); 
                        exit(3);
                    }
                    if((pidd = fork()) == -1)
                    {
                        perror("Eroare la fork, linia 419.\n");
                        exit(4);
                    }
                    else
                        if(pidd) //parinte
                        {
                            close(sockp[0]);
                            char msg[500];
                            strcpy(msg, strrchr(s, ' '));
                            if(write(sockp[1], msg, sizeof(msg)) < 0)
                            {
                                perror("Eroare la scrierea in socket, linia 430.\n");
                                exit(2);
                            }
                            wait(NULL);
                            int lung;
                            if(read(sockp[1], &lung, sizeof(lung)) < 0)
                            {
                                perror("Eroare la citirea din socket, linia 437.\n");
                                exit(1);
                            }
                            if(read(sockp[1], msg, lung) < 0)
                            {
                                perror("Eroare la citirea din socket, linia 442.\n");
                                exit(1);
                            }
                            msg[lung] = '\0';
                            if(write(fd1, &lung, sizeof(lung)) < 0)
                            {
                                perror("Eroare la scrierea in fifo, linia 448.\n");
                                exit(2);
                            } 
                            if(write(fd1, msg, lung) < 0)
                            {
                                perror("Eroare la scrierea in fifo, linia 453.\n");
                                exit(2);
                            }
                            close(sockp[1]);
                        }
                        else //copil
                        {
                            close(sockp[1]);
                            char msg[500];
                            if(read(sockp[0], msg, sizeof(msg)) < 0)
                            {
                                perror("Eroare la citirea din socket, linia 464.\n");
                                exit(1);
                            }
                            get_proc_info(msg, sockp[0]);
                            close(sockp[0]);
                            exit(11);
                        }
                }
                else
                    if(!strcmp(comanda, "logout"))
                        Logout(fd1);
                    else
                        if(!strcmp(comanda, "quit"))
                            quit(pid, fd1);
                        else
                            comanda_necunoscuta(fd1);        
    }
}

int main()
{
    server();
    return 0;
}




/* 
Are mai multe procese-copil.
Trimite clientului raspunsuri sub forma de siruri de octeti prefixate de lungimea raspunsului.
    Procesele copil nu comunica direct cu clientul, ci doar cu procesul parinte.
    Protocolul minimal cuprinde comenzile: 
      - "login : username" - a carei existenta este validata prin utilizarea unui fisier de configurare, care contine toti utilizatorii care au acces la functionalitati. Executia comenzii va fi realizata intr-un proces copil din server;
      - "get-logged-users" - afiseaza informatii (username, hostname for remote login, time entry was made) despre utilizatorii autentificati pe sistemul de operare (vezi "man 5 utmp" si "man 3 getutent"). Aceasta comanda nu va putea fi executata daca utilizatorul nu este autentificat in aplicatie. Executia comenzii va fi realizata intr-un proces copil din server;
      - "get-proc-info : pid" - afiseaza informatii (name, state, ppid, uid, vmsize) despre procesul indicat (sursa informatii: fisierul /proc/<pid>/status). Aceasta comanda nu va putea fi executata daca utilizatorul nu este autentificat in aplicatie. Executia comenzii va fi realizata intr-un proces copil din server;
      - "logout";
      - "quit".
*/