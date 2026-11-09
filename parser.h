#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

// Convierte el texto en un árbol.
// Si hay un error de sintaxis devuelve NULL y escribe el motivo en `error`.
// OJO: los nombres del árbol apuntan dentro de `entrada`, así que `entrada`
// tiene que seguir viva mientras se use el árbol.
Nodo *parsear(char *entrada, char *error, size_t tam_error);

#endif
