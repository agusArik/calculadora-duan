#include "lexer.h"
#include <ctype.h>

static char pico(Lexer *lexer) {
  return *lexer->current;
} // mira el carácter actual sin consumirlo

static char avanzar(Lexer *lexer) {
  char c = *lexer->current;
  lexer->current++;
  return c;
}

static int es_fin(Lexer *lexer) { return *lexer->current == '\0'; }

static Token crear_token(Lexer *lexer, TokenType tipo) {
  Token token;
  token.tipo = tipo;
  token.lexema.data = lexer->start;
  token.lexema.count = (size_t)(lexer->current - lexer->start);
  return token;
}

Token siguiente_token(Lexer *lexer) {
  while (!es_fin(lexer) && isspace((unsigned char)pico(lexer))) {
    avanzar(lexer);
  }
  lexer->start = lexer->current;

  if (es_fin(lexer)) {
    return crear_token(lexer, TokenType_EOF);
  }

  char c = avanzar(lexer);

  if (isdigit((unsigned char)c)) {
    while (!es_fin(lexer) && isdigit((unsigned char)pico(lexer))) {
      avanzar(lexer);
    }
    if (!es_fin(lexer) && pico(lexer) == '.') {
      avanzar(lexer);
      while (!es_fin(lexer) && isdigit((unsigned char)pico(lexer))) {
        avanzar(lexer);
      }
    }
    return crear_token(lexer, TokenType_Number);
  }

  if (isalpha((unsigned char)c) || c == '_') {
    while (!es_fin(lexer) &&
           (isalnum((unsigned char)pico(lexer)) || pico(lexer) == '_')) {
      avanzar(lexer);
    }
    return crear_token(lexer, TokenType_Ident);
  }

  switch (c) {
  case '+':
    return crear_token(lexer, TokenType_Plus);
  case '-':
    return crear_token(lexer, TokenType_Minus);
  case '*':
    return crear_token(lexer, TokenType_Star);
  case '/':
    return crear_token(lexer, TokenType_Slash);
  case '%':
    return crear_token(lexer, TokenType_Percent);
  case '^':
    return crear_token(lexer, TokenType_Caret);
  case '=':
    return crear_token(lexer, TokenType_Equals);
  case '(':
    return crear_token(lexer, TokenType_OpenParenthesis);
  case ')':
    return crear_token(lexer, TokenType_CloseParenthesis);
  case ',':
    return crear_token(lexer, TokenType_Comma);
  }

  return crear_token(lexer, TokenType_Error);
}

const char *nombre_token(TokenType tipo) {
  switch (tipo) {
  case TokenType_Error:
    return "ERROR";
  case TokenType_EOF:
    return "EOF";
  case TokenType_Ident:
    return "IDENT";
  case TokenType_Number:
    return "NUMBER";
  case TokenType_Plus:
    return "PLUS";
  case TokenType_Minus:
    return "MINUS";
  case TokenType_Star:
    return "STAR";
  case TokenType_Slash:
    return "SLASH";
  case TokenType_Percent:
    return "PERCENT";
  case TokenType_Caret:
    return "CARET";
  case TokenType_Equals:
    return "EQUALS";
  case TokenType_OpenParenthesis:
    return "OPEN_PAREN";
  case TokenType_CloseParenthesis:
    return "CLOSE_PAREN";
  case TokenType_Comma:
    return "COMMA";
  default:
    return "UNKNOWN";
  }
}
