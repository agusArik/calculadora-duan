#ifndef EVALUADOR_H
#define EVALUADOR_H

#include "ast.h"

#define MAX_VARIABLES 64

typedef struct Variable {
  char *nombre; // copia propia (malloc), no apunta a la entrada
  double valor;
} Variable;

typedef struct Entorno {
  Variable vars[MAX_VARIABLES];
  size_t num_vars;
  const char *error; // NULL si no hubo error
} Entorno;

void entorno_iniciar(Entorno *env);
void entorno_liberar(Entorno *env);

// Recorre el árbol y calcula su valor.
// Si algo sale mal deja el motivo en env->error.
double evaluar(const Nodo *nodo, Entorno *env);

#endif
