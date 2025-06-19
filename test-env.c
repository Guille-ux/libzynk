#include <stdio.h>
#include <stdbool.h>
#include <string.h> 

// Incluye el archivo de cabecera principal de Zynk que debería incluir todo lo demás.
#include "src/zynk.h" 

// --- Definiciones globales para el sistema sysarena ---
// Tamaño total de la memoria que sysarena gestionará
#define TEST_MEMORY_SIZE (1024 * 64) 
// Número máximo de "arenas" (bloques de memoria) que sysarena puede gestionar
// MODIFICADO: Aumentado el número máximo de arenas para acomodar las entradas de la tabla.
#define MAX_ARENAS 10000

// Buffer de memoria global para sysarena
uint8_t global_memory_buffer[TEST_MEMORY_SIZE];
// Array de estructuras Arena para que sysarena las gestione
Arena global_arenas[MAX_ARENAS];

// --- Funciones de ayuda para las aserciones de prueba ---

void assert_true(bool condition, const char* message) {
    if (condition) {
        printf("[EXITO] %s\n", message);
    } else {
        printf("[FALLO] %s\n", message);
    }
}

void assert_equal_number(Value val, double expected, const char* message) {
    if (val.type == ZYNK_NUMBER && val.as.number == expected) {
        printf("[EXITO] %s (Valor: %.2f)\n", message, val.as.number);
    } else {
        printf("[FALLO] %s (Esperado: %.2f, Tipo Obtenido: %d, Valor Obtenido: %.2f)\n", 
               message, expected, val.type, val.as.number);
    }
}

void assert_equal_bool(Value val, bool expected, const char* message) {
    if (val.type == ZYNK_BOOL && val.as.boolean == expected) {
        printf("[EXITO] %s (Valor: %s)\n", message, expected ? "true" : "false");
    } else {
        printf("[FALLO] %s (Esperado: %s, Tipo Obtenido: %d, Valor Obtenido: %s)\n", 
               message, expected ? "true" : "false", val.type, val.as.boolean ? "true" : "false");
    }
}

void assert_is_null(Value val, const char* message) {
    if (val.type == ZYNK_NULL) {
        printf("[EXITO] %s\n", message);
    } else {
        printf("[FALLO] %s (Esperado: NULL, Tipo Obtenido: %d)\n", message, val.type);
    }
}

// --- Función principal de pruebas ---
int main() {
    printf("--- Pruebas de Entorno Zynk y Gestión de Memoria ---\n");

    ArenaManager manager;
    ZynkEnv env;

    // 1. Inicializar el Arena Manager
    printf("\n--- Prueba: sysarena_init ---\n");
    bool sysarena_ok = sysarena_init(&manager, global_memory_buffer, global_arenas, TEST_MEMORY_SIZE, MAX_ARENAS);
    assert_true(sysarena_ok, "sysarena_init: Manager inicializado con exito.");
    if (!sysarena_ok) {
        printf("Fatal: Fallo al inicializar sysarena manager. Saliendo.\n");
        return 1;
    }

    // 2. Asignar env.local (ZynkEnvTable) y su array de entradas (ZynkEnvEntry**)
    // Las entradas individuales (ZynkEnvEntry*) serán asignadas por initZynkTable.
    env.local = (ZynkEnvTable*)sysarena_alloc(&manager, sizeof(ZynkEnvTable));
    if (env.local == NULL) {
        printf("Fatal: Fallo al asignar memoria para env->local (ZynkEnvTable). Saliendo.\n");
        return 1;
    }
    
    env.local->entries = (ZynkEnvEntry**)sysarena_alloc(&manager, TABLE_CAPACITY * sizeof(ZynkEnvEntry*));
    if (env.local->entries == NULL) {
        printf("Fatal: Fallo al asignar memoria para env->local->entries (array de punteros). Saliendo.\n");
        return 1;
    }

    // Inicializar los punteros del array a NULL, para que initZynkTable sepa que debe asignarlos.
    for (size_t i = 0; i < TABLE_CAPACITY; i++) {
        env.local->entries[i] = NULL;
    }

    // 3. Inicializar el Entorno Zynk
    printf("\n--- Prueba: zynkEnvInit ---\n");
    bool env_ok = zynkEnvInit(&env, TABLE_CAPACITY, NULL, &manager); 
    assert_true(env_ok, "zynkEnvInit: Entorno inicializado con exito.");
    if (!env_ok) {
        printf("Fatal: Fallo al inicializar el entorno Zynk. Saliendo.\n");
        return 1;
    }
    assert_true(env.local != NULL, "zynkEnvInit: env->local asignado correctamente (por test).");
    assert_true(env.local->entries != NULL, "zynkEnvInit: env->local->entries asignado correctamente (por test).");
    assert_true(env.local->count == 0, "zynkEnvInit: Contador inicial es 0.");
    assert_true(env.local->capacity == TABLE_CAPACITY, "zynkEnvInit: Capacidad coincide con TABLE_CAPACITY.");


    // 4. Prueba zynkTableNew (Añadir nuevas entradas)
    printf("\n--- Prueba: zynkTableNew (añadir) ---\n");
    bool new_ok_1 = zynkTableNew(&env, "miNumero", zynkNumber(123.45), &manager); 
    assert_true(new_ok_1, "zynkTableNew: Agregado 'miNumero' con exito.");
    assert_true(env.local->count == 1, "zynkTableNew: Contador incrementado a 1.");

    bool new_ok_2 = zynkTableNew(&env, "miBooleano", zynkBool(true), &manager); 
    assert_true(new_ok_2, "zynkTableNew: Agregado 'miBooleano' con exito.");
    assert_true(env.local->count == 2, "zynkTableNew: Contador incrementado a 2.");

    bool new_ok_3 = zynkTableNew(&env, "otroNumero", zynkNumber(99.0), &manager); 
    assert_true(new_ok_3, "zynkTableNew: Agregado 'otroNumero' con exito.");
    assert_true(env.local->count == 3, "zynkTableNew: Contador incrementado a 3.");

    // Intentar añadir una entrada existente con zynkTableNew.
    bool new_ok_4 = zynkTableNew(&env, "miNumero", zynkNumber(543.21), &manager); 
    assert_true(!new_ok_4, "zynkTableNew: Fallo al re-añadir 'miNumero' (esperado si es solo para nuevas inserciones).");
    assert_true(env.local->count == 3, "zynkTableNew: Contador se mantiene en 3 despues de fallo.");


    // 5. Prueba zynkTableGet (Recuperar entradas)
    printf("\n--- Prueba: zynkTableGet ---\n");
    Value get_val_1 = zynkTableGet(&env, "miNumero");
    assert_equal_number(get_val_1, 123.45, "zynkTableGet: Recuperado 'miNumero' correctamente.");

    Value get_val_2 = zynkTableGet(&env, "miBooleano");
    assert_equal_bool(get_val_2, true, "zynkTableGet: Recuperado 'miBooleano' correctamente.");

    Value get_val_3 = zynkTableGet(&env, "noExiste");
    assert_is_null(get_val_3, "zynkTableGet: Recuperado 'noExiste' como NULL (esperado).");

    // 6. Prueba zynkTableSet (Actualizar entradas existentes)
    printf("\n--- Prueba: zynkTableSet (actualizar) ---\n");
    bool set_ok_1 = zynkTableSet(&env, "miNumero", zynkNumber(789.0)); 
    assert_true(set_ok_1, "zynkTableSet: Actualizado 'miNumero' con exito.");
    Value updated_val_1 = zynkTableGet(&env, "miNumero");
    assert_equal_number(updated_val_1, 789.0, "zynkTableSet: Valor de 'miNumero' actualizado correctamente.");
    assert_true(env.local->count == 3, "zynkTableSet: Contador se mantiene en 3 despues de la actualizacion.");

    bool set_ok_2 = zynkTableSet(&env, "noExiste", zynkNumber(100.0)); 
    assert_true(!set_ok_2, "zynkTableSet: Fallo al setear 'noExiste' (esperado, ya que no existe).");


    // 7. Prueba zynkTableDelete (Borrar entradas)
    printf("\n--- Prueba: zynkTableDelete ---\n");
    bool delete_ok_1 = zynkTableDelete(&env, "miBooleano", &manager); 
    assert_true(delete_ok_1, "zynkTableDelete: Borrado 'miBooleano' con exito.");
    assert_true(env.local->count == 2, "zynkTableDelete: Contador decrementado a 2.");
    Value deleted_val_1 = zynkTableGet(&env, "miBooleano");
    assert_is_null(deleted_val_1, "zynkTableDelete: 'miBooleano' es ahora NULL despues del borrado.");

    bool delete_ok_2 = zynkTableDelete(&env, "noExiste", &manager); 
    assert_true(!delete_ok_2, "zynkTableDelete: Fallo al borrar 'noExiste' (esperado).");
    assert_true(env.local->count == 2, "zynkTableDelete: Contador se mantiene en 2 despues de fallo.");

    // 8. Prueba freeZynkTable y sysarena_defragment (Limpieza)
    printf("\n--- Prueba: freeZynkTable y sysarena_defragment ---\n");
    // Ahora freeZynkTable se encarga de liberar la mayoría de la memoria asignada por el entorno.
    bool free_ok = freeZynkTable(&manager, env.local);
    assert_true(free_ok, "freeZynkTable: Liberacion de tabla exitosa.");

    // Finalmente, libera la propia estructura ZynkEnvTable que test-env.c asignó.
    sysarena_free(&manager, env.local);

    printf("[INFO] sysarena_defragment llamado. Para una verificacion completa, se requiere inspeccion manual o una funcion `sysarena_is_fully_merged` mas detallada.\n");
    sysarena_defragment(&manager);

    printf("\n--- Todas las pruebas completadas ---\n");

    return 0;
}
