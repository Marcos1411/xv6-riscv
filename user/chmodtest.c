#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main() {
  char *filename = "testfile";

  // Crear archivo
  int fd = open(filename, O_CREATE | O_RDWR);
  if(fd < 0) {
    printf("Error creando archivo\n");
    exit(1);
  }
  write(fd, "hello", 5);
  close(fd);

  // Cambiar permisos a solo lectura
  if(chmod(filename, 1) < 0) {
    printf("Error cambiando permisos\n");
    exit(1);
  }

  // Intentar abrir en modo escritura
  fd = open(filename, O_WRONLY);
  if(fd >= 0) {
    printf("Error: Se pudo abrir en modo escritura con permisos de solo lectura\n");
    exit(1);
  }

  // Cambiar permisos a inmutable
  if(chmod(filename, 5) < 0) {
    printf("Error cambiando a inmutable\n");
    exit(1);
  }

  // Intentar modificar permisos de archivo inmutable
  if(chmod(filename, 3) == 0) {
    printf("Error: Se pudieron modificar permisos de archivo inmutable\n");
    exit(1);
  }

  printf("Pruebas completadas exitosamente\n");
  exit(0);
}
