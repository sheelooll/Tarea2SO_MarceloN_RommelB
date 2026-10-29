#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("uso: %s n m [config.txt]\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);

    printf("torneo iniciado con %d jugadores y %d mesas\n", n, m);

    if (argc >= 4) {
        char *config = argv[3];
        printf("usando config: %s\n", config);
    } else {
        printf("sin config inicial\n");
    }

    return 0;
}
