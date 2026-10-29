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

int df[] = {-1, 1, 0, 0, -1, -1, 1, 1};
int dc[] = {0, 0, -1, 1, -1, 1, -1, 1};

typedef struct {
    int id;
    int f;
    int c;
} Posicion;

int tablero[7][5];
pthread_mutex_t mutex_tablero;

int perros_capturados = 0;
int juego_terminado = 0; // avisa a los hilos que la partida concluyo

void mostrar_tablero() {
    pthread_mutex_lock(&mutex_tablero);
    printf("\n--- ESTADO DEL TABLERO (Capturados: %d) ---\n", perros_capturados);
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
    printf("-------------------------------------------\n\n");
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

// intenta mover o capturar si es el puma
int intentar_accion_puma(int f, int c, int dir, int *f_out, int *c_out) {
    int f_adj = f + df[dir];
    int c_adj = c + dc[dir];

    if (f_adj < 0 || f_adj >= 7 || c_adj < 0 || c_adj >= 5) return 0;

    pthread_mutex_lock(&mutex_tablero);

    // movimiento a casilla vacia
    if (tablero[f_adj][c_adj] == VACIO) {
        tablero[f][c] = VACIO;
        tablero[f_adj][c_adj] = PUMA;
        *f_out = f_adj;
        *c_out = c_adj;
        pthread_mutex_unlock(&mutex_tablero);
        return 1;
    }

    // captura saltando sobre un perro
    if (tablero[f_adj][c_adj] == PERRO) {
        int f_salto = f + 2 * df[dir];
        int c_salto = c + 2 * dc[dir];

        if (f_salto >= 0 && f_salto < 7 && c_salto >= 0 && c_salto < 5) {
            if (tablero[f_salto][c_salto] == VACIO) {
                tablero[f][c] = VACIO;
                tablero[f_adj][c_adj] = VACIO; // perro capturado
                tablero[f_salto][c_salto] = PUMA;
                perros_capturados++;
                printf("puma: CAPTURO a un perro en (%d, %d)!\n", f_adj, c_adj);
                if (perros_capturados >= 6) {
                    juego_terminado = 1;
                    printf("¡VICTORIA DEL PUMA! Capturo 6 perros.\n");
                }

                *f_out = f_salto;
                *c_out = c_salto;
                pthread_mutex_unlock(&mutex_tablero);
                return 1;
            }
        }
    }

    pthread_mutex_unlock(&mutex_tablero);
    return 0;
}

int intentar_mover_perro(int f_act, int c_act, int f_sig, int c_sig) {
    if (f_sig < 0 || f_sig >= 7 || c_sig < 0 || c_sig >= 5) return 0;

    pthread_mutex_lock(&mutex_tablero);
    if (tablero[f_sig][c_sig] == VACIO) {
        tablero[f_act][c_act] = VACIO;
        tablero[f_sig][c_sig] = PERRO;
        pthread_mutex_unlock(&mutex_tablero);
        return 1;
    }
    pthread_mutex_unlock(&mutex_tablero);
    return 0;
}

void* rutina_puma(void* arg) {
    (void)arg;
    int f = 4, c = 2;

    while (!juego_terminado) {
        retardo_aleatorio();
        int dir = rand() % 8;
        int f_nueva, c_nueva;

        if (intentar_accion_puma(f, c, dir, &f_nueva, &c_nueva)) {
            f = f_nueva;
            c = c_nueva;
            mostrar_tablero();
        }
    }
    return NULL;
}

void* rutina_perro(void* arg) {
    Posicion* p = (Posicion*)arg;
    int f = p->f;
    int c = p->c;

    while (!juego_terminado) {
        retardo_aleatorio();
        int dir = rand() % 8;
        int f_dest = f + df[dir];
        int c_dest = c + dc[dir];

        if (intentar_mover_perro(f, c, f_dest, c_dest)) {
            f = f_dest;
            c = c_dest;
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

    tablero[4][2] = PUMA;

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
