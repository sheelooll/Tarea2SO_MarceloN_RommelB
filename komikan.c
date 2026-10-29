#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define VACIO 0
#define PUMA 1
#define PERRO 2
#define INVALIDO -1

int tablero[7][5];
pthread_mutex_t mutex_tablero;

// funcion para imprimir el estado actual de la matriz
void mostrar_tablero() {
    pthread_mutex_lock(&mutex_tablero);
    printf("\n--- ESTADO DEL TABLERO ---\n");
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 5; j++) {
            if (tablero[i][j] == INVALIDO) {
                printf("   ");
            } else if (tablero[i][j] == VACIO) {
                printf(" . ");
            } else if (tablero[i][j] == PUMA) {
                printf(" X "); // X representa al Puma
            } else if (tablero[i][j] == PERRO) {
                printf(" O "); // O representa a los Perros
            }
        }
        printf("\n");
    }
    printf("--------------------------\n\n");
    pthread_mutex_unlock(&mutex_tablero);
}

void inicializar_tablero() {
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 5; j++) {
            tablero[i][j] = VACIO;
        }
    }
    
    // anular casillas fuera del mapa
    tablero[0][0] = INVALIDO; tablero[0][1] = INVALIDO;
    tablero[0][3] = INVALIDO; tablero[0][4] = INVALIDO;
    tablero[1][0] = INVALIDO; tablero[1][4] = INVALIDO;

    // posicion inicial del puma
    tablero[4][2] = PUMA;

    // posiciones iniciales de los 12 perros
    for (int j = 0; j < 5; j++) tablero[2][j] = PERRO; // 5 perros en fila 2
    for (int j = 0; j < 5; j++) tablero[3][j] = PERRO; // 5 perros en fila 3
    tablero[4][0] = PERRO;                             // 1 perro
    tablero[4][4] = PERRO;                             // 1 perro

    pthread_mutex_init(&mutex_tablero, NULL);
    printf("tablero e inicializacion de piezas ok\n");
}

void retardo_aleatorio() {
    int delay_ms = (rand() % (300 - 50 + 1)) + 50;
    usleep(delay_ms * 1000); 
}

void* rutina_puma(void* arg) {
    (void)arg;
    retardo_aleatorio();
    printf("puma: listo en posicion (4,2)\n");
    return NULL;
}

void* rutina_perro(void* arg) {
    int id = *(int*)arg;
    retardo_aleatorio();
    printf("perro %d: listo en posicion inicial\n", id);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("uso: %s n m [config.txt]\n", argv[0]);
        return 1;
    }

    srand(time(NULL)); 

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);

    printf("torneo: %d jug, %d mesas\n", n, m);

    inicializar_tablero();
    
    // tablero cargado
    mostrar_tablero();

    pthread_t hilo_puma;
    pthread_t hilos_perros[12];
    int ids_perros[12];

    pthread_create(&hilo_puma, NULL, rutina_puma, NULL);

    for (int i = 0; i < 12; i++) {
        ids_perros[i] = i + 1;
        pthread_create(&hilos_perros[i], NULL, rutina_perro, &ids_perros[i]);
    }

    pthread_join(hilo_puma, NULL);
    for (int i = 0; i < 12; i++) {
        pthread_join(hilos_perros[i], NULL);
    }

    pthread_mutex_destroy(&mutex_tablero);
    printf("partida finalizada\n");

    return 0;
}
