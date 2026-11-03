# Torneo de Komikan Concurrente — Tarea 2, Sistemas Operativos

Simulación concurrente de un torneo de **Komikan** (juego tradicional mapuche: 1 Puma contra 12 Perros) donde todas las fichas se mueven en tiempo real, sin turnos, usando hilos POSIX. Un servidor empareja a los jugadores, limita el número de mesas activas y cada mesa tiene un juez que administra el tablero y detecta el fin de la partida.

**Integrantes:** Marcelo Navarrete · Rommel Bustamante

## Requisitos

- Sistema tipo Unix con `gcc` y `make` (probado en Ubuntu 24.04, gcc 13.3).
- Bibliotecas POSIX: `pthread` y semáforos `sem_t`.

> **Nota sobre macOS:** el programa usa semáforos sin nombre (`sem_init`), que macOS no implementa. Compilar y ejecutar en Linux (o WSL en Windows).

## Compilación

```bash
make            # genera el ejecutable ./torneo
make clean      # borra el ejecutable
```

Se compila con los flags exigidos por el enunciado: `gcc -Wall -Wextra -std=c17 -pthread`, sin warnings.

## Ejecución

```bash
./torneo N M [config.txt]
```

| Argumento    | Descripción                                                   |
|--------------|---------------------------------------------------------------|
| `N`          | Número total de jugadores. Se forman `N/2` partidas (pares Puma–Perros). |
| `M`          | Número máximo de mesas activas al mismo tiempo.              |
| `config.txt` | (Opcional) archivo de configuración.                          |

También se puede usar el Makefile:

```bash
make run              # equivale a ./torneo 4 2
make run N=10 M=3     # equivale a ./torneo 10 3
make run N=10 M=3 CONFIG=config.txt
```

### Archivo de configuración

Formato `clave valor`, uno por línea. Actualmente se lee la semilla aleatoria, para que las ejecuciones sean reproducibles:

```
seed 42
```

Si no se entrega archivo, la semilla se toma de `time(NULL)`.

### Interrupción con Ctrl+C

Al presionar **Ctrl+C** (`SIGINT`) el servidor deja de iniciar partidas nuevas, avisa a las partidas activas para que terminen, espera (`pthread_join`) a todos los hilos, destruye los mutex y el semáforo de mesas y termina con `torneo finalizado`.

## Representación del tablero

El tablero es una matriz de 7×5: las filas 0–1 forman el trono (triángulo) y las filas 2–6 la cuadrícula de 5×5. Las celdas fuera del trono se marcan como inválidas.

```
       .            <- trono
    .  .  .
 o  o  o  o  o
 o  o  o  o  o
 o  .  x  .  o      x = Puma, o = Perro, . = casilla libre
 .  .  .  .  .
 .  .  .  .  .
```

## Diseño de concurrencia (resumen)

| Elemento | Implementación |
|---|---|
| Fichas | 1 hilo para el Puma y 12 hilos para los Perros por partida, con retardo aleatorio de 50–300 ms entre intentos de movimiento. |
| Tablero | Cada partida tiene su propio tablero y su propio `pthread_mutex_t`. Verificar si la casilla destino está libre y moverse ocurre dentro de la misma sección crítica, así que si dos fichas apuntan a la misma casilla solo una la obtiene. |
| Mesas | Semáforo contador `sem_mesas_libres` inicializado en `M`. Cada juez debe tomar un permiso antes de iniciar su partida y lo devuelve al terminar. |
| Juez | Un hilo por partida: inicializa el tablero, lanza las fichas, imprime el tablero cada 400 ms, espera el fin del juego y libera la mesa. |
| Victoria | El Puma gana al capturar 6 perros; los Perros ganan si el Puma queda sin movimientos válidos. |
| SIGINT | El manejador solo cambia una bandera `volatile sig_atomic_t`; todos los hilos la revisan en sus ciclos y terminan de forma ordenada. |

El análisis detallado (deadlock, livelock, starvation y evaluación del diseño) está en el informe PDF de la entrega.

## Herramientas de verificación

```bash
make valgrind N=2 M=1   # revisa fugas de memoria con valgrind
make tsan               # compila con ThreadSanitizer para detectar carreras de datos
```
