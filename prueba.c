#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h> // Para gettimeofday

// Incluye la cabecera donde se declara zynk_hash_string.
#include "src/zynk.h"

// --- Configuración de la Tabla Hash ---
#define HASH_TABLE_SIZE (1 << 20) // 2^20 = 1,048,576 cubetas

typedef struct HashEntry {
    uint32_t hash_value;
    struct HashEntry *next;
} HashEntry;

HashEntry* hash_table[HASH_TABLE_SIZE];

size_t total_unique_hashes = 0;
size_t total_collisions = 0;

void init_hash_table() {
    for (size_t i = 0; i < HASH_TABLE_SIZE; ++i) {
        hash_table[i] = NULL;
    }
}

void free_hash_table() {
    for (size_t i = 0; i < HASH_TABLE_SIZE; ++i) {
        HashEntry *current = hash_table[i];
        while (current != NULL) {
            HashEntry *temp = current;
            current = current->next;
            free(temp);
        }
    }
}

bool add_hash_and_check_collision(uint32_t hash) {
    size_t bucket_index = hash & (HASH_TABLE_SIZE - 1);

    HashEntry *current = hash_table[bucket_index];
    while (current != NULL) {
        if (current->hash_value == hash) {
            total_collisions++;
            return true;
        }
        current = current->next;
    }

    HashEntry *new_entry = (HashEntry*)malloc(sizeof(HashEntry));
    if (new_entry == NULL) {
        fprintf(stderr, "Error: Fallo en la asignación de memoria para HashEntry.\n");
        exit(EXIT_FAILURE);
    }
    new_entry->hash_value = hash;
    new_entry->next = hash_table[bucket_index];
    hash_table[bucket_index] = new_entry;
    total_unique_hashes++;
    return false;
}

char* generate_random_string(int max_len) {
    int len = (rand() % max_len) + 1;
    char *str = (char*)malloc(len + 1);
    if (str == NULL) {
        fprintf(stderr, "Error: Fallo en la asignación de memoria para la cadena aleatoria.\n");
        exit(EXIT_FAILURE);
    }

    const char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{}|;':\",.<>/?`~";
    size_t charset_len = strlen(charset);

    for (int i = 0; i < len; ++i) {
        str[i] = charset[rand() % charset_len];
    }
    str[len] = '\0';
    return str;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <num_cadenas_a_generar> <longitud_maxima_cadena>\n", argv[0]);
        fprintf(stderr, "Ejemplo: %s 10000000 30\n", argv[0]);
        return EXIT_FAILURE;
    }

    unsigned long num_strings_to_generate = strtoul(argv[1], NULL, 10);
    int max_string_length = atoi(argv[2]);

    if (num_strings_to_generate == 0 || max_string_length <= 0) {
        fprintf(stderr, "Error: Los parámetros deben ser números positivos.\n");
        return EXIT_FAILURE;
    }

    srand((unsigned int)time(NULL));
    init_hash_table();

    printf("--- Iniciando prueba de colisiones de zynk_hash_string ---\n");
    printf("Número de cadenas a generar: %lu\n", num_strings_to_generate);
    printf("Longitud máxima de cadena: %d caracteres\n", max_string_length);
    printf("Tamaño de la tabla hash (cubetas): %u\n", HASH_TABLE_SIZE);

    unsigned long progress_interval = num_strings_to_generate / 100;
    if (progress_interval == 0) progress_interval = 1;

    struct timeval start, end;
    gettimeofday(&start, NULL); // Registrar tiempo de inicio

    for (unsigned long i = 0; i < num_strings_to_generate; ++i) {
        char *random_str = generate_random_string(max_string_length);
        uint32_t hash = zynk_hash_string(random_str);
        add_hash_and_check_collision(hash);
        free(random_str);

        if (i % progress_interval == 0 || i == num_strings_to_generate - 1) {
            float progress = (float)(i + 1) * 100.0f / num_strings_to_generate;
            printf("\rProgreso: %.2f%% | Colisiones: %zu", progress, total_collisions);
            fflush(stdout);
        }
    }

    gettimeofday(&end, NULL); // Registrar tiempo de fin

    // Calcular el tiempo transcurrido en segundos
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("\n--- Prueba finalizada ---\n");
    printf("Tiempo total de ejecución: %.2f segundos\n", elapsed_time);
    printf("Cadenas probadas: %lu\n", num_strings_to_generate);
    printf("Hashes únicos encontrados: %zu\n", total_unique_hashes);
    printf("Número total de colisiones: %zu\n", total_collisions);
    
    if (num_strings_to_generate > 0) {
        printf("Porcentaje de colisiones: %.2f%%\n", (float)total_collisions * 100.0f / num_strings_to_generate);
    } else {
        printf("Porcentaje de colisiones: 0.00%%\n");
    }

    free_hash_table();

    return EXIT_SUCCESS;
}
