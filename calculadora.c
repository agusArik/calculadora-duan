#include "ast.h"
#include "evaluador.h"
#include "parser.h"
#include <stdio.h>

int main(void) {
  char entrada[256];
  char error[128];
  Entorno env;
  entorno_iniciar(&env);

  printf("Calculadora. Ejemplos: 1 + 2 * 3   x = 2^10   max(x, 5) / 2\n");
  printf("Ctrl+D o 'salir' para terminar.\n");

  for (;;) {
    printf("> ");
    if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
      break; // fin de la entrada (Ctrl+D)
    }
    size_t len = strlen(entrada);
    if (len > 0 && entrada[len - 1] == '\n') {
      entrada[len - 1] = '\0';
    }
    if (strcmp(entrada, "salir") == 0) {
      break;
    }
    if (entrada[0] == '\0') {
      continue;
    }

    Nodo *arbol = parsear(entrada, error, sizeof(error));
    if (arbol == NULL) {
      printf("Error de sintaxis: %s\n", error);
      continue;
    }

    ast_imprimir(arbol);

    env.error = NULL;
    double resultado = evaluar(arbol, &env);
    if (env.error) {
      printf("Error: %s\n", env.error);
    } else {
      printf("= %g\n", resultado);
    }

    ast_liberar(arbol);
  }

  entorno_liberar(&env);
  return 0;
}
