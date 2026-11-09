#ifndef AST_H
#define AST_H

#include "lexer.h"

// Cada nodo del árbol es de uno de estos tipos.
typedef enum {
  Nodo_Numero,     // 3.14
  Nodo_Variable,   // x
  Nodo_Unario,     // -x
  Nodo_Binario,    // a + b, a * b, a ^ b, ...
  Nodo_Asignacion, // x = expr
  Nodo_Llamada,    // max(a, b)
} TipoNodo;

typedef struct Nodo Nodo;

// Un nodo guarda su tipo y, en una "union", los datos de ESE tipo.
// Una union reserva memoria para el campo más grande y todos comparten
// el mismo lugar: solo se usa el campo que corresponde a `tipo`.
struct Nodo {
  TipoNodo tipo;
  union {
    double numero; // Nodo_Numero
    string nombre; // Nodo_Variable

    struct {
      TokenType op;
      Nodo *operando;
    } unario;

    struct {
      TokenType op;
      Nodo *izq;
      Nodo *der;
    } binario;

    struct {
      string nombre;
      Nodo *valor;
    } asignacion;

    struct {
      string nombre;
      Nodo **args; // arreglo dinámico de punteros a nodos
      size_t num_args;
    } llamada;
  };
};

// Constructores: reservan un nodo en el heap (malloc) y lo devuelven.
Nodo *nodo_numero(double valor);
Nodo *nodo_variable(string nombre);
Nodo *nodo_unario(TokenType op, Nodo *operando);
Nodo *nodo_binario(TokenType op, Nodo *izq, Nodo *der);
Nodo *nodo_asignacion(string nombre, Nodo *valor);
Nodo *nodo_llamada(string nombre, Nodo **args, size_t num_args);

// Libera el nodo y todos sus hijos (recursivo). Acepta NULL.
void ast_liberar(Nodo *nodo);

// Dibuja el árbol en la terminal.
void ast_imprimir(const Nodo *nodo);

#endif
