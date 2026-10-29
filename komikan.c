#include <stdio.h>
#include <stdlib.h>

#define VACIO 0
#define PUMA 1
#define PERRO 2
#define INVALIDO -1

// tablero 7x5 para incluir el trono arriba
int tablero[7][5];

void inicializar_tablero() {
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 5; j++) {
            tablero[i][j] = VACIO;
        }
    }
    
    // anular casillas q no son del tablero (puntas del triangulo)
    tablero[0][0] = INVALIDO;
    tablero[0][1] = INVALIDO;
    tablero[0][3] = INVALIDO;
    tablero[0][4] = INVALIDO;
    
    tablero[1][0] = INVALIDO;
    tablero[1][4] = INVALIDO;

    printf("tablero ok\n");
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

    return 0;
}
