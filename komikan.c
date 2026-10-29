#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h> // para los numeros aleatorios

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

// funcion auxiliar que genera el retardo aleatorio entre 50 y 300 ms
void retardo_aleatorio() {
    int delay_ms = (rand() % (300 - 50 + 1)) + 50;
    // usleep trabaja en microsegundos, multiplica por 1000
    usleep(delay_ms * 1000); 
}

void* rutina_puma(void* arg) {
    (void)arg; 
    
    printf("puma: calculando movimiento...\n");
    retardo_aleatorio();
    printf("puma: listo para jugar despues del retardo\n");
    
    return NULL;
}

void* rutina_perro(void* arg) {
    int id = *(int*)arg;
    
    printf("perro %d: calculando movimiento...\n", id);
    retardo_aleatorio();
    printf("perro %d: listo para jugar despues del retardo\n", id);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("uso: %s n m [config.txt]\n", argv[0]);
        return 1;
    }

    // semilla aleatoria para los retardos
    srand(time(NULL)); 

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);

    printf("torneo: %d jug, %d mesas\n", n, m);

    inicializar_tablero();

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
    printf("partida finalizada limpiamente\n");

    return 0;
}
