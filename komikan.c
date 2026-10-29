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

// estructura para pasarle a cada perro su id y posicion inicial
typedef struct {
    int id;
    int f;
    int c;
} Posicion;

int tablero[7][5];
pthread_mutex_t mutex_tablero;

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
                printf(" X ");
            } else if (tablero[i][j] == PERRO) {
                printf(" O ");
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
    
    tablero[0][0] = INVALIDO; tablero[0][1] = INVALIDO;
    tablero[0][3] = INVALIDO; tablero[0][4] = INVALIDO;
    tablero[1][0] = INVALIDO; tablero[1][4] = INVALIDO;

    pthread_mutex_init(&mutex_tablero, NULL);
}

void retardo_aleatorio() {
    int delay_ms = (rand() % (300 - 50 + 1)) + 50;
    usleep(delay_ms * 1000); 
}

// funcion atómica con mutex para mover fichas
int intentar_mover(int f_act, int c_act, int f_sig, int c_sig, int ficha) {
    if (f_sig < 0 || f_sig >= 7 || c_sig < 0 || c_sig >= 5) return 0;

    pthread_mutex_lock(&mutex_tablero);
    if (tablero[f_sig][c_sig] == VACIO) {
        tablero[f_act][c_act] = VACIO;
        tablero[f_sig][c_sig] = ficha;
        pthread_mutex_unlock(&mutex_tablero);
        return 1;
    }
    pthread_mutex_unlock(&mutex_tablero);
    return 0;
}

void* rutina_puma(void* arg) {
    (void)arg;
    int f = 4, c = 2; // posicion inicial puma

    for (int i = 0; i < 3; i++) { // 3 movimientos de prueba
        retardo_aleatorio();
        int f_destino = f + 1; // intenta avanzar abajo
        if (intentar_mover(f, c, f_destino, c, PUMA)) {
            printf("puma: movio a (%d, %d)\n", f_destino, c);
            f = f_destino;
            mostrar_tablero();
        } else {
            printf("puma: no pudo mover a (%d, %d)\n", f_destino, c);
        }
    }
    return NULL;
}

void* rutina_perro(void* arg) {
    Posicion* p = (Posicion*)arg;
    int f = p->f;
    int c = p->c;

    for (int i = 0; i < 3; i++) { // 3 movimientos de prueba
        retardo_aleatorio();
        int f_destino = f + 1; // intenta avanzar abajo
        if (intentar_mover(f, c, f_destino, c, PERRO)) {
            printf("perro %d: movio a (%d, %d)\n", p->id, f_destino, c);
            f = f_destino;
            mostrar_tablero();
        }
    }
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

    pthread_t hilo_puma;
    pthread_t hilos_perros[12];
    Posicion pos_perros[12];

    // ubica al puma
    tablero[4][2] = PUMA;

    // ubica a los 12 perros
    int idx = 0;
    for (int j = 0; j < 5; j++) {
        tablero[2][j] = PERRO;
        pos_perros[idx] = (Posicion){idx + 1, 2, j};
        idx++;
    }
    for (int j = 0; j < 5; j++) {
        tablero[3][j] = PERRO;
        pos_perros[idx] = (Posicion){idx + 1, 3, j};
        idx++;
    }
    tablero[4][0] = PERRO;
    pos_perros[10] = (Posicion){11, 4, 0};
    tablero[4][4] = PERRO;
    pos_perros[11] = (Posicion){12, 4, 4};

    mostrar_tablero();

    pthread_create(&hilo_puma, NULL, rutina_puma, NULL);

    for (int i = 0; i < 12; i++) {
        pthread_create(&hilos_perros[i], NULL, rutina_perro, &pos_perros[i]);
    }

    pthread_join(hilo_puma, NULL);
    for (int i = 0; i < 12; i++) {
        pthread_join(hilos_perros[i], NULL);
    }

    pthread_mutex_destroy(&mutex_tablero);
    printf("partida finalizada\n");

    return 0;
}
