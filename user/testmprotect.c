#include "user.h"
#include "types.h"


int main() {
    char *addr = sbrk(2 * 4096); // Reservar espacio para dos páginas
    if (mprotect(addr, 1) < 0) {
        printf("mprotect failed :(\n");
        exit(1);
    }

    // Intentar escribir en la página protegida
    addr[0] = 'A'; // Esto debería causar un fallo de segmentación

    if (munprotect(addr, 1) < 0) {
        printf("munprotect failed :(\n");
        exit(1);
    }

    addr[0] = 'B'; // Esto debería funcionar

    printf("Test passed! :)\n");
    exit(0);
}
