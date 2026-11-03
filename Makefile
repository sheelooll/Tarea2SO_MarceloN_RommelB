# Makefile - Torneo de Komikan Concurrente (Tarea 2, Sistemas Operativos)

CC      := gcc
CFLAGS  := -Wall -Wextra -std=c17 -pthread
LDFLAGS := -pthread

TARGET  := torneo
SRC     := komikan.c

# Parametros por defecto para "make run" (se pueden cambiar: make run N=10 M=3)
N       ?= 4
M       ?= 2
CONFIG  ?=

.PHONY: all run debug valgrind tsan clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) $(N) $(M) $(CONFIG)

# Compilacion con simbolos de depuracion
debug: CFLAGS += -g -O0
debug: clean $(TARGET)

# Verificacion de fugas de memoria
valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) $(N) $(M) $(CONFIG)

# Deteccion de condiciones de carrera con ThreadSanitizer
tsan: clean
	$(CC) $(CFLAGS) -g -fsanitize=thread -o $(TARGET) $(SRC) $(LDFLAGS) -fsanitize=thread

clean:
	rm -f $(TARGET) *.o
