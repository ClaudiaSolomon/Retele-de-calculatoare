#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <sqlite3.h>
/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
#define PORT 2906
char t[80];
void prefix(char d[80])
{
  int i;
  bzero(t, sizeof(t));
  for (i = 0; i < strlen(d); i++)
  {
    if (d[i] < 32 || d[i] > 126)
      break;
    else
    {
      if (d[i] == ';')
        t[i] = '\n';
      else
        t[i] = d[i];
    }
  }
  strcpy(d, t);
}

int main(int argc, char *argv[])
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
                             // mesajul trimis
  char buf[800];

  /* stabilim portul */

  /* cream socketul */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  /* portul de conectare */
  server.sin_port = htons(PORT);
  /* ne conectam la server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }
  fflush(stdout);
  int num, n, m, num2 = 10, x, k = 0;
  char s[800], d[800], nr[800], a[800], *tok, user[800];
  printf("Comenzi disponibile:\n -autentificare username\n -login username\n -lista useri\n -lista online\n -trimitere mesaj='...' catre:username\n -reply la mesajul cu numarul=x cu '...'\n -istoric\n -istoric cu:username\n -logout\n -exit\n");
  printf("Tastati comanda: \n");
  bzero(user, sizeof(user));
  while (1)
  {
    bzero(nr, sizeof(nr));
    fflush(stdout);
    read(0, &nr, sizeof(nr));
    prefix(nr);
    int poz3;
    // bzero(user, sizeof(user));
    if (nr[0] == 'l' && nr[1] == 'o' && nr[2] == 'g' && nr[3] == 'i' && nr[4] == 'n' && nr[5] == ' ')
    {
      for (int i = 6; i < strlen(nr); i++)
      {
        user[k++] = nr[i];
      }
    }
    // printf("%s", user);
    fflush(stdout);
    // printf("[client] Am citit %s\n", nr);

    // trimiterea mesajului la server
    if (num = write(sd, &nr, strlen(nr)) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      return errno;
    }
    else
    {
      do
      {
        // nr[num]='\0';
        fflush(stdout);
        bzero(d, sizeof(d));
        // citirea raspunsului dat de server
        //(apel blocant pina cind serverul raspunde)
        if (n = read(sd, &d, 800) <= 0)
        {
          perror("[client]Eroare la read() de la server.\n");
          return errno;
        }
        else
        {
          fflush(stdout);
          // afisam mesajul primit
          prefix(d);
          if (strcmp(d, "exit") == 0)
          {
            exit(1);
          }
          else
          {
            printf(" %s\n", d);
          }
          num2 = -1;
        }

      } while (num2 > 0);
    }
    sqlite3_stmt *res, *res1;
    sqlite3 *datab;
    int handle = sqlite3_open("db.db", &datab);
    // printf("%s", user);
    char *sql = sqlite3_mprintf("select mesaj from useri where modificat=1 and username='%q'", user);
    int result = sqlite3_prepare_v2(datab, sql, -1, &res1, 0);
    if (sqlite3_step(res1) == SQLITE_ROW)
    {
      // char a[256];
      // printf("aici");
      bzero(a, sizeof(a));
      strcpy(a, sqlite3_column_text(res1, 0));
      printf("%s\n", a);

      char *sql1 = sqlite3_mprintf("update useri set modificat=0 where username='%q'", user);
      result = sqlite3_prepare_v2(datab, sql1, -1, &res, 0);
      if (sqlite3_step(res) != SQLITE_ROW)
      {
        //perror("eroare update");
      }
      sqlite3_finalize(res);
    }
    sqlite3_finalize(res1);
    sqlite3_close(datab);
  }

  /* inchidem conexiunea, am terminat */
  close(sd);
}