#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <utmp.h>
#define UT_NAMESIZE 32
#define UT_HOSTSIZE 256
#define BOOT_TIME 2
int fr, fw;
void Fork(char d[256], int **logged)
{
    FILE *users = fopen("users.txt", "r");
    int s[2], p[2], nbytes, ast; // p e pipe, s e socket
    pipe(p);
    socketpair(AF_UNIX, SOCK_STREAM, 0, s);
    char string2[100], ceva[100];
    int num, num2, k;
    if (d[0] == 'l' && d[1] == 'o' && d[2] == 'g' && d[3] == 'i' && d[4] == 'n' && d[5] == ' ' && d[6] == ':' && d[7] == ' ')
    {
        k = fork();
        if (k == 0) // child
        {
            if (*logged == 1)
            {
                strcpy(ceva, "18 Sunteti deja logat\n");
            }
            else
            {
                char *c = 0, *tok, t[256], ceva[256];
                tok = strtok(d, " ");
                tok = strtok(NULL, " ");
                tok = strtok(NULL, " ");
                strcpy(d, tok);
                int ok = 0;
                while (fgets(t, 256, users) != NULL)
                {
                    int cnt = 0, len;
                    for (int i = 0; i < strlen(t); i++)
                    {
                        if (d[i] == t[i])
                            cnt++;
                    }
                    if (cnt == strlen(d))
                    {
                        strcpy(ceva, "11 User corect");
                        *logged = 1;
                        ok = 1;
                        break;
                    }
                }
                fclose(users);
                if (ok == 0)
                    strcpy(ceva, "28 User incorect, reincercati");
            }
            close(p[0]);
            write(p[1], ceva, (strlen(ceva + 1) + 1)); // rezultatul din child in ceva (string)
            exit(1);
        }
        if (k > 0) // parent
        {
            wait(&ast);
            close(p[1]);
            nbytes = read(p[0], &string2, sizeof(string2));
            close(p[0]);
            if ((num2 = write(fr, string2, strlen(string2 + 1) + 1)) == -1)
                perror("problema la scriere in fifo2");
        }
    }
    else
    {
        if (d[0] == 'g' && d[1] == 'e' && d[2] == 't' && d[3] == '-' && d[4] == 'l' && d[5] == 'o' && d[6] == 'g' && d[7] == 'g' && d[8] == 'e' && d[9] == 'd' && d[10] == '-' && d[11] == 'u' && d[12] == 's' && d[13] == 'e' && d[14] == 'r' && d[15] == 's')
        {
            k = fork();
            if (k == 0) // child
            {
                if (*logged == 0)
                {
                    close(s[0]);
                    write(s[1], "30 Trebuie sa va logati mai intai", 33 + 1);
                }
                else
                {
                    struct utmp *data;
                    char info[256];
                    char *timp;
                    data = getutent();
                    while (data != NULL)
                    {
                        /*strcat(info, data->ut_user);
                        strcat(info, "\n");
                        strcat(info, data->ut_host);
                        strcat(info, "\n");
                        itoa(data->ut_time, timp, 10);
                        strcat(info, timp);
                        strcat(info, "\n");
                        write(s[1], info, strlen(info) + 1);*/
                        printf("ut_user: %s\n", data->ut_user);
                        printf("ut_host: %s\n", data->ut_host);
                        printf("ut_time: %d\n\n", data->ut_time);
                        data = getutent();
                    }
                    close(s[0]);
                    write(s[1], "15 merge comanda 2", 18 + 1);
                }
                exit(1);
            }
            if (k > 0) // parent
            {
                wait(&ast);
                close(s[1]);
                nbytes = read(s[0], &string2, sizeof(string2));
                close(s[0]);
                if ((num2 = write(fr, string2, strlen(string2 + 1) + 1)) == -1)
                    perror("problema la scriere in fifo2");
            }
        }
        else
        {
            if (d[0] == 'g' && d[1] == 'e' && d[2] == 't' && d[3] == '-' && d[4] == 'p' && d[5] == 'r' && d[6] == 'o' && d[7] == 'c' && d[8] == '-' && d[9] == 'i' && d[10] == 'n' && d[11] == 'f' && d[12] == 'o' && d[13] == ' ' && d[14] == ':' && d[15] == ' ')
            {
                k = fork();
                if (k == 0) // child
                {
                    if (*logged == 0)
                    {
                        close(s[0]);
                        write(s[1], "30 Trebuie sa va logati mai intai", 33 + 1);
                    }
                    else
                    {
                        close(s[0]);
                        write(s[1], "15 merge comanda 3", 18 + 1);
                    }

                    exit(1);
                }

                if (k > 0) // parent
                {
                    wait(&ast);
                    close(s[1]);
                    nbytes = read(s[0], &string2, sizeof(string2));
                    close(s[0]);
                    if ((num2 = write(fr, string2, strlen(string2 + 1) + 1)) == -1)
                        perror("problema la scriere in fifo2");
                }
            }
            else
            {
                if (d[0] == 'l' && d[1] == 'o' && d[2] == 'g' && d[3] == 'o' && d[4] == 'u' && d[5] == 't')
                {
                    k = fork();
                    if (k == 0) // child
                    {
                        if (*logged == 0)
                        {
                            close(s[0]);
                            write(s[1], "30 Trebuie sa va logati mai intai", 33 + 1);
                        }
                        else
                        {
                            close(s[0]);
                            write(s[1], "13 V-ati delogat", 16 + 1);
                            *logged = 0;
                        }

                        exit(1);
                    }

                    if (k > 0) // parent
                    {
                        wait(&ast);
                        close(s[1]);
                        nbytes = read(s[0], &string2, sizeof(string2));
                        close(s[0]);
                        if ((num2 = write(fr, string2, strlen(string2 + 1) + 1)) == -1)
                            perror("problema la scriere in fifo2");
                    }
                }
                else
                {
                    if (d[0] == 'q' && d[1] == 'u' && d[2] == 'i' && d[3] == 't')
                    {
                        k = fork();
                        if (k == 0) // child
                        {

                            close(s[0]);
                            write(s[1], "5 break", 7 + 1);

                            exit(1);
                        }

                        if (k > 0) // parent
                        {
                            wait(&ast);
                            close(s[1]);
                            nbytes = read(s[0], &string2, sizeof(string2));
                            close(s[0]);
                            if ((num2 = write(fr, string2, strlen(string2 + 1) + 1)) == -1)
                                perror("problema la scriere in fifo2");
                        }
                    }
                    else
                    {
                        k = fork();
                        if (k == 0) // child
                        {

                            close(s[0]);
                            write(s[1], "19 Comanda inexistenta", 22 + 1);

                            exit(1);
                        }

                        if (k > 0) // parent
                        {
                            wait(&ast);
                            close(s[1]);
                            nbytes = read(s[0], &string2, sizeof(string2));
                            close(s[0]);
                            if ((num2 = write(fr, string2, strlen(string2 + 1) + 1)) == -1)
                                perror("problema la scriere in fifo2");
                        }
                    }
                }
            }
        }
    }
}
int main(int argc, char *argv[])
{
    char d[256];
    int num;
    int *logged = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    mknod("fr", S_IFIFO | 0666, 0);
    mknod("fw", S_IFIFO | 0666, 0);

    printf("Autentificare...\n");
    fw = open("fw", O_RDONLY);
    fr = open("fr", O_WRONLY);
    if (fw == -1 || fr == -1)
    {
        perror("Unable to open fifos\n");
        exit(EXIT_FAILURE);
    }
    do
    {
        if ((num = read(fw, &d, 255)) == -1)
            printf("Problema la citire in fifo1");
        else
        {
            
            if(num==0)
            {
                close(fw);
                fw = open("fw", O_RDONLY);
                //strcpy(d,NULL);
            }
            else
            {
                d[num] = '\0';
            printf("S-au citit din FIFO %d bytes: \"%s\"\n", num, d);
            Fork(d, &(*logged));
            }
            
        }
    } while (num > 0);
}
/* surse:
comunicarea fifo, pipes si sockets: exemplele de pe site-ul cursului
folosirea functiei mmap: https://www.youtube.com/watch?v=rPV6b8BUwxM
functia wait cu parametru dat prin referinta: geeksforgeeks
folosirea structurii utmp: stackoverflow
*/