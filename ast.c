#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

static Nodo *nuevo_nodo(TipoNodo tipo) {
  Nodo *nodo = malloc(sizeof(Nodo));
  if (nodo == NULL) {
    fprintf(stderr, "sin memoria\n");
    exit(1);
  }
  nodo->tipo = tipo;
  return nodo;
}

Nodo *nodo_numero(double valor) {
  Nodo *n = nuevo_nodo(Nodo_Numero);
  n->numero = valor;
  return n;
}

Nodo *nodo_variable(string nombre) {
  Nodo *n = nuevo_nodo(Nodo_Variable);
  n->nombre = nombre;
  return n;
}

Nodo *nodo_unario(TokenType op, Nodo *operando) {
  Nodo *n = nuevo_nodo(Nodo_Unario);
  n->unario.op = op;
  n->unario.operando = operando;
  return n;
}

Nodo *nodo_binario(TokenType op, Nodo *izq, Nodo *der) {
  Nodo *n = nuevo_nodo(Nodo_Binario);
  n->binario.op = op;
  n->binario.izq = izq;
  n->binario.der = der;
  return n;
}

Nodo *nodo_asignacion(string nombre, Nodo *valor) {
  Nodo *n = nuevo_nodo(Nodo_Asignacion);
  n->asignacion.nombre = nombre;
  n->asignacion.valor = valor;
  return n;
}

Nodo *nodo_llamada(string nombre, Nodo **args, size_t num_args) {
  Nodo *n = nuevo_nodo(Nodo_Llamada);
  n->llamada.nombre = nombre;
  n->llamada.args = args;
  n->llamada.num_args = num_args;
  return n;
}

void ast_liberar(Nodo *nodo) {
  if (nodo == NULL) {
    return;
  }
  switch (nodo->tipo) {
  case Nodo_Numero:
  case Nodo_Variable:
    break; // no tienen hijos
  case Nodo_Unario:
    ast_liberar(nodo->unario.operando);
    break;
  case Nodo_Binario:
    ast_liberar(nodo->binario.izq);
    ast_liberar(nodo->binario.der);
    break;
  case Nodo_Asignacion:
    ast_liberar(nodo->asignacion.valor);
    break;
  case Nodo_Llamada:
    for (size_t i = 0; i < nodo->llamada.num_args; i++) {
      ast_liberar(nodo->llamada.args[i]);
    }
    free(nodo->llamada.args);
    break;
  }
  free(nodo);
}

// ---------- impresión del árbol ----------

static const char *simbolo(TokenType op) {
  switch (op) {
  case TokenType_Plus:
    return "+";
  case TokenType_Minus:
    return "-";
  case TokenType_Star:
    return "*";
  case TokenType_Slash:
    return "/";
  case TokenType_Percent:
    return "%";
  case TokenType_Caret:
    return "^";
  default:
    return "?";
  }
}

static void imprimir_etiqueta(const Nodo *n) {
  if (n == NULL) {
    printf("(vacío)");
    return;
  }
  switch (n->tipo) {
  case Nodo_Numero:
    printf("%g", n->numero);
    break;
  case Nodo_Variable:
    printf("%.*s", (int)n->nombre.count, n->nombre.data);
    break;
  case Nodo_Unario:
    printf("%s (unario)", simbolo(n->unario.op));
    break;
  case Nodo_Binario:
    printf("%s", simbolo(n->binario.op));
    break;
  case Nodo_Asignacion:
    printf("%.*s =", (int)n->asignacion.nombre.count,
           n->asignacion.nombre.data);
    break;
  case Nodo_Llamada:
    printf("%.*s()", (int)n->llamada.nombre.count, n->llamada.nombre.data);
    break;
  }
}

static size_t cantidad_hijos(const Nodo *n) {
  if (n == NULL) {
    return 0;
  }
  switch (n->tipo) {
  case Nodo_Unario:
  case Nodo_Asignacion:
    return 1;
  case Nodo_Binario:
    return 2;
  case Nodo_Llamada:
    return n->llamada.num_args;
  default:
    return 0;
  }
}

static const Nodo *hijo(const Nodo *n, size_t i) {
  switch (n->tipo) {
  case Nodo_Unario:
    return n->unario.operando;
  case Nodo_Asignacion:
    return n->asignacion.valor;
  case Nodo_Binario:
    return i == 0 ? n->binario.izq : n->binario.der;
  case Nodo_Llamada:
    return n->llamada.args[i];
  default:
    return NULL;
  }
}

// `prefijo` es lo que se dibuja a la izquierda (las líneas verticales de
// los niveles de arriba).
static void imprimir_hijos(const Nodo *n, const char *prefijo) {
  size_t total = cantidad_hijos(n);
  for (size_t i = 0; i < total; i++) {
    int es_ultimo = (i == total - 1);
    const Nodo *h = hijo(n, i);

    printf("%s%s", prefijo, es_ultimo ? "└── " : "├── ");
    imprimir_etiqueta(h);
    printf("\n");

    char nuevo_prefijo[512];
    snprintf(nuevo_prefijo, sizeof(nuevo_prefijo), "%s%s", prefijo,
             es_ultimo ? "    " : "│   ");
    imprimir_hijos(h, nuevo_prefijo);
  }
}

void ast_imprimir(const Nodo *nodo) {
  imprimir_etiqueta(nodo);
  printf("\n");
  imprimir_hijos(nodo, "");
}
