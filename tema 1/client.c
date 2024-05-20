#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
int main(int argc, char *argv[])
{
    int num, num2, fr, fw, x;
    char s[300], d[300], *tok;
    fw = open("fw", O_WRONLY);
    fr = open("fr", O_RDONLY);
    printf("Tastati comanda: \n");
    while (gets(s), !feof(stdin))
    {
        if ((num = write(fw, s, strlen(s))) == -1)
            perror("problema la scriere in fifo1");
        else
        {
            do
            {
                if ((num2 = read(fr, &d, 300)) == -1)
                    printf("Problema la citire in fifo2");
                else
                {
                    if (d[2] == 'b' && d[3] == 'r' && d[4] == 'e' && d[5] == 'a' && d[6] == 'k')
                    { 
                        exit(1);
                       
                    }
                    else
                    {
                        tok = strtok(d, " ");
                        x = atoi(tok);
                        for (int i = 3; i < x + 3; i++)
                        {
                            printf("%c", d[i]);
                        }
                        printf("\n");
                        num2 = -1;
                    }
                }
            } while (num2 > 0);
        }
    }
}
