#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sys/mman.h>
#include <utmp.h>
#include <sqlite3.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
/* portul folosit */
#define PORT 2906

/* codul de eroare returnat de anumite apeluri */
extern int errno;
int sd;
struct thData tdL;
typedef struct thData
{
    int idThread; // id-ul thread-ului tinut in evidenta de acest program
    int cl;       // descriptorul intors de accept
} thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
char t[800];
void prefix(char d[800])
{
    int i;
    bzero(t, sizeof(t));
    for (i = 0; i < strlen(d); i++)
    {
        if (d[i] < 32 || d[i] > 126)
            break;
        else
            t[i] = d[i];
    }
    strcpy(d, t);
}
char x[10];
void itoa(int i, char x[10])
{
    int count = 1, o = 0, aux = i, k = 0;
    char c;
    while (aux != 0)
    {
        aux = aux / 10;
        count = count * 10;
    }
    count = count / 10;
    while (count != 0)
    {
        o = o + i % 10 * count;
        count = count / 10;
        i = i / 10;
    }
    while (o != 0)
    {
        c = o % 10 + 48;
        o = o / 10;
        x[k++] = c;
    }
}
int main()
{
    struct sockaddr_in server; // structura folosita de server
    struct sockaddr_in from;
    int nr; // mesajul primit de trimis la client
    // int sd;		//descriptorul de socket
    int pid;
    pthread_t th[100]; // Identificatorii thread-urilor care se vor crea
    int i = 0;

    /* crearea unui socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    /* utilizarea optiunii SO_REUSEADDR */
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /* pregatirea structurilor de date */
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons(PORT);

    /* atasam socketul */
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 2) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }
    /* servim in mod concurent clientii...folosind thread-uri */
    while (1)
    {
        int client;
        thData *td; // parametru functia executata de thread
        int length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        // client= malloc(sizeof(int));
        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        /* s-a realizat conexiunea, se astepta mesajul */

        // int idThread; //id-ul threadului
        // int cl; //descriptorul intors de accept

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;

        pthread_create(&th[i], NULL, &treat, td);

    } // while
};
static void *treat(void *arg)
{
    int id;
    struct thData tdL;
    tdL = *((struct thData *)arg);
    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush(stdout);
    pthread_detach(pthread_self());
    // verif(&id);
    raspunde((struct thData *)arg);
    // printf("%d",id);

    /* am terminat cu acest client, inchidem conexiunea */
    close((intptr_t)arg);
    // close(sd);
    return (NULL);
};

// login
void logare(char nr[800], int *logged, char ceva[800], int *id, void *arg)
{
    char *tok;
    struct thData tdL;
    tdL = *((struct thData *)arg);
    if (*logged == 1)
    {
        strcpy(ceva, "Sunteti deja logat\n");
    }
    else
    {

        sqlite3_stmt *res, *res1, *res2;
        sqlite3 *datab;
        int result2;
        int handle = sqlite3_open("db.db", &datab);
        int result;

        char *sql8 = sqlite3_mprintf("update verificare set comanda='%q' where id=1", nr);
        result = sqlite3_prepare_v2(datab, sql8, -1, &res1, 0);
        sqlite3_step(res1);
        sqlite3_finalize(res1);

        tok = strtok(nr, " ");
        tok = strtok(NULL, " ");
        strcpy(nr, tok);

        char *sql2 = sqlite3_mprintf("Select * from useri where username = '%q'", nr);
        result = sqlite3_prepare_v2(datab, sql2, -1, &res, 0);
        if (result == SQLITE_OK)
        {
            if (sqlite3_step(res) == SQLITE_ROW)
            {
                strcpy(ceva, "User existent,v-ati logat");
                char *sql = sqlite3_mprintf("update useri set online=1 where username='%q'", nr);
                result = sqlite3_prepare_v2(datab, sql, -1, &res1, 0);
                sqlite3_step(res1);
                sqlite3_finalize(res1);
                *logged = 1;

                char *sql3 = sqlite3_mprintf("update useri set tdlcl='%d' where username='%q'", tdL.cl, nr);
                result = sqlite3_prepare_v2(datab, sql3, -1, &res1, 0);
                sqlite3_step(res1);
                sqlite3_finalize(res1);

                char *sql1 = sqlite3_mprintf("select id from useri where username='%q'", nr);
                result2 = sqlite3_prepare_v2(datab, sql1, -1, &res2, 0);
                sqlite3_step(res2);
                *id = sqlite3_column_int(res2, 0);
                sqlite3_finalize(res2);
                // printf("%d", id);
            }
            else
            {
                strcpy(ceva, "User inexistent. Va autentificati?");
            }
        }
        sqlite3_finalize(res);
        sqlite3_close(datab);
    }
}
// autentificare
void autentificare(char nr[800], int *logged, char ceva[800], int *id)
{
    char *tok;
    if (*logged == 1)
    {
        strcpy(ceva, "Sunteti deja logat\n");
    }
    else
    {

        sqlite3_stmt *res, *res1;
        sqlite3 *datab;
        int handle = sqlite3_open("db.db", &datab);
        int result;

        char *sql8 = sqlite3_mprintf("update verificare set comanda='%q' where id=1", nr);
        result = sqlite3_prepare_v2(datab, sql8, -1, &res1, 0);
        sqlite3_step(res1);
        sqlite3_finalize(res1);

        tok = strtok(nr, " ");
        tok = strtok(NULL, " ");
        strcpy(nr, tok);

        char *sql2 = sqlite3_mprintf("Select * from useri where username = '%q'", nr);
        result = sqlite3_prepare_v2(datab, sql2, -1, &res, 0);
        if (result == SQLITE_OK)
        {
            if (sqlite3_step(res) == SQLITE_ROW)
            {
                strcpy(ceva, "User existent,v-ati autentificat deja. Va logati?");
            }
            else
            {
                strcpy(ceva, "User inexistent,v-ati autentificat");
                char *sql = sqlite3_mprintf("INSERT INTO useri(username,online,modificat) VALUES('%q',0,0);", nr);
                result = sqlite3_prepare_v2(datab, sql, -1, &res1, 0);
                sqlite3_step(res1);
                sqlite3_finalize(res1);
            }
        }
        sqlite3_finalize(res);
        sqlite3_close(datab);
    }
}
// logout
void delogare(char nr[800], int *logged, char ceva[800], int *id, void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    sqlite3_stmt *res, *res1;
    sqlite3 *datab;
    int handle = sqlite3_open("db.db", &datab);
    if (*logged == 0)
    {
        strcpy(ceva, "Nu v-ati logat inca");
    }
    else
    {
        *logged = 0;

        strcpy(ceva, "V-ati delogat cu succes!");
        // printf("%d", id);
        char *sql = sqlite3_mprintf("update useri set online=0 where id='%d'", (int)(*id));
        int result = sqlite3_prepare_v2(datab, sql, -1, &res1, 0);
        sqlite3_step(res1);
        *id = 0;
        sqlite3_finalize(res1);

        char *sql1 = sqlite3_mprintf("update useri set tdlcl=0 where id='%d'", (int)(*id));
        result = sqlite3_prepare_v2(datab, sql1, -1, &res1, 0);
        sqlite3_step(res1);
        sqlite3_finalize(res1);
    }

    sqlite3_close(datab);
}
// lista useri
void lista(char nr[800], int *logged, char ceva[800], int *id)
{
    char str[800];
    int count;
    sqlite3_stmt *res1, *res;
    sqlite3 *datab;
    int handle = sqlite3_open("db.db", &datab);
    if (*logged == 0)
    {
        strcpy(ceva, "Nu v-ati logat inca");
    }
    else
    {

        strcpy(ceva, "Lista useri: ");
        char *sql1 = sqlite3_mprintf("select count(id) from useri");
        int result1 = sqlite3_prepare_v2(datab, sql1, -1, &res, 0);
        sqlite3_step(res);
        count = sqlite3_column_int(res, 0);
        // printf("%d", count);
        bzero(str, sizeof(str));
        sqlite3_finalize(res);
        while (count != 0)
        {
            char *sql = sqlite3_mprintf("select username from useri where id='%d'", count);
            int result = sqlite3_prepare_v2(datab, sql, -1, &res1, 0);
            sqlite3_step(res1);
            strcat(str, sqlite3_column_text(res1, 0));
            if (count > 1)
                strcat(str, ";");
            // printf("%s", str);
            sqlite3_finalize(res1);
            count--;
        }
        // printf("%s",str);
        strcat(ceva, str);
    }

    sqlite3_close(datab);
}
// lista useri online
void lista_on(char nr[800], int *logged, char ceva[800], int *id)
{
    char str[800];
    int count;
    sqlite3_stmt *res1, *res;
    sqlite3 *datab;
    int handle = sqlite3_open("db.db", &datab);
    if (*logged == 0)
    {
        strcpy(ceva, "Nu v-ati logat inca");
    }
    else
    {

        strcpy(ceva, "Lista useri online: ");
        char *sql1 = sqlite3_mprintf("select count(id) from useri");
        int result1 = sqlite3_prepare_v2(datab, sql1, -1, &res, 0);
        sqlite3_step(res);
        count = sqlite3_column_int(res, 0);
        // printf("%d", count);
        sqlite3_finalize(res);
        bzero(str, sizeof(str));
        while (count != 0)
        {
            char *sql = sqlite3_mprintf("select username from useri where id='%d' and online=1", count);
            int result = sqlite3_prepare_v2(datab, sql, -1, &res1, 0);
            if (sqlite3_step(res1) == SQLITE_ROW)
            {
                strcat(str, sqlite3_column_text(res1, 0));
                if (count > 1)
                    strcat(str, ";");
                // printf("%s", str);
            }

            sqlite3_finalize(res1);
            count--;
        }
        // printf("%s",str);
        strcat(ceva, str);
    }
    sqlite3_close(datab);
}
// trimitere mesaj
void trimitere(char nr[800], int *logged, char ceva[800], int *id)
{
    char *tok, msj[800], user[800];
    int poz1 = 0, poz2 = 0, poz3 = 0;
    if (*logged == 0)
    {
        strcpy(ceva, "Nu v-ati logat inca");
    }
    else
    {
        for (int i = 0; i < strlen(nr); i++)
        {
            if (nr[i] == 39) // caut '
            {
                if (poz1 == 0)
                    poz1 = i + 1;
                else
                    poz2 = i - 1;
            }
            if (nr[i] == ':')
                poz3 = i + 1;
        }
        int j = 0, k = 0;
        for (int i = poz1; i <= poz2; i++)
        {
            msj[j++] = nr[i];
        }
        for (int i = poz3; i < strlen(nr); i++)
        {
            user[k++] = nr[i];
        }
        msj[j] = '\0';
        user[k] = '\0';
        // printf("%s",msj);
        // printf("%s",user);
        sqlite3_stmt *res, *res1;
        sqlite3 *datab;
        int handle = sqlite3_open("db.db", &datab);
        char *db_err = 0;
        char *sql2 = sqlite3_mprintf("Select * from useri where username = '%q'", user);
        int result = sqlite3_prepare_v2(datab, sql2, -1, &res, 0);
        if (result == SQLITE_OK)
        {
            if (sqlite3_step(res) == SQLITE_ROW)
            {
                strcpy(ceva, "Mesaj trimis cu succes!");
                int id2 = sqlite3_column_int(res, 0);
                char *sql = sqlite3_mprintf("insert into mesaje(userid1,userid2,message) values('%d','%d','%q');", (int)(*id), id2, msj);
                result = sqlite3_prepare_v2(datab, sql, -1, &res1, 0);
                sqlite3_step(res1);
                sqlite3_finalize(res1);

                char *sql3 = sqlite3_mprintf("update useri set modificat=1 where id='%d'", id2);
                result = sqlite3_prepare_v2(datab, sql3, -1, &res1, 0);
                sqlite3_step(res1);
                sqlite3_finalize(res1);
                char s[800];
                char *sql5 = sqlite3_mprintf("select username from useri where id='%d'", (int)(*id));
                result = sqlite3_prepare_v2(datab, sql5, -1, &res1, 0);
                sqlite3_step(res1);
                strcpy(s, sqlite3_column_text(res1, 0));
                sqlite3_finalize(res1);

                char m[800];
                strcpy(m, "[");
                strcat(m, s);
                strcat(m, "]:");
                strcat(m, msj);
                char *sql4 = sqlite3_mprintf("update useri set mesaj='%q' where id='%d'", m, id2);
                result = sqlite3_prepare_v2(datab, sql4, -1, &res1, 0);
                sqlite3_step(res1);
                sqlite3_finalize(res1);

                char *sql1 = sqlite3_mprintf("select tdlcl from useri where id='%d' and online=1", id2);
                result = sqlite3_prepare_v2(datab, sql1, -1, &res1, 0);
                if (sqlite3_step(res1) == SQLITE_ROW)
                {
                    int td = sqlite3_column_int(res1, 0);
                    // write(td, m, strlen(m + 1) + 1);
                   /* int fifo = open(user, O_WRONLY | O_TRUNC);
                   if( write(fifo, m, strlen(m + 1) + 1)<=0)
                   {
                    perror("eroare in write fifo");
                   }*/
                   // close(fifo);
                    // sleep(2);
                    sqlite3_finalize(res1);
                }
            }
            else
            {
                strcpy(ceva, "User catre care se trimite inexistent");
            }
        }
        sqlite3_finalize(res);
        sqlite3_close(datab);
    }
}
// istoric general
void istoric(char nr[800], int *logged, char ceva[800], int *id)
{
    if (*logged == 0)
    {
        strcpy(ceva, "Nu v-ati logat inca");
    }
    else
    {
        char str[800];
        strcpy(str, "[");
        sqlite3_stmt *res, *res1, *res2;
        sqlite3 *datab;
        int result, id2 = 0, count, i = 0;
        char aux[800];
        int handle = sqlite3_open("db.db", &datab);
        char *db_err = 0;

        char *sql = sqlite3_mprintf("select count(id) from mesaje");
        int result1 = sqlite3_prepare_v2(datab, sql, -1, &res, 0);
        sqlite3_step(res);
        count = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
        int c = count;
        while (count > 0)
        {

            char *sql2 = sqlite3_mprintf("select message,userid2 from mesaje where userid1='%d' and id='%d';", (int)(*id), count);
            result = sqlite3_prepare_v2(datab, sql2, -1, &res2, 0);
            if (result == SQLITE_OK)
            {
                if (sqlite3_step(res2) == SQLITE_ROW)
                {
                    bzero(aux, sizeof(aux));
                    char *sql1 = sqlite3_mprintf("select username from useri where id='%d'", (int)(*id));
                    result = sqlite3_prepare_v2(datab, sql1, -1, &res1, 0);
                    sqlite3_step(res1);
                    strcat(str, sqlite3_column_text(res1, 0));
                    strcat(str, "]->");
                    id2 = sqlite3_column_int(res2, 1);
                    strcpy(aux, sqlite3_column_text(res2, 0));
                    sqlite3_finalize(res1);

                    strcat(str, "[");
                    char *sql3 = sqlite3_mprintf("select username from useri where id='%d';", id2);
                    result = sqlite3_prepare_v2(datab, sql3, -1, &res, 0);
                    sqlite3_step(res);
                    strcat(str, sqlite3_column_text(res, 0));
                    strcat(str, "]:");
                    strcat(str, aux);
                    if (count > 1)
                        strcat(str, ";[");
                    sqlite3_finalize(res);
                }
            }
            sqlite3_finalize(res2);
            count--;
        }

        while (c > 0)
        {
            bzero(aux, sizeof(aux));
            char *sql4 = sqlite3_mprintf("select message,userid1 from mesaje where userid2='%d' and id='%d';", (int)(*id), c);
            result = sqlite3_prepare_v2(datab, sql4, -1, &res1, 0);
            if (result == SQLITE_OK)
            {
                if (sqlite3_step(res1) == SQLITE_ROW)
                {
                    id2 = sqlite3_column_int(res1, 1);
                    strcpy(aux, sqlite3_column_text(res1, 0));

                    strcat(str, ";[");

                    char *sql5 = sqlite3_mprintf("select username from useri where id='%d';", id2);
                    result = sqlite3_prepare_v2(datab, sql5, -1, &res, 0);
                    sqlite3_step(res);
                    strcat(str, sqlite3_column_text(res, 0));
                    strcat(str, "]->[");
                    sqlite3_finalize(res);

                    char *sql6 = sqlite3_mprintf("select username from useri where id='%d';", (int)(*id));
                    result = sqlite3_prepare_v2(datab, sql6, -1, &res2, 0);
                    sqlite3_step(res2);
                    strcat(str, sqlite3_column_text(res2, 0));
                    strcat(str, "]:");
                    strcat(str, aux);
                    sqlite3_finalize(res2);
                }
            }
            sqlite3_finalize(res1);
            c--;
        }
        // printf("%s", str);
        sqlite3_close(datab);
        strcpy(ceva, "Istoric: ");
        if (strcmp(str, "[") == 0)
            strcpy(str, "Nu exista conversatii de afisat");
        strcat(ceva, str);
    }
}
// istoric cu fiecare
void istoric_sep(char nr[800], int *logged, char ceva[800], int *id)
{
    if (*logged == 0)
    {
        strcpy(ceva, "Nu v-ati logat inca");
    }
    else
    {

        int poz, k = 0;
        char user2[800];
        for (int j = 0; j < strlen(nr); j++)
        {
            if (nr[j] == ':')
            {
                poz = j + 1;
                break;
            }
        }
        bzero(user2, sizeof(user2));
        for (int j = poz; j < strlen(nr); j++)
        {
            user2[k++] = nr[j];
        }
        // printf("%s\n", user2);
        char str[800];
        strcpy(str, "\0");
        sqlite3_stmt *res, *res1, *res2, *res3, *res4;
        sqlite3 *datab;
        int result, count, i = 1, id2;
        char aux[800];
        int handle = sqlite3_open("db.db", &datab);
        char *db_err = 0;

        char *sql = sqlite3_mprintf("select count(id) from mesaje");
        int result1 = sqlite3_prepare_v2(datab, sql, -1, &res, 0);
        sqlite3_step(res);
        count = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
        char *sql3 = sqlite3_mprintf("select id from useri where username='%q';", user2);
        result = sqlite3_prepare_v2(datab, sql3, -1, &res, 0);
        if (sqlite3_step(res) != SQLITE_ROW)
        {
            strcpy(str, "Nu exista acest user");
        }
        else
        {

            id2 = sqlite3_column_int(res, 0);
            sqlite3_finalize(res);
            // printf("%d", count);
            while (i <= count)
            {
                bzero(x, sizeof(x));
                char *sql2 = sqlite3_mprintf("select message from mesaje where userid1='%d' and id='%d' and userid2='%d';", (int)(*id), i, id2);
                result = sqlite3_prepare_v2(datab, sql2, -1, &res2, 0);
                if (result == SQLITE_OK)
                {
                    if (sqlite3_step(res2) == SQLITE_ROW)
                    {
                        if (i != 1)
                            strcat(str, ";");
                        itoa(i, x);
                        strcat(str, x);
                        if (i >= 1 && i <= count)
                            strcat(str, "[");
                        bzero(aux, sizeof(aux));
                        char *sql1 = sqlite3_mprintf("select username from useri where id='%d'", (int)(*id));
                        result = sqlite3_prepare_v2(datab, sql1, -1, &res1, 0);
                        sqlite3_step(res1);
                        strcat(str, sqlite3_column_text(res1, 0));
                        sqlite3_finalize(res1);
                        strcat(str, "]->");
                        strcpy(aux, sqlite3_column_text(res2, 0));

                        strcat(str, "[");
                        strcat(str, user2);
                        strcat(str, "]:");
                        strcat(str, aux);
                        if (i == count)
                            strcat(str, ";");
                    }
                    else
                    {
                        bzero(aux, sizeof(aux));
                        char *sql4 = sqlite3_mprintf("select message from mesaje where userid2='%d' and id='%d' and userid1='%d';", (int)(*id), i, id2);
                        result = sqlite3_prepare_v2(datab, sql4, -1, &res1, 0);
                        if (result == SQLITE_OK)
                        {
                            if (sqlite3_step(res1) == SQLITE_ROW)
                            {
                                if (i != 1)
                                    strcat(str, ";");
                                itoa(i, x);
                                strcat(str, x);
                                strcpy(aux, sqlite3_column_text(res1, 0));

                                if (i >= 1 && i <= count)
                                    strcat(str, "[");

                                strcat(str, user2);
                                strcat(str, "]->[");

                                char *sql6 = sqlite3_mprintf("select username from useri where id='%d';", (int)(*id));
                                result = sqlite3_prepare_v2(datab, sql6, -1, &res3, 0);
                                sqlite3_step(res3);
                                strcat(str, sqlite3_column_text(res3, 0));
                                sqlite3_finalize(res3);
                                strcat(str, "]:");
                                strcat(str, aux);
                                if (i == count)
                                    strcat(str, ";");
                            }
                        }
                        sqlite3_finalize(res1);
                    }
                }
                sqlite3_finalize(res2);
                i++;
            }
            if (strcmp(str, "\0") == 0)
                strcpy(str, "Nu exista conversatie");
        }
        strcpy(ceva, str);
        sqlite3_close(datab);
    }
}
// exit
void inchidere(char nr[800], int *logged, char ceva[800], int *id)
{
    strcpy(ceva, "exit");
}
// reply
void reply(char nr[800], int *logged, char ceva[800], int *id)
{
    if (*logged == 0)
    {
        strcpy(ceva, "Nu v-ati logat inca");
    }
    else
    {
        char *tok, msj[800], numar[800], aux[800], raspuns[800];
        int poz1 = 0, poz2 = 0, poz3 = 0, n, id2, var1, var2;
        for (int i = 0; i < strlen(nr); i++)
        {
            if (nr[i] == 39) // caut '
            {
                if (poz1 == 0)
                    poz1 = i + 1;
                else
                    poz2 = i - 1;
            }
            if (nr[i] == '=')
                poz3 = i + 1;
        }
        int j = 0, k = 0;
        for (int i = poz1; i <= poz2; i++)
        {
            msj[j++] = nr[i];
        }
        int i = poz3;
        while (nr[i] != 32)
        {
            numar[k++] = nr[i];
            i++;
        }
        msj[j] = '\0';
        numar[k] = '\0';
        n = atoi(numar);
        printf("%s\n", msj);
        printf("%d\n", n);
        sqlite3_stmt *res, *res1;
        sqlite3 *datab;
        int handle = sqlite3_open("db.db", &datab);
        char *db_err = 0;
        char *sql2 = sqlite3_mprintf("Select message,userid1,userid2 from mesaje where id='%d' and (userid1='%d' or userid2='%d')", n, (int)(*id), (int)(*id));
        int result = sqlite3_prepare_v2(datab, sql2, -1, &res, 0);
        if (result == SQLITE_OK)
        {
            if (sqlite3_step(res) == SQLITE_ROW)
            {
                strcpy(ceva, "Reply trimis cu succes!");
                strcpy(aux, sqlite3_column_text(res, 0));
                printf("%s\n", aux);
                var1 = sqlite3_column_int(res, 1);
                var2 = sqlite3_column_int(res, 2);
                if (var1 == *id)
                    id2 = var2;
                else
                    id2 = var1;
                printf("%d\n", id2);
                strcpy(raspuns, "reply la mesajul '");
                strcat(raspuns, aux);
                strcat(raspuns, "' cu '");
                strcat(raspuns, msj);
                strcat(raspuns, "'");
                printf("%s\n", raspuns);
                char *sql9 = sqlite3_mprintf("insert into mesaje(userid1,userid2,message) values('%d','%d','%q');", (int)(*id), id2, raspuns);
                result = sqlite3_prepare_v2(datab, sql9, -1, &res1, 0);
                sqlite3_step(res1);
                sqlite3_finalize(res1);

                char *sql3 = sqlite3_mprintf("update useri set modificat=1 where id='%d'", id2);
                result = sqlite3_prepare_v2(datab, sql3, -1, &res1, 0);
                sqlite3_step(res1);
                sqlite3_finalize(res1);

                char s[800];
                char *sql5 = sqlite3_mprintf("select username from useri where id='%d'", (int)(*id));
                result = sqlite3_prepare_v2(datab, sql5, -1, &res1, 0);
                sqlite3_step(res1);
                strcpy(s, sqlite3_column_text(res1, 0));
                sqlite3_finalize(res1);

                char m[800];
                strcpy(m, "[");
                strcat(m, s);
                strcat(m, "]:");
                strcat(m, raspuns);

                char *sql4 = sqlite3_mprintf("update useri set mesaj='%q' where id='%d'", m, id2);
                result = sqlite3_prepare_v2(datab, sql4, -1, &res1, 0);
                sqlite3_step(res1);
                sqlite3_finalize(res1);

                char *sql1 = sqlite3_mprintf("select tdlcl from useri where id='%d' and online=1", id2);
                result = sqlite3_prepare_v2(datab, sql1, -1, &res1, 0);
                if (sqlite3_step(res1) == SQLITE_ROW)
                {
                    int td = sqlite3_column_int(res1, 0);
                   // write(td, m, strlen(m + 1) + 1);
                    sqlite3_finalize(res1);
                }
            }
            else
            {
                strcpy(ceva, "Nu e mesajul tau,nu poti da reply");
            }
        }
        // printf("%s", ceva);

        sqlite3_finalize(res);
        sqlite3_close(datab);
    }
}

void raspunde(void *arg)
{
    int id;
    int i = 0, num, nu = 0;
    char nr[800], rasp[800], a[800];
    char aux[800];
    struct thData tdL;
    int result;
    int logged, ok = 0;
    tdL = *((struct thData *)arg);
    /*pthread_t thread1;
   pthread_create(&thread1, NULL, &treat, NULL);
   pthread_detach(thread1);
   pthread_exit(NULL);*/
    while (1)
    {
        fflush(stdout);
        bzero(nr, sizeof(nr));
        if (num = read(tdL.cl, &nr, 255) <= 0)
        {
            printf("[Thread %d]\n", tdL.idThread);
            perror("Eroare la read() de la client.\n");
            break;
        }
        else
        {
            sqlite3_stmt *res, *res1;
            sqlite3 *datab;
            int handle = sqlite3_open("db.db", &datab);

            prefix(nr);
            printf("[Thread %d]Mesajul a fost receptionat...%s\n", tdL.idThread, nr);
            /*pregatim mesajul de raspuns */
            // bzero(rasp, sizeof(rasp));
            if (strcmp(nr, "da") == 0 || strcmp(nr, "nu") == 0)
            {
                sqlite3_stmt *res, *res1;
                sqlite3 *datab;
                handle = sqlite3_open("db.db", &datab);
                char *sql2 = sqlite3_mprintf("Select client from verificare");
                result = sqlite3_prepare_v2(datab, sql2, -1, &res, 0);
                if (result == SQLITE_OK)
                {
                    if (sqlite3_step(res) == SQLITE_ROW)
                    {
                        strcpy(aux, sqlite3_column_text(res, 0));
                        if (strcmp(nr, "da") == 0)
                        {

                            if (strcmp(aux, "User existent,v-ati autentificat deja. Va logati?") == 0)
                            {
                                // printf("%s", "login");
                                char *sql5 = sqlite3_mprintf("select comanda from verificare");
                                result = sqlite3_prepare_v2(datab, sql5, -1, &res1, 0);
                                sqlite3_step(res1);
                                char a[800];
                                bzero(a, sizeof(a));
                                bzero(nr, sizeof(nr));
                                strcpy(a, sqlite3_column_text(res1, 0));
                                char *tok;
                                tok = strtok(a, " ");
                                tok = strtok(NULL, " ");
                                strcpy(a, tok);
                                // printf("%s", a);
                                sqlite3_finalize(res1);

                                strcpy(nr, "login ");
                                strcat(nr, a);
                                printf("%s", nr);
                                sqlite3_close(datab);
                            }
                            else
                            {
                                if (strcmp(aux, "User inexistent. Va autentificati?") == 0)
                                {
                                    // printf("%s", "autentificare");
                                    char *sql6 = sqlite3_mprintf("select comanda from verificare");
                                    result = sqlite3_prepare_v2(datab, sql6, -1, &res1, 0);
                                    sqlite3_step(res1);
                                    char a[800];
                                    bzero(a, sizeof(a));
                                    bzero(nr, sizeof(nr));
                                    strcpy(a, sqlite3_column_text(res1, 0));
                                    char *tok;
                                    tok = strtok(a, " ");
                                    tok = strtok(NULL, " ");
                                    strcpy(a, tok);
                                    // printf("%s", a);
                                    sqlite3_finalize(res1);

                                    strcpy(nr, "autentificare ");
                                    strcat(nr, a);
                                    printf("%s", nr);
                                    sqlite3_close(datab);
                                }
                                else
                                {
                                    strcpy(rasp, "comanda inexistenta");
                                }
                            }
                        }
                        if (strcmp(nr, "nu") == 0)
                        {
                            strcpy(rasp, "ok!");
                            nu = 1;
                        }
                    }
                    else
                    {
                        strcpy(rasp, "Comanda inexistenta");
                    }
                }
                // printf("%s",ceva);
                sqlite3_finalize(res);
                sqlite3_close(datab);
            }
            if (nr[0] == 'l' && nr[1] == 'o' && nr[2] == 'g' && nr[3] == 'i' && nr[4] == 'n' && nr[5] == ' ')
            {
                logare(nr, &logged, rasp, &id, (struct thData *)arg);
            }
            else
            {
                if (nr[0] == 'a' && nr[1] == 'u' && nr[2] == 't' && nr[3] == 'e' && nr[4] == 'n' && nr[5] == 't' && nr[6] == 'i' && nr[7] == 'f' && nr[8] == 'i' && nr[9] == 'c' && nr[10] == 'a' && nr[11] == 'r' && nr[12] == 'e' && nr[13] == ' ')
                {

                    autentificare(nr, &logged, rasp, &id);
                }
                else
                {
                    if (nr[0] == 'l' && nr[1] == 'o' && nr[2] == 'g' && nr[3] == 'o' && nr[4] == 'u' && nr[5] == 't')
                    {
                        delogare(nr, &logged, rasp, &id, (struct thData *)arg);
                    }
                    else
                    {

                        if (nr[0] == 'l' && nr[1] == 'i' && nr[2] == 's' && nr[3] == 't' && nr[4] == 'a' && nr[5] == ' ' && nr[6] == 'o' && nr[7] == 'n' && nr[8] == 'l' && nr[9] == 'i' && nr[10] == 'n' && nr[11] == 'e')
                        {
                            lista_on(nr, &logged, rasp, &id);
                        }
                        else
                        {
                            if (nr[0] == 'l' && nr[1] == 'i' && nr[2] == 's' && nr[3] == 't' && nr[4] == 'a' && nr[5] == ' ' && nr[6] == 'u' && nr[7] == 's' && nr[8] == 'e' && nr[9] == 'r' && nr[10] == 'i')
                            {
                                lista(nr, &logged, rasp, &id);
                            }
                            else
                            {
                                if (nr[0] == 't' && nr[1] == 'r' && nr[2] == 'i' && nr[3] == 'm' && nr[4] == 'i' && nr[5] == 't' && nr[6] == 'e' && nr[7] == 'r' && nr[8] == 'e' && nr[9] == ' ')
                                {
                                    trimitere(nr, &logged, rasp, &id);
                                    ok = 1;
                                }
                                else
                                {
                                    if (nr[0] == 'i' && nr[1] == 's' && nr[2] == 't' && nr[3] == 'o' && nr[4] == 'r' && nr[5] == 'i' && nr[6] == 'c' && nr[7] == ' ' && nr[8] == 'c' && nr[9] == 'u')
                                    {
                                        istoric_sep(nr, &logged, rasp, &id);
                                    }
                                    else
                                    {
                                        if (nr[0] == 'i' && nr[1] == 's' && nr[2] == 't' && nr[3] == 'o' && nr[4] == 'r' && nr[5] == 'i' && nr[6] == 'c')
                                        {
                                            istoric(nr, &logged, rasp, &id);
                                        }
                                        else
                                        {
                                            if (nr[0] == 'e' && nr[1] == 'x' && nr[2] == 'i' && nr[3] == 't')
                                            {
                                                inchidere(nr, &logged, rasp, &id);
                                            }
                                            else
                                            {
                                                if (nr[0] == 'r' && nr[1] == 'e' && nr[2] == 'p' && nr[3] == 'l' && nr[4] == 'y' && nr[5] == ' ')
                                                {
                                                    reply(nr, &logged, rasp, &id);
                                                }
                                                else if (nu == 0)
                                                    strcpy(rasp, "Comanda inexistenta");
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            prefix(rasp);
            handle = sqlite3_open("db.db", &datab);
            char *sql = sqlite3_mprintf("update verificare set client='%q' where id=1", rasp);
            result = sqlite3_prepare_v2(datab, sql, -1, &res1, 0);
            sqlite3_step(res1);
            sqlite3_finalize(res1);
            sqlite3_close(datab);
            // printf("%s", rasp);
            // printf("%d", tdL.cl);
            // sleep(2);
            if (num = write(tdL.cl, rasp, strlen(rasp + 1) + 1) <= 0)
            {
                printf("[Thread %d] ", tdL.idThread);
                perror("[Thread]Eroare la write() catre client.\n");
            }
            else
            {
                rasp[num] = '\0';
            }
            fflush(stdout);
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
        }
    }
}
