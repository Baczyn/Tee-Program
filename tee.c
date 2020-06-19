#include <unistd.h> //lib for accces
#include "signal.h"
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int flag =0;

int fileExist(int nfile, char *files[])
{
    for (short i = 0; i < nfile; i++)
    {
        if (access(files[i], F_OK) == 0)
        {
            if(write(STDERR_FILENO, "File: exists.\nUse append -a, or truncate -t flag\n", 50)<0)
                perror("Error - writing on STDOUT ");

            return 0;
        }
    }
    return -1;
}

sig_atomic_t stop = 0;
void handler(int signum)
{
    if (signum == SIGINT)
        stop = 1;
}

void tee(int nfile, char *files[], int flag)
{
    size_t BUFSIZ = 8192;
    if(write(STDOUT_FILENO, "To save your work to file(s) press CTRL + D,\nTo exit press CTRL C\n", 67)<0)
        perror("Error - writing on STDOUT ");
    char buffer[BUFSIZ];
    int bytes_read = 0;
    int temp = 1;

    while (1)
    {
        if (stop == 1)
        {
            _exit(0);
        }
        temp = read(STDIN_FILENO, buffer + bytes_read, sizeof buffer - bytes_read);
        if(temp < 0 || errno==EINTR)
            perror("Error - reading from STDIO ");
        else if (temp != 0)
        {
            bytes_read += temp;
        }
        else
        {
            for (short i = 0; i < nfile; ++i)
            {
                

               
                    int file = open(files[i], flag, 0777);
                    if (file == -1)
                    {
                        perror("Error - opening file");
                    }
                    else
                    {
                        if(write(file, buffer, bytes_read)<0)
                            perror("Error - writing to file ");
                        if(close(file)<0)
                            perror("Error - closing the file ");
                    }

            }
            temp = 1;
            bytes_read = 0;
            flag = (O_WRONLY | O_APPEND | O_CREAT);
        }
    }
}

int main(int argc, char *argv[])
{

    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_flags = SA_RESTART; /* Restart functions if interrupted by handler */
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    if(sigaction(SIGINT, &sa, NULL)<0)
        perror("Error - sigction");

    if (argc < 2)
    {
        if(write(STDERR_FILENO, "Run program with name(s) of file(s) as argument(s)\n", 52)<0)
            perror("Error - writing in STDERR");
        return -1;
    }
    else if (argv[1][0] == '-' && argv[1][1] == 'a')
        flag = O_WRONLY | O_APPEND | O_CREAT;

    else if (argv[1][0] == '-' && argv[1][1] == 't')
        flag = O_CREAT|O_WRONLY | O_TRUNC | O_CREAT;

    if (flag != ' ')
        tee(argc - 2, argv + 2, flag);

    else if (fileExist(argc - 1, argv + 1))
        tee(argc - 1, argv + 1, (O_WRONLY | O_CREAT));

    return 0;
}
