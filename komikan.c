#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define VACIO 0
#define PUMA 1
#define PERRO 2
#define INVALIDO -1

int df[] = {-1, 1, 0, 0, -1, -1, 1, 1};
int dc[] = {0, 0, -1, 1, -1, 1, -1, 1};

sem_t sem_mesas_libres;

// estructura para aislar los datos de cada partida
typedef struct {
    int id_partida;
    int id_mesa;
    int tablero[7][5];
    pthread_mutex_t mutex_tablero;
    int perros_capturados;
    int juego_terminado;
} Partida;

typedef struct {
    int id;
    int f;
    int c;
    Partida* partida;
} InfoPerro;

typedef struct {
    Partida* partida;
} InfoPuma;

void mostrar_tablero(Partida* p) {
    pthread_mutex_lock(&p->mutex_tablero);
    printf("\n--- PARTIDA %d (MESA %d) | Capturados: %d ---\n", p->id_partida, p->id_mesa, p->perros_capturados);
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 5; j++) {
            if (p->tablero[i][j] == INVALIDO) {
                printf("   ");
            } else if (p->tablero[i][j] == VACIO) {
                printf(" . ");
            } else if (p->tablero[i][j] == PUMA) {
                printf(" X ");
            } else if (p->tablero[i][j] == PERRO) {
                printf(" O ");
            }
        }
        printf("\n");
    }
    printf("---------------------------------------------\n\n");
    pthread_mutex_unlock(&p->mutex_tablero);
}

void inicializar_tablero(Partida* p) {
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 5; j++) {
            p->tablero[i][j] = VACIO;
        }
    }
    p->tablero[0][0] = INVALIDO; p->tablero[0][1] = INVALIDO;
    p->tablero[0][3] = INVALIDO; p->tablero[0][4] = INVALIDO;
    p->tablero[1][0] = INVALIDO; p->tablero[1][4] = INVALIDO;

    p->perros_capturados = 0;
    p->juego_terminado = 0;
    pthread_mutex_init(&p->mutex_tablero, NULL);
}

void retardo_aleatorio() {
    int delay_ms = (rand() % (300 - 50 + 1)) + 50;
    usleep(delay_ms * 1000); 
}

int intentar_accion_puma(Partida* p, int f, int c, int dir, int *f_out, int *c_out) {
    int f_adj = f + df[dir];
    int c_adj = c + dc[dir];

    if (f_adj < 0 || f_adj >= 7 || c_adj < 0 || c_adj >= 5) return 0;

    pthread_mutex_lock(&p->mutex_tablero);

    if (p->tablero[f_adj][c_adj] == VACIO) {
        p->tablero[f][c] = VACIO;
        p->tablero[f_adj][c_adj] = PUMA;
        *f_out = f_adj;
        *c_out = c_adj;
        pthread_mutex_unlock(&p->mutex_tablero);
        return 1;
    }

    if (p->tablero[f_adj][c_adj] == PERRO) {
        int f_salto = f + 2 * df[dir];
        int c_salto = c + 2 * dc[dir];

        if (f_salto >= 0 && f_salto < 7 && c_salto >= 0 && c_salto < 5) {
            if (p->tablero[f_salto][c_salto] == VACIO) {
                p->tablero[f][c] = VACIO;
                p->tablero[f_adj][c_adj] = VACIO;
                p->tablero[f_salto][c_salto] = PUMA;
                p->perros_capturados++;
                if (p->perros_capturados >= 6) {
                    p->juego_terminado = 1;
                    printf("¡VICTORIA DEL PUMA en Partida %d! Capturo 6 perros.\n", p->id_partida);
                }

                *f_out = f_salto;
                *c_out = c_salto;
                pthread_mutex_unlock(&p->mutex_tablero);
                return 1;
            }
        }
    }

    pthread_mutex_unlock(&p->mutex_tablero);
    return 0;
}

int intentar_mover_perro(Partida* p, int f_act, int c_act, int f_sig, int c_sig) {
    if (f_sig < 0 || f_sig >= 7 || c_sig < 0 || c_sig >= 5) return 0;

    pthread_mutex_lock(&p->mutex_tablero);
    if (p->tablero[f_sig][c_sig] == VACIO) {
        p->tablero[f_act][c_act] = VACIO;
        p->tablero[f_sig][c_sig] = PERRO;
        pthread_mutex_unlock(&p->mutex_tablero);
        return 1;
    }
    pthread_mutex_unlock(&p->mutex_tablero);
    return 0;
}

void* rutina_puma(void* arg) {
    InfoPuma* info = (InfoPuma*)arg;
    Partida* p = info->partida;
    int f = 4, c = 2;

    while (!p->juego_terminado) {
        retardo_aleatorio();
        int dir = rand() % 8;
        int f_nueva, c_nueva;

        if (intentar_accion_puma(p, f, c, dir, &f_nueva, &c_nueva)) {
            f = f_nueva;
            c = c_nueva;
        }
    }
    return NULL;
}

void* rutina_perro(void* arg) {
    InfoPerro* info = (InfoPerro*)arg;
    Partida* p = info->partida;
    int f = info->f;
    int c = info->c;

    while (!p->juego_terminado) {
        retardo_aleatorio();
        int dir = rand() % 8;
        int f_dest = f + df[dir];
        int c_dest = c + dc[dir];

        if (intentar_mover_perro(p, f, c, f_dest, c_dest)) {
            f = f_dest;
            c = c_dest;
        }
    }
    return NULL;
}

void* rutina_juez_mesa(void* arg) {
    Partida* p = (Partida*)arg;
    printf("[Matchmaker] Partida %d esperando mesa libre...\n", p->id_partida);
    sem_wait(&sem_mesas_libres);

    printf(">>> INICIANDO PARTIDA %d en MESA %d <<<\n", p->id_partida, p->id_mesa);

    inicializar_tablero(p);

    pthread_t hilo_puma;
    pthread_t hilos_perros[12];
    InfoPerro pos_perros[12];
    InfoPuma info_puma = {p};

    p->tablero[4][2] = PUMA;

    int idx = 0;
    for (int j = 0; j < 5; j++) {
        p->tablero[2][j] = PERRO;
        pos_perros[idx] = (InfoPerro){idx + 1, 2, j, p};
        idx++;
    }
    for (int j = 0; j < 5; j++) {
        p->tablero[3][j] = PERRO;
        pos_perros[idx] = (InfoPerro){idx + 1, 3, j, p};
        idx++;
    }
    p->tablero[4][0] = PERRO;
    pos_perros[10] = (InfoPerro){11, 4, 0, p};
    p->tablero[4][4] = PERRO;
    pos_perros[11] = (InfoPerro){12, 4, 4, p};

    pthread_create(&hilo_puma, NULL, rutina_puma, &info_puma);
    for (int i = 0; i < 12; i++) {
        pthread_create(&hilos_perros[i], NULL, rutina_perro, &pos_perros[i]);
    }

    while (!p->juego_terminado) {
        usleep(400000);
        mostrar_tablero(p);
    }

    pthread_join(hilo_puma, NULL);
    for (int i = 0; i < 12; i++) {
        pthread_join(hilos_perros[i], NULL);
    }

    printf("<<< FIN PARTIDA %d en MESA %d >>>\n", p->id_partida, p->id_mesa);
    pthread_mutex_destroy(&p->mutex_tablero);

    sem_post(&sem_mesas_libres);

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

    int total_partidas = n / 2;
    if (total_partidas == 0) total_partidas = 1;

    printf("TORNEO INICIADO: %d Jugadores | %d Partidas totales | %d Mesas Simultaneas (M)\n\n", 
           n, total_partidas, m);

    sem_init(&sem_mesas_libres, 0, m);

    pthread_t hilos_jueces[total_partidas];
    Partida partidas[total_partidas];

    for (int i = 0; i < total_partidas; i++) {
        partidas[i].id_partida = i + 1;
        partidas[i].id_mesa = (i % m) + 1;
        pthread_create(&hilos_jueces[i], NULL, rutina_juez_mesa, &partidas[i]);
    }

    for (int i = 0; i < total_partidas; i++) {
        pthread_join(hilos_jueces[i], NULL);
    }

    sem_destroy(&sem_mesas_libres);

    printf("\nTorneo finalizado\n");

    return 0;
}
