#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 1024

void read_and_distribute_lines(const char *filename, int pipe1[], int pipe2[]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 0;

    // Cierra las entradas de las tuberías (para lectura)
    close(pipe1[0]);
    close(pipe2[0]);

    printf("Reading file and distributing lines...\n");
    while (fgets(line, sizeof(line), file)) {
        if (line_number % 2 == 0) {
            // Línea par, escribir en pipe1
            printf("Writing to pipe1: %s", line);
            write(pipe1[1], line, strlen(line));
        } else {
            // Línea impar, escribir en pipe2
            printf("Writing to pipe2: %s", line);
            write(pipe2[1], line, strlen(line));
        }
        line_number++;
    }

    // Cerrar las salidas de las tuberías después de escribir
    close(pipe1[1]);
    close(pipe2[1]);

    fclose(file);
    printf("Finished distributing lines.\n");
}

void grep_lines(int pipe_in[], int pipe_out[], const char *word) {
    // Redirigir la entrada estándar a la tubería de entrada
    dup2(pipe_in[0], STDIN_FILENO);
    // Redirigir la salida estándar a la tubería de salida
    dup2(pipe_out[1], STDOUT_FILENO);

    // Cierra las tuberías que ya no son necesarias
    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
    close(pipe_out[1]);

    execlp("grep", "grep", word, NULL);
    perror("Error executing grep");
    exit(EXIT_FAILURE);
}

void sort_lines(int pipe_in[]) {
    // Redirigir la entrada estándar a la tubería de entrada
    dup2(pipe_in[0], STDIN_FILENO);

    // Cierra las tuberías que ya no son necesarias
    close(pipe_in[0]);
    close(pipe_in[1]);

    execlp("sort", "sort", NULL);
    perror("Error executing sort");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <filename> <word1> <word2>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    const char *word1 = argv[2];
    const char *word2 = argv[3];

    int pipe1[2], pipe2[2], pipe3[2];

    // Crear las tuberías
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1 || pipe(pipe3) == -1) {
        perror("Error creating pipes");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Proceso hijo 1 - Leer archivo y distribuir líneas
        read_and_distribute_lines(filename, pipe1, pipe2);
        exit(EXIT_SUCCESS);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Proceso hijo 2 - Grep en líneas pares
        grep_lines(pipe1, pipe3, word1);
        exit(EXIT_SUCCESS);
    }

    pid_t pid3 = fork();
    if (pid3 == 0) {
        // Proceso hijo 3 - Grep en líneas impares
        grep_lines(pipe2, pipe3, word2);
        exit(EXIT_SUCCESS);
    }

    pid_t pid4 = fork();
    if (pid4 == 0) {
        // Proceso hijo 4 - Sort de la salida del grep
        sort_lines(pipe3);
        exit(EXIT_SUCCESS);
    }

    // Cerrar las tuberías en el proceso padre
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);
    close(pipe3[0]);
    close(pipe3[1]);

    // Esperar a que todos los hijos terminen
    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);

    printf("All child processes finished.\n");

    return 0;
}
