#include <stdio.h>
#include <pthread.h>

// Variable global para el saldo
double saldo = 1000.0; // Saldo inicial

// Mutex para hacer las operaciones atómicas
pthread_mutex_t mutex_saldo;

// Función para realizar un depósito
void* depositar(void* arg) {
    double cantidad = *(double*)arg;
    pthread_mutex_lock(&mutex_saldo); // Bloquea el acceso a la variable saldo
    saldo += cantidad;
    printf("Depósito de: %.2f, Saldo actual: %.2f\n", cantidad, saldo);
    pthread_mutex_unlock(&mutex_saldo); // Desbloquea el acceso a la variable saldo
    return NULL;
}

// Función para realizar un retiro
void* retirar(void* arg) {
    double cantidad = *(double*)arg;
    pthread_mutex_lock(&mutex_saldo); // Bloquea el acceso a la variable saldo
    if (cantidad <= saldo) {
        saldo -= cantidad;
        printf("Retiro de: %.2f, Saldo actual: %.2f\n", cantidad, saldo);
    } else {
        printf("Saldo insuficiente para retirar: %.2f, Saldo actual: %.2f\n", cantidad, saldo);
    }
    pthread_mutex_unlock(&mutex_saldo); // Desbloquea el acceso a la variable saldo
    return NULL;
}

int main() {
    pthread_t hilo_deposito, hilo_retiro;

    // Inicializa el mutex
    pthread_mutex_init(&mutex_saldo, NULL);

    // Cantidades de depósito y retiro
    double cantidad_deposito = 500.0;
    double cantidad_retiro = 300.0;

    // Crear hilos
    pthread_create(&hilo_deposito, NULL, depositar, &cantidad_deposito);
    pthread_create(&hilo_retiro, NULL, retirar, &cantidad_retiro);

    // Esperar a que los hilos terminen
    pthread_join(hilo_deposito, NULL);
    pthread_join(hilo_retiro, NULL);

    // Destruir el mutex
    pthread_mutex_destroy(&mutex_saldo);

    return 0;
}
