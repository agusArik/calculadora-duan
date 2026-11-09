#include "evaluador.h"
#include <math.h>
#include <stdlib.h>

static int nombre_igual(string a, const char *b) {
  return strlen(b) == a.count && memcmp(a.data, b, a.count) == 0;
}

static Variable *buscar_variable(Entorno *env, string nombre) {
  for (size_t i = 0; i < env->num_vars; i++) {
    if (nombre_igual(nombre, env->vars[i].nombre)) {
      return &env->vars[i];
    }
  }
  return NULL;
}

static void definir(Entorno *env, string nombre, double valor) {
  Variable *v = buscar_variable(env, nombre);
  if (v != NULL) {
    v->valor = valor;
    return;
  }
  if (env->num_vars == MAX_VARIABLES) {
    env->error = "demasiadas variables";
    return;
  }
  char *copia = malloc(nombre.count + 1);
  memcpy(copia, nombre.data, nombre.count);
  copia[nombre.count] = '\0';
  env->vars[env->num_vars].nombre = copia;
  env->vars[env->num_vars].valor = valor;
  env->num_vars++;
}

void entorno_iniciar(Entorno *env) {
  env->num_vars = 0;
  env->error = NULL;
  definir(env, (string){"pi", 2}, M_PI);
  definir(env, (string){"e", 1}, M_E);
}

void entorno_liberar(Entorno *env) {
  for (size_t i = 0; i < env->num_vars; i++) {
    free(env->vars[i].nombre);
  }
  env->num_vars = 0;
}

static double llamar(const Nodo *n, Entorno *env) {
  string nombre = n->llamada.nombre;
  size_t argc = n->llamada.num_args;
  double a[2] = {0, 0};
  for (size_t i = 0; i < argc && i < 2; i++) {
    a[i] = evaluar(n->llamada.args[i], env);
  }
  if (env->error) {
    return 0;
  }

  // funciones de 1 argumento
  struct {
    const char *nombre;
    double (*f)(double); // puntero a función
  } unarias[] = {
      {"sin", sin}, {"cos", cos},   {"tan", tan}, {"sqrt", sqrt},
      {"abs", fabs}, {"log", log},  {"exp", exp}, {"floor", floor},
      {"ceil", ceil},
  };
  for (size_t i = 0; i < sizeof(unarias) / sizeof(unarias[0]); i++) {
    if (nombre_igual(nombre, unarias[i].nombre)) {
      if (argc != 1) {
        env->error = "esa función recibe 1 argumento";
        return 0;
      }
      return unarias[i].f(a[0]);
    }
  }

  // funciones de 2 argumentos
  if (nombre_igual(nombre, "max") || nombre_igual(nombre, "min") ||
      nombre_igual(nombre, "pow")) {
    if (argc != 2) {
      env->error = "esa función recibe 2 argumentos";
      return 0;
    }
    if (nombre_igual(nombre, "max")) return a[0] > a[1] ? a[0] : a[1];
    if (nombre_igual(nombre, "min")) return a[0] < a[1] ? a[0] : a[1];
    return pow(a[0], a[1]);
  }

  env->error = "función desconocida";
  return 0;
}

double evaluar(const Nodo *n, Entorno *env) {
  if (env->error) {
    return 0; // ya falló algo antes: no seguimos
  }

  switch (n->tipo) {
  case Nodo_Numero:
    return n->numero;

  case Nodo_Variable: {
    Variable *v = buscar_variable(env, n->nombre);
    if (v == NULL) {
      env->error = "variable no definida";
      return 0;
    }
    return v->valor;
  }

  case Nodo_Unario: {
    double x = evaluar(n->unario.operando, env);
    return n->unario.op == TokenType_Minus ? -x : x;
  }

  case Nodo_Binario: {
    // primero se evalúan los hijos, después se combina: recorrido post-orden
    double izq = evaluar(n->binario.izq, env);
    double der = evaluar(n->binario.der, env);
    switch (n->binario.op) {
    case TokenType_Plus:
      return izq + der;
    case TokenType_Minus:
      return izq - der;
    case TokenType_Star:
      return izq * der;
    case TokenType_Slash:
      if (der == 0) {
        env->error = "división por cero";
        return 0;
      }
      return izq / der;
    case TokenType_Percent:
      if (der == 0) {
        env->error = "módulo por cero";
        return 0;
      }
      return fmod(izq, der);
    case TokenType_Caret:
      return pow(izq, der);
    }
    env->error = "operador desconocido";
    return 0;
  }

  case Nodo_Asignacion: {
    double valor = evaluar(n->asignacion.valor, env);
    if (!env->error) {
      definir(env, n->asignacion.nombre, valor);
    }
    return valor;
  }

  case Nodo_Llamada:
    return llamar(n, env);
  }

  env->error = "nodo desconocido";
  return 0;
}
