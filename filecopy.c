#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

long get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    long file_size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    return file_size;
}

void write_file(const char *dest_file_name, char *buffer)
{
    FILE *fptr = fopen(dest_file_name, "w");
    fprintf(fptr, "%s", buffer);
    fclose(fptr);
}

char *read_file(const char *input_file_name)
{
    FILE *fptr = fopen(input_file_name, "r");

    long file_size = get_file_size(fptr);

    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL)
    {
        perror("Memory allocation failed");
        fclose(fptr);
        exit(1);
    }

    fread(buffer, sizeof(char), file_size, fptr);
    buffer[file_size] = '\0';
    fclose(fptr);

    return buffer;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "The arguments must be equal 3");
    }

    pid_t pid;
    int fd[2];
    char **args = &argv[1];
    const char *input_file_name = args[0];
    const char *destination_file_name = args[1];

    char *content_input_file = read_file(input_file_name);
    size_t file_size = strlen(content_input_file);

    if (pipe(fd) == -1)
    {
        perror("Pipe failed");
        return 1;
    }

    pid = fork();

    if (pid < 0)
    {
        perror("Fork Failed");
        return 1;
    }

    if (pid == 0)
    {
        close(fd[1]);
        char buffer[file_size];
        if (read(fd[0], buffer, file_size + 1) == -1)
        {
            perror("Read failed");
            exit(1);
        }
        close(fd[0]);

        write_file(destination_file_name, buffer);
    }
    else
    {
        close(fd[0]);

        if (write(fd[1], content_input_file, file_size + 1) == -1)
        {
            perror("Write to pipe failed");
            return 1;
        }

        close(fd[1]);

        wait(NULL);

        printf("Complete Child Process\n");
    }

    return 0;
}
