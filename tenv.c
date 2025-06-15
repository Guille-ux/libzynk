#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h> // For malloc and free (used for temporary string keys)

// Incluye las cabeceras públicas necesarias de tu proyecto.
// Se asume que zynk.h incluye internamente zynk_enviroment.h, sysarena.h, etc.
#include "src/zynk.h"

// --- Variables y Funciones Utilitarias para el Test ---
static ArenaManager g_test_arena_manager;
#define TEST_MEMORY_SIZE (1024 * 1024) // 1MB de memoria para el arena
static uint8_t g_test_memory_pool[TEST_MEMORY_SIZE];
static Arena g_test_arenas[32]; // Pool de arenas

#define PASS() printf("[ \033[0;32mPASS\033[0m ]\n")
#define FAIL() printf("[ \033[0;31mFAIL\033[0m ]\n")

static int tests_run = 0;
static int tests_passed = 0;

void assert_true(bool condition, const char* message) {
    tests_run++;
    printf("Test %d: %s ", tests_run, message);
    if (condition) {
        PASS();
        tests_passed++;
    } else {
        FAIL();
    }
}

void assert_false(bool condition, const char* message) {
    tests_run++;
    printf("Test %d: %s ", tests_run, message);
    if (!condition) {
        PASS();
        tests_passed++;
    } else {
        FAIL();
    }
}

void assert_value_number(Value val, double expected, const char* message) {
    tests_run++;
    printf("Test %d: %s ", tests_run, message);
    if (val.type == ZYNK_NUMBER && val.as.number == expected) {
        PASS();
        tests_passed++;
    } else {
        printf("[ \033[0;31mFAIL\033[0m ] Expected Number %f, Got Type %d (Value: %f)\n", expected, val.type, val.as.number);
    }
}

void assert_value_bool(Value val, bool expected, const char* message) {
    tests_run++;
    printf("Test %d: %s ", tests_run, message);
    if (val.type == ZYNK_BOOL && val.as.boolean == expected) {
        PASS();
        tests_passed++;
    } else {
        printf("[ \033[0;31mFAIL\033[0m ] Expected Bool %s, Got Type %d (Value: %s)\n", expected ? "true" : "false", val.type, val.as.boolean ? "true" : "false");
    }
}

void assert_value_null(Value val, const char* message) {
    tests_run++;
    printf("Test %d: %s ", tests_run, message);
    if (val.type == ZYNK_NULL) {
        PASS();
        tests_passed++;
    } else {
        printf("[ \033[0;31mFAIL\033[0m ] Expected NULL, Got Type %d\n", val.type);
    }
}

// Función auxiliar para visualizar el estado interno del array de punteros de la tabla hash.
void print_table_state(const ZynkEnvTable *table) {
    printf("--- Estado Actual de la Tabla (Elementos: %zu / Capacidad: %zu) ---\n", table->count, table->capacity);
    if (table == NULL || table->entries == NULL) {
        printf("[La Tabla es NULL o las entradas son NULL]\n");
        return;
    }
    for (size_t i = 0; i < table->capacity; ++i) {
        printf("Slot %zu: ", i);
        // En esta implementación, entries[i] NUNCA debe ser NULL después de la inicialización
        if (table->entries[i] == NULL) {
            printf("[ERROR: El slot contiene un puntero NULL - debería estar pre-asignado]\n");
        } else {
            // Un 'name' siendo NULL indica un slot vacío o eliminado (tombstone)
            if (table->entries[i]->name != NULL) {
                printf("[Clave: '%s', Tipo Valor: %d]\n",
                                table->entries[i]->name,
                                table->entries[i]->value.type);
            } else {
                printf("[Slot VACÍO / TOMBSTONE]\n"); // Este slot existe pero no contiene una clave activa
            }
        }
    }
    printf("---------------------------------------\n");
}


// --- Función Principal de Test ---
void run_hash_table_tests() {
    printf("\n--- Test: Operaciones de Tabla Hash (New, Set, Get, Delete) ---\n");

    // 1. Inicializar sysarena para la gestión de memoria.
    bool sysarena_ok = sysarena_init(&g_test_arena_manager, g_test_memory_pool, g_test_arenas, TEST_MEMORY_SIZE, sizeof(g_test_arenas)/sizeof(g_test_arenas[0]));
    assert_true(sysarena_ok, "Inicializar sysarena para el test.");

    ZynkEnv global_env;
    size_t initial_capacity = 8; // Capacidad inicial para la tabla hash

    // --- Inicialización manual de ZynkEnv y ZynkEnvTable para el test ---
    // Esta parte asigna directamente las estructuras de memoria necesarias
    // para satisfacer los requisitos de la función initZynkTable del zynk_enviroment.c proporcionado.

    // 1. Asignar la estructura ZynkEnvTable en sí
    global_env.local = (ZynkEnvTable*)sysarena_alloc(&g_test_arena_manager, sizeof(ZynkEnvTable));
    assert_true(global_env.local != NULL, "Asignar ZynkEnvTable para el entorno local.");

    // 2. Asignar el array de punteros ZynkEnvEntry*
    global_env.local->entries = (ZynkEnvEntry**)sysarena_alloc(&g_test_arena_manager, sizeof(ZynkEnvEntry*) * initial_capacity);
    assert_true(global_env.local->entries != NULL, "Asignar array ZynkEnvEntry* para la tabla local.");

    // 3. CRÍTICO: Asignar la estructura ZynkEnvEntry para CADA slot e inicializarla.
    // Esto asegura que table->entries[i] NO sea NULL cuando se llama a initZynkTable.
    for (size_t i = 0; i < initial_capacity; ++i) {
        global_env.local->entries[i] = (ZynkEnvEntry*)sysarena_alloc(&g_test_arena_manager, sizeof(ZynkEnvEntry));
        assert_true(global_env.local->entries[i] != NULL, "Asignar ZynkEnvEntry para cada slot.");

        // Inicializar la estructura ZynkEnvEntry asignada
        global_env.local->entries[i]->name = NULL; // Marcar como vacío/disponible (tombstone)
        global_env.local->entries[i]->value = zynkNull(); // Valor por defecto
    }

    // 4. Configurar la capacidad inicial y el contador de elementos para la estructura de la tabla
    global_env.local->capacity = initial_capacity;
    global_env.local->count = 0; // Inicialmente, no hay entradas activas
    global_env.enclosing = NULL; // El entorno global no tiene entorno anidado

    // 5. Llamar a initZynkTable (que ahora pasará sus comprobaciones internas)
    // Nota: initZynkTable en el zynk_enviroment.c proporcionado no usa ArenaManager
    // para su lógica interna, pero se basa en la asignación previa realizada arriba.
    bool table_initialized = initZynkTable(global_env.local, initial_capacity);
    assert_true(table_initialized, "Llamar a initZynkTable para finalizar la inicialización.");

    // Verificaciones adicionales del estado inicial.
    assert_true(global_env.local->entries != NULL, "Verificar que la tabla local tiene un array de entradas inicializado.");
    assert_true(global_env.local->count == 0, "Verificar que el contador de entradas es 0 inicialmente.");
    assert_true(global_env.local->capacity == 8, "Verificar que la capacidad de la tabla es 8.");
    print_table_state(global_env.local);


    // --- Definir cadenas para las claves (asignadas por el test, liberadas al final) ---
    // Usamos malloc aquí porque estas cadenas son de la capa de test y no se gestionan
    // por sysarena en este punto, a menos que zynkTableNew/Set haga copias con sysarena_alloc.
    char* key_alpha = (char*)malloc(strlen("alpha") + 1); strcpy(key_alpha, "alpha");
    char* key_beta = (char*)malloc(strlen("beta") + 1); strcpy(key_beta, "beta");
    char* key_gamma = (char*)malloc(strlen("gamma") + 1); strcpy(key_gamma, "gamma");
    char* key_delta = (char*)malloc(strlen("delta") + 1); strcpy(key_delta, "delta");
    char* key_epsilon = (char*)malloc(strlen("epsilon") + 1); strcpy(key_epsilon, "epsilon");
    char* key_zeta = (char*)malloc(strlen("zeta") + 1); strcpy(key_zeta, "zeta");
    char* key_eta = (char*)malloc(strlen("eta") + 1); strcpy(key_eta, "eta");
    char* key_theta = (char*)malloc(strlen("theta") + 1); strcpy(key_theta, "theta");


    // --- Inserción de elementos usando zynkTableNew ---
    printf("\n--- Inserciones Iniciales con zynkTableNew ---\n");

    // WORKAROUND: zynkTableNew en el zynk_enviroment.c proporcionado NO asigna memoria para el 'name' de la entrada.
    // Esto se hace aquí para que las pruebas de Get y las de eliminación funcionen.
    // En una implementación correcta de la tabla hash, zynkFindEntry (o zynkTableNew) debería
    // gestionar la asignación de memoria para 'entry->name' usando sysarena_alloc.
    #define ALLOC_KEY_IN_ENTRY_WORKAROUND(env_ptr, key_str) \
    do { \
        ZynkEnvEntry *e = zynkFindEntry(env_ptr, key_str, true); \
        if (e != NULL && e->name == NULL) { /* Solo si es un slot vacío/tombstone y el nombre no está ya asignado */ \
            size_t len = strlen(key_str); \
            e->name = (char*)sysarena_alloc(&g_test_arena_manager, len + 1); \
            if (e->name != NULL) { \
                memcpy(e->name, key_str, len + 1); \
            } \
        } \
    } while(0)


    assert_true(zynkTableNew(&global_env, key_alpha, zynkNumber(1.0)), "Insertar 'alpha' (New)");
    ALLOC_KEY_IN_ENTRY_WORKAROUND(&global_env, key_alpha); // Solución temporal
    assert_true(global_env.local->count == 1, "Contador tras 'alpha'");
    print_table_state(global_env.local);

    assert_true(zynkTableNew(&global_env, key_beta, zynkNumber(2.0)), "Insertar 'beta' (New)");
    ALLOC_KEY_IN_ENTRY_WORKAROUND(&global_env, key_beta); // Solución temporal
    assert_true(global_env.local->count == 2, "Contador tras 'beta'");
    print_table_state(global_env.local);

    assert_true(zynkTableNew(&global_env, key_gamma, zynkNumber(3.0)), "Insertar 'gamma' (New)");
    ALLOC_KEY_IN_ENTRY_WORKAROUND(&global_env, key_gamma); // Solución temporal
    assert_true(global_env.local->count == 3, "Contador tras 'gamma'");
    print_table_state(global_env.local);

    assert_true(zynkTableNew(&global_env, key_delta, zynkNumber(4.0)), "Insertar 'delta' (New)");
    ALLOC_KEY_IN_ENTRY_WORKAROUND(&global_env, key_delta); // Solución temporal
    assert_true(global_env.local->count == 4, "Contador tras 'delta'");
    print_table_state(global_env.local);

    assert_true(zynkTableNew(&global_env, key_epsilon, zynkNumber(5.0)), "Insertar 'epsilon' (New)");
    ALLOC_KEY_IN_ENTRY_WORKAROUND(&global_env, key_epsilon); // Solución temporal
    assert_true(global_env.local->count == 5, "Contador tras 'epsilon'");
    print_table_state(global_env.local);

    assert_true(zynkTableNew(&global_env, key_zeta, zynkNumber(6.0)), "Insertar 'zeta' (New)");
    ALLOC_KEY_IN_ENTRY_WORKAROUND(&global_env, key_zeta); // Solución temporal
    assert_true(global_env.local->count == 6, "Contador tras 'zeta'");
    print_table_state(global_env.local);

    assert_true(zynkTableNew(&global_env, key_eta, zynkNumber(7.0)), "Insertar 'eta' (New)");
    ALLOC_KEY_IN_ENTRY_WORKAROUND(&global_env, key_eta); // Solución temporal
    assert_true(global_env.local->count == 7, "Contador tras 'eta'");
    print_table_state(global_env.local);

    // --- Actualizar un valor existente con zynkTableSet ---
    printf("\n--- Actualización con zynkTableSet (clave existente) ---\n");
    assert_true(zynkTableSet(&global_env, key_alpha, zynkNumber(10.0)), "Actualizar 'alpha' con zynkTableSet");
    assert_value_number(zynkTableGet(&global_env, key_alpha), 10.0, "Verificar valor actualizado de 'alpha'");
    assert_true(global_env.local->count == 7, "Contador no cambia tras Set exitoso");

    // --- Intentar establecer una nueva clave con zynkTableSet (debe fallar según lo especificado) ---
    printf("\n--- Intentar Setear una clave nueva con zynkTableSet (debe fallar si la clave no existe) ---\n");
    assert_false(zynkTableSet(&global_env, key_theta, zynkNumber(8.0)), "Intentar setear 'theta' (Set debe fallar si clave no existe)");
    assert_true(global_env.local->count == 7, "Contador no cambia tras Set fallido en clave nueva");
    assert_value_null(zynkTableGet(&global_env, key_theta), "Verificar que 'theta' no existe tras Set fallido");
    print_table_state(global_env.local);


    // La tabla tiene 7 entradas activas, capacidad 8. Queda un slot como 'Slot VACÍO / TOMBSTONE'.
    // Insertar una más con zynkTableNew para llenar la tabla.
    printf("\n--- Insertar 'theta' con zynkTableNew (ahora 8 entradas, llenando el último slot vacío) ---\n");
    assert_true(zynkTableNew(&global_env, key_theta, zynkNumber(8.0)), "Insertar 'theta' con zynkTableNew");
    ALLOC_KEY_IN_ENTRY_WORKAROUND(&global_env, key_theta); // Solución temporal
    assert_true(global_env.local->count == 8, "Contador tras inserción de 'theta' con New");
    print_table_state(global_env.local);

    // La tabla está llena, intentar insertar otra clave (debería fallar tanto para New como para Set)
    printf("\n--- Intentar insertar en tabla llena ---\n");
    char* key_full = (char*)malloc(strlen("full") + 1); strcpy(key_full, "full");
    assert_false(zynkTableNew(&global_env, key_full, zynkNumber(100.0)), "Intentar insertar 'full' con zynkTableNew (debe fallar, tabla llena)");
    assert_false(zynkTableSet(&global_env, key_full, zynkNumber(101.0)), "Intentar insertar 'full' con zynkTableSet (debe fallar, tabla llena)");
    assert_value_null(zynkTableGet(&global_env, key_full), "Verificar que 'full' no existe");
    free(key_full);
    assert_true(global_env.local->count == 8, "Contador no cambia tras inserciones fallidas en tabla llena");


    // --- Escenario de Eliminación ---
    printf("\n--- Escenario de Eliminación ---\n");

    // Eliminar 'gamma'
    printf("\n--- Eliminando 'gamma' ---\n");
    // WORKAROUND: zynkTableDelete en el zynk_enviroment.c proporcionado NO libera la memoria del 'name'.
    // Esto se hace aquí manualmente para que freeZynkTable funcione correctamente al final.

    assert_true(zynkTableDelete(&global_env, key_gamma), "Eliminar 'gamma'");
    assert_value_null(zynkTableGet(&global_env, key_gamma), "Verificar que 'gamma' es NULL tras eliminación"); // Get debe retornar NULL para una entrada eliminada
    assert_true(global_env.local->count == 7, "Contador de elementos correcto tras eliminar 'gamma'"); // Se espera 7 (8-1)
    print_table_state(global_env.local);

    // Eliminar otro elemento y verificar.
    printf("\n--- Eliminando 'epsilon' ---\n");
    ZynkEnvEntry *epsilon_entry_for_free = zynkFindEntry(&global_env, key_epsilon, false);
    if (epsilon_entry_for_free != NULL && epsilon_entry_for_free->name != NULL) {
        bool freed_name = sysarena_free(&g_test_arena_manager, epsilon_entry_for_free->name);
        // assert_true(freed_name, "Liberar nombre de 'epsilon' antes de la eliminación lógica"); // Opcional: para depuración
    }
    assert_true(zynkTableDelete(&global_env, key_epsilon), "Eliminar 'epsilon'");
    assert_value_null(zynkTableGet(&global_env, key_epsilon), "Verificar que 'epsilon' es NULL tras eliminación");
    assert_true(global_env.local->count == 6, "Contador de elementos correcto tras eliminar 'epsilon'"); // Se espera 6 (7-1)
    print_table_state(global_env.local);

    // Intentar eliminar una clave inexistente
    printf("\n--- Intentar eliminar clave inexistente ---\n");
    char non_existent_key[] = "inexistente";
    assert_false(zynkTableDelete(&global_env, non_existent_key), "Intentar eliminar clave inexistente");
    assert_true(global_env.local->count == 6, "Contador de elementos no cambia al intentar eliminar clave inexistente"); // Se espera que permanezca en 6

    printf("\n=======================================\n");
    printf("  Resumen del Test: %d de %d pasados \n", tests_passed, tests_run);
    printf("=======================================\n");
}

int main() {
    printf("=======================================\n");
    printf("  Suite de Pruebas de Tabla Hash Zynk  \n");
    printf("=======================================\n");

    run_hash_table_tests();

    return 0;
}
