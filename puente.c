#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>  // Para usar la función sleep

#define NUM_VEHICULOS 50  // Número de vehículos por sentido

// Variables globales y semáforos
pthread_mutex_t puente_lock;
sem_t prioridad_norte, prioridad_sur;
int sentido_actual = -1;  // -1 indica que no hay sentido actual, 0 para norte, 1 para sur
int vehiculos_en_puente = 0;
int esperando_norte = 0;
int esperando_sur = 0;

// Función para simular el paso del vehículo por el puente
void *vehiculo(void *arg) {
    int sentido = *((int *)arg);
    usleep(rand() % 200000);  // Espera al azar hasta 0.2 segundos antes de intentar entrar

    pthread_mutex_lock(&puente_lock);
    printf("Vehículo %s esperando para entrar al puente.\n", sentido == 0 ? "norte" : "sur");

    if (sentido == 0) {  // Sentido norte
        esperando_norte++;
        while ((sentido_actual == 1) || (esperando_sur > 0 && sentido_actual == -1)) {
            pthread_mutex_unlock(&puente_lock);
            sem_wait(&prioridad_norte);  // Espera hasta que se le dé prioridad al norte
            pthread_mutex_lock(&puente_lock);
        }
        esperando_norte--;
    } else {  // Sentido sur
        esperando_sur++;
        while ((sentido_actual == 0) || (esperando_norte > 0 && sentido_actual == -1)) {
            pthread_mutex_unlock(&puente_lock);
            sem_wait(&prioridad_sur);  // Espera hasta que se le dé prioridad al sur
            pthread_mutex_lock(&puente_lock);
        }
        esperando_sur--;
    }

    // Entra en el puente
    sentido_actual = sentido;
    vehiculos_en_puente++;
    printf("Vehículo %s entrando al puente. Vehículos en el puente: %d\n", sentido == 0 ? "norte" : "sur", vehiculos_en_puente);
    pthread_mutex_unlock(&puente_lock);

    sleep(1);  // Simula el tiempo que tarda en cruzar el puente

    pthread_mutex_lock(&puente_lock);
    vehiculos_en_puente--;
    printf("Vehículo %s salió del puente. Vehículos restantes en el puente: %d\n", sentido == 0 ? "norte" : "sur", vehiculos_en_puente);

    // Si el puente está vacío, cambiar el sentido actual
    if (vehiculos_en_puente == 0) {
        sentido_actual = -1;
        // Despertar a los vehículos que están esperando del otro sentido si hay alguno
        if (esperando_norte > 0) {
            sem_post(&prioridad_norte);
        } else if (esperando_sur > 0) {
            sem_post(&prioridad_sur);
        }
    }
    pthread_mutex_unlock(&puente_lock);

    free(arg);
    return NULL;
}

// Función para crear y gestionar los vehículos
void crear_vehiculos() {
    pthread_t hilos[NUM_VEHICULOS * 2];
    for (int i = 0; i < NUM_VEHICULOS * 2; i++) {
        int *sentido = malloc(sizeof(int));
        *sentido = i % 2;  // Alterna el sentido entre 0 (norte) y 1 (sur)
        pthread_create(&hilos[i], NULL, vehiculo, sentido);
    }

    for (int i = 0; i < NUM_VEHICULOS * 2; i++) {
        pthread_join(hilos[i], NULL);
    }
}

int main() {
    srand(time(NULL));  // Inicializa la semilla para generar números aleatorios

    // Inicializar el mutex y los semáforos
    pthread_mutex_init(&puente_lock, NULL);
    sem_init(&prioridad_norte, 0, 0);
    sem_init(&prioridad_sur, 0, 0);

    crear_vehiculos();

    // Destruir el mutex y los semáforos
    pthread_mutex_destroy(&puente_lock);
    sem_destroy(&prioridad_norte);
    sem_destroy(&prioridad_sur);

    return 0;
}
