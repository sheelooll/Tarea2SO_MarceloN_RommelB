#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define VACIO 0
#define PUMA 1
#define PERRO 2
#define INVALIDO -1

int tablero[7][5];
pthread_mutex_t mutex_tablero;

void inicializar_tablero() {
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 5; j++) {
            tablero[i][j] = VACIO;
        }
    }
    
    tablero[0][0] = INVALIDO; tablero[0][1] = INVALIDO;
    tablero[0][3] = INVALIDO; tablero[0][4] = INVALIDO;
    tablero[1][0] = INVALIDO; tablero[1][4] = INVALIDO;

    pthread_mutex_init(&mutex_tablero, NULL);
    printf("tablero ok con mutex\n");
}

// funcion que ejecuta el hilo del puma
void* rutina_puma(void* arg) {
    printf("puma: listo para jugar\n");
    return NULL;
}

// funcion que ejecutan los 12 hilos de los perros
void* rutina_perro(void* arg) {
    int id = *(int*)arg;
    printf("perro %d: listo para jugar\n", id);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("uso: %s n m [config.txt]\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);

    printf("torneo: %d jug, %d mesas\n", n, m);

    inicializar_tablero();

    // variables para guardar los hilos
    pthread_t hilo_puma;
    pthread_t hilos_perros[12];
    int ids_perros[12]; // para que cada perro sepa su numero

    // hilo del puma
    pthread_create(&hilo_puma, NULL, rutina_puma, NULL);

    // 12 hilos de los perros
    for (int i = 0; i < 12; i++) {
        ids_perros[i] = i + 1;
        pthread_create(&hilos_perros[i], NULL, rutina_perro, &ids_perros[i]);
    }

    // espera a que todos terminen para que el programa no muera antes
    pthread_join(hilo_puma, NULL);
    for (int i = 0; i < 12; i++) {
        pthread_join(hilos_perros[i], NULL);
    }

    pthread_mutex_destroy(&mutex_tablero);
    printf("partida finalizada limpiamente\n");

    return 0;
}
