#include <stdio.h> // Para printf
#include <stdbool.h> // Para bool
#include <stdlib.h> // Para exit, si quieres salir en errores graves

#include "src/zynk.h"

// --- Pequeña función auxiliar para visualizar el ref_count ---
void print_ref_count(const char* name, Value val) {
    if (val.type == ZYNK_OBJ && val.as.obj != NULL) {
        printf("DEBUG: %s -> ref_count: %u\n", name, val.as.obj->ref_count);
    } else if (val.type == ZYNK_NULL) {
        printf("DEBUG: %s -> Es NULL\n", name);
    } else {
        printf("DEBUG: %s -> No es un objeto ZYNK_OBJ (Tipo: %d)\n", name, val.type);
    }
}

// --- MAIN DEL SISTEMA DE PRUEBAS ---
int main() {
    printf("--- Iniciando pruebas de Conteo de Referencias ---\n\n");

    // 1. Inicializar el ArenaManager
    ArenaManager manager;
    if (!init_sysarena(&manager, 1024 * 1024)) { // 1MB de memoria para la arena
        fprintf(stderr, "ERROR: No se pudo inicializar ArenaManager.\n");
        return 1;
    }
    printf("ArenaManager inicializado.\n\n");

    // --- ESCENARIO 1: Creación y Liberación Básica de String ---
    printf("--- ESCENARIO 1: String Básico ---\n");
    Value str1 = zynkStringCreate(&manager, "Hola Zynk!");
    print_ref_count("str1 (inicial)", str1); // Esperado: ref_count = 1

    zynk_release(str1, &manager);
    printf("str1 liberado una vez.\n");
    // En este punto, si todo funciona, str1 debería haberse liberado si su ref_count llega a 0.
    // Intentar acceder a str1.as.obj->ref_count aquí podría causar un fallo, lo cual sería bueno!
    // Para depurar, puedes poner un breakpoint en zynk_release cuando ref_count llega a 0.

    printf("\n");

    // --- ESCENARIO 2: Múltiples Referencias a un String ---
    printf("--- ESCENARIO 2: Múltiples Referencias de String ---\n");
    Value mainStr = zynkStringCreate(&manager, "Compartiendo memoria!");
    print_ref_count("mainStr (inicial)", mainStr); // Esperado: ref_count = 1

    Value ref1 = zynk_retain(mainStr); // Nueva referencia
    print_ref_count("mainStr (después de ref1)", mainStr); // Esperado: ref_count = 2

    Value ref2 = zynk_retain(mainStr); // Otra nueva referencia
    print_ref_count("mainStr (después de ref2)", mainStr); // Esperado: ref_count = 3

    printf("Liberando ref1...\n");
    zynk_release(ref1, &manager);
    print_ref_count("mainStr (después de liberar ref1)", mainStr); // Esperado: ref_count = 2

    printf("Liberando ref2...\n");
    zynk_release(ref2, &manager);
    print_ref_count("mainStr (después de liberar ref2)", mainStr); // Esperado: ref_count = 1

    printf("Liberando mainStr (final)...\n");
    zynk_release(mainStr, &manager);
    print_ref_count("mainStr (después de liberar mainStr)", mainStr); // Debería indicar que ya no es un objeto válido si se liberó.

    printf("\n");

    // --- ESCENARIO 3: Array con Objetos Anidados ---
    printf("--- ESCENARIO 3: Array con Objetos Anidados ---\n");

    Value arrayStr1 = zynkStringCreate(&manager, "Elemento Uno");
    Value arrayStr2 = zynkStringCreate(&manager, "Elemento Dos");
    Value arrayStr3 = zynkStringCreate(&manager, "Elemento Tres");

    print_ref_count("arrayStr1 (inicial)", arrayStr1); // Esperado: 1
    print_ref_count("arrayStr2 (inicial)", arrayStr2); // Esperado: 1
    print_ref_count("arrayStr3 (inicial)", arrayStr3); // Esperado: 1

    Value zynkArray = zynkArrayCreate(&manager, 5);
    print_ref_count("zynkArray (inicial)", zynkArray); // Esperado: 1

    // Para añadir elementos a un array (si aún no tienes una función de "push" o "set"):
    // Necesitas asegurarte de que el array tenga capacidad y actualizar su 'len'
    // ¡Y lo más importante, debes retain-ear el objeto que pones en el array!
    if (zynkArray.type == ZYNK_OBJ && zynkArray.as.obj != NULL && zynkArray.as.obj->type == ObjArray) {
        ZynkArray* arrPtr = zynkArray.as.obj->as.array; // Acceso correcto a la unión 'as'
        if (arrPtr->len < arrPtr->capacity) {
            arrPtr->array[arrPtr->len++] = zynk_retain(arrayStr1); // Agrega y retiene
            arrPtr->array[arrPtr->len++] = zynk_retain(arrayStr2); // Agrega y retiene
            arrPtr->array[arrPtr->len++] = zynk_retain(arrayStr3); // Agrega y retiene
            printf("Elementos añadidos al array.\n");
        }
    }
    
    print_ref_count("arrayStr1 (después de añadir al array)", arrayStr1); // Esperado: 2
    print_ref_count("arrayStr2 (después de añadir al array)", arrayStr2); // Esperado: 2
    print_ref_count("arrayStr3 (después de añadir al array)", arrayStr3); // Esperado: 2

    printf("Liberando zynkArray...\n");
    zynk_release(zynkArray, &manager); // Esto debería liberar arrayStr1, arrayStr2, arrayStr3 también al final

    print_ref_count("arrayStr1 (después de liberar zynkArray)", arrayStr1); // Esperado: ya no es un objeto válido si se liberó
    print_ref_count("arrayStr2 (después de liberar zynkArray)", arrayStr2); // Esperado: ya no es un objeto válido si se liberó
    print_ref_count("arrayStr3 (después de liberar zynkArray)", arrayStr3); // Esperado: ya no es un objeto válido si se liberó

    printf("\n");

    // --- ESCENARIO 4: Reasignación (simulada) ---
    printf("--- ESCENARIO 4: Reasignación Simulada ---\n");
    Value varA = zynkStringCreate(&manager, "Primer Valor");
    print_ref_count("varA (inicial)", varA); // Esperado: 1

    Value varB = zynk_retain(varA); // varB ahora también apunta a "Primer Valor"
    print_ref_count("varA (después de varB)", varA); // Esperado: 2
    print_ref_count("varB (después de asignación)", varB); // Esperado: 2

    printf("Reasignando varA a un nuevo string...\n");
    Value oldVarA = varA; // Guarda la referencia antigua para liberarla.
    varA = zynkStringCreate(&manager, "Segundo Valor"); // Crea un nuevo objeto para varA
    zynk_release(oldVarA, &manager); // Libera la referencia antigua de varA
    
    print_ref_count("varA (nuevo valor)", varA); // Esperado: 1 (para el nuevo string)
    print_ref_count("varB (aún apuntando al 1er valor)", varB); // Esperado: 1 (el viejo string)
    // El "Primer Valor" todavía debería existir porque varB lo sigue referenciando.

    printf("Liberando varA final...\n");
    zynk_release(varA, &manager); // Libera el "Segundo Valor"

    printf("Liberando varB final...\n");
    zynk_release(varB, &manager); // Libera el "Primer Valor"

    printf("\n--- Pruebas de Conteo de Referencias Finalizadas ---\n");

    // 2. Liberar el ArenaManager al final de todas las operaciones
    // (Asegúrate de que no queden objetos no liberados para ver si hay fugas)
    deinit_sysarena(&manager);
    printf("ArenaManager desinicializado. Verifica el uso de memoria.\n");

    return 0;
}
