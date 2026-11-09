#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include <string.h>

// Un "string" que NO es dueño de su memoria: apunta a un pedazo del texto
// original (data) y dice cuántos caracteres ocupa (count). No termina en '\0'.
typedef struct string {
  char *data;
  size_t count;
} string;

// tokenizador para entrada de símbolos
typedef unsigned int u32;
typedef u32 TokenType;
enum {
  TokenType_Error,
  TokenType_EOF,
  TokenType_Ident,
  TokenType_Number,
  TokenType_Plus,
  TokenType_Minus,
  TokenType_Star,
  TokenType_Slash,
  TokenType_Percent,
  TokenType_Caret,
  TokenType_Equals,
  TokenType_OpenParenthesis,
  TokenType_CloseParenthesis,
  TokenType_Comma,
};

typedef struct Token {
  TokenType tipo;
  string lexema;
} Token;

typedef struct Lexer {
  char *start;   // dónde empieza el token que se está leyendo
  char *current; // el próximo carácter a leer
} Lexer;

Token siguiente_token(Lexer *lexer);
const char *nombre_token(TokenType tipo);

#endif
