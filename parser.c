// Parser de descenso recursivo.
//
// Gramática (de menor a mayor precedencia):
//
//   expresion  -> asignacion
//   asignacion -> IDENT '=' asignacion | suma
//   suma       -> termino (('+' | '-') termino)*
//   termino    -> unario (('*' | '/' | '%') unario)*
//   unario     -> ('-' | '+') unario | potencia
//   potencia   -> primario ('^' unario)?        (asocia a la derecha)
//   primario   -> NUMBER
//               | IDENT
//               | IDENT '(' (expresion (',' expresion)*)? ')'
//               | '(' expresion ')'
//
// Cada regla es una función. Las de menor precedencia llaman a las de
// mayor, por eso en "1 + 2 * 3" el '*' queda más abajo en el árbol y se
// calcula primero.

#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Parser {
  Lexer lexer;
  Token actual; // el token que estamos mirando
  int hubo_error;
  char *error;
  size_t tam_error;
} Parser;

static void avanzar_token(Parser *p) { p->actual = siguiente_token(&p->lexer); }

static void reportar_error(Parser *p, const char *mensaje) {
  if (p->hubo_error) {
    return; // nos quedamos con el primer error
  }
  p->hubo_error = 1;
  if (p->actual.tipo == TokenType_EOF) {
    snprintf(p->error, p->tam_error, "%s (al final de la entrada)", mensaje);
  } else {
    snprintf(p->error, p->tam_error, "%s (cerca de '%.*s')", mensaje,
             (int)p->actual.lexema.count, p->actual.lexema.data);
  }
}

// Si el token actual es `tipo`, lo consume y devuelve 1. Si no, devuelve 0.
static int coincide(Parser *p, TokenType tipo) {
  if (p->actual.tipo == tipo) {
    avanzar_token(p);
    return 1;
  }
  return 0;
}

static Nodo *expresion(Parser *p);
static Nodo *unario(Parser *p);

static double convertir_numero(string lexema) {
  char buffer[64];
  size_t n = lexema.count < sizeof(buffer) - 1 ? lexema.count : sizeof(buffer) - 1;
  memcpy(buffer, lexema.data, n);
  buffer[n] = '\0';
  return strtod(buffer, NULL);
}

static Nodo *primario(Parser *p) {
  Token token = p->actual;

  if (coincide(p, TokenType_Number)) {
    return nodo_numero(convertir_numero(token.lexema));
  }

  if (coincide(p, TokenType_Ident)) {
    if (!coincide(p, TokenType_OpenParenthesis)) {
      return nodo_variable(token.lexema);
    }
    // es una llamada a función: nombre(arg1, arg2, ...)
    Nodo **args = NULL;
    size_t num_args = 0;
    if (!coincide(p, TokenType_CloseParenthesis)) {
      do {
        args = realloc(args, (num_args + 1) * sizeof(Nodo *));
        args[num_args++] = expresion(p);
      } while (coincide(p, TokenType_Comma));
      if (!coincide(p, TokenType_CloseParenthesis)) {
        reportar_error(p, "se esperaba ')' después de los argumentos");
      }
    }
    return nodo_llamada(token.lexema, args, num_args);
  }

  if (coincide(p, TokenType_OpenParenthesis)) {
    Nodo *adentro = expresion(p);
    if (!coincide(p, TokenType_CloseParenthesis)) {
      reportar_error(p, "se esperaba ')'");
    }
    return adentro; // los paréntesis no generan nodo: ya están en la forma del árbol
  }

  reportar_error(p, "se esperaba un número, variable o '('");
  return NULL;
}

static Nodo *potencia(Parser *p) {
  Nodo *base = primario(p);
  if (coincide(p, TokenType_Caret)) {
    // llamamos a unario (que puede volver a llegar a potencia) para que
    // 2^3^2 sea 2^(3^2) y para permitir 2^-1
    Nodo *exponente = unario(p);
    return nodo_binario(TokenType_Caret, base, exponente);
  }
  return base;
}

static Nodo *unario(Parser *p) {
  TokenType op = p->actual.tipo;
  if (coincide(p, TokenType_Minus) || coincide(p, TokenType_Plus)) {
    return nodo_unario(op, unario(p));
  }
  return potencia(p);
}

static Nodo *termino(Parser *p) {
  Nodo *izq = unario(p);
  for (;;) {
    TokenType op = p->actual.tipo;
    if (!(coincide(p, TokenType_Star) || coincide(p, TokenType_Slash) ||
          coincide(p, TokenType_Percent))) {
      break;
    }
    Nodo *der = unario(p);
    izq = nodo_binario(op, izq, der); // el árbol crece hacia la izquierda
  }
  return izq;
}

static Nodo *suma(Parser *p) {
  Nodo *izq = termino(p);
  for (;;) {
    TokenType op = p->actual.tipo;
    if (!(coincide(p, TokenType_Plus) || coincide(p, TokenType_Minus))) {
      break;
    }
    Nodo *der = termino(p);
    izq = nodo_binario(op, izq, der);
  }
  return izq;
}

static Nodo *asignacion(Parser *p) {
  Nodo *izq = suma(p);
  if (p->actual.tipo != TokenType_Equals) {
    return izq;
  }
  if (izq == NULL || izq->tipo != Nodo_Variable) {
    reportar_error(p, "solo se puede asignar a una variable");
  }
  avanzar_token(p);            // consumir el '='
  Nodo *valor = asignacion(p); // a = b = 3 asocia a la derecha
  if (izq == NULL || izq->tipo != Nodo_Variable) {
    ast_liberar(izq);
    return valor;
  }
  string nombre = izq->nombre;
  ast_liberar(izq);
  return nodo_asignacion(nombre, valor);
}

static Nodo *expresion(Parser *p) { return asignacion(p); }

Nodo *parsear(char *entrada, char *error, size_t tam_error) {
  Parser p = {0};
  p.lexer.start = entrada;
  p.lexer.current = entrada;
  p.error = error;
  p.tam_error = tam_error;
  error[0] = '\0';

  avanzar_token(&p); // cargar el primer token
  Nodo *raiz = expresion(&p);

  if (p.actual.tipo != TokenType_EOF) {
    reportar_error(&p, "token inesperado");
  }
  if (p.hubo_error) {
    ast_liberar(raiz);
    return NULL;
  }
  return raiz;
}
