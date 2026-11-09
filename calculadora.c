#include "lexer.h"
#include <stddef.h>
#include <stdio.h>

int main(void) {
  char entrada[256];
  fgets(entrada, sizeof(entrada), stdin);
  size_t len = strlen(entrada);
  if (len > 0 && entrada[len - 1] == '\n') {
    entrada[len - 1] = '\0';
  }

  printf("Calculadora. Ejemplos: 1 + 2 * 3   x = 2^10   max(x, 5) / 2\n");
  printf("Ctrl+D o 'salir' para terminar.\n");

  Lexer lexer = {.start = entrada, .current = entrada};
  Token token;
  do {
    token = siguiente_token(&lexer);
    printf("%-12s '%.*s'\n", nombre_token(token.tipo), (int)token.lexema.count,
           token.lexema.data);
  } while (token.tipo != TokenType_EOF);

  return 0;
}
