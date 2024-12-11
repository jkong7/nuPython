/*parser.c*/

//
// Recursive-descent parsing functions for the programming language.
// The parser is responsible for checking if the input follows the syntax
// ("grammar") rules. If successful, a copy of the tokens is
// returned so the program can be analyzed and executed.
//


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  
#include <assert.h>

#include "token.h"
#include "tokenqueue.h"
#include "scanner.h"
#include "parser.h"


//
// declarations of private functions:
//
static void errorMsg(char* expecting, char* value, struct Token found);
static bool match(struct TokenQueue* tokens, int expectedID, char* expectedValue);

static bool parser_expr(struct TokenQueue* tokens);
static bool parser_body(struct TokenQueue* tokens);
static bool parser_else(struct TokenQueue* tokens);

static bool parser_if_then_else(struct TokenQueue* tokens);
static bool parser_pass_stmt(struct TokenQueue* tokens);
static bool parser_empty_stmt(struct TokenQueue* tokens);
static bool startOfStmt(struct TokenQueue* tokens);
static bool parser_stmt(struct TokenQueue* tokens);
static bool parser_stmts(struct TokenQueue* tokens);
static bool parser_program(struct TokenQueue* tokens);


//
// errorMsg:
//
// Outputs a properly-formatted syntax error message of the form
// "expecting X, found Y".
//
static void errorMsg(char* expecting, char* value, struct Token found)
{
  printf("**SYNTAX ERROR @ (%d,%d): expecting %s, found '%s'\n",
    found.line, found.col, expecting, value);
}


//
// match
//
// Checks to see if the token at the front of the queue matches
// the exectedID. If so, the token is removed from the queue and
// true is returned. If not, an error message is output and false
// is returned.
// 
// If false is returned, the error message output is of the form
// "expecting X, found Y" where X is the value of the expected 
// token and Y is the expectedValue passed in. 
//
static bool match(struct TokenQueue* tokens, int expectedID, char* expectedValue)
{
  //
  // does the token match the expected token?
  //
  struct Token curToken = tokenqueue_peekToken(tokens);
  char* curValue = tokenqueue_peekValue(tokens);

  if (curToken.id != expectedID)  // no, => error
  {
    errorMsg(expectedValue, curValue, curToken);
    return false;
  }

  //
  // yes, it matched, so discard and return true:
  //
  tokenqueue_dequeue(tokens);

  return true;
}


//
// <expr> ::= <unary_expr> [<op> <unary_expr>]
//
static bool parser_expr(struct TokenQueue* tokens)
{
  return true;
}




//
// <body> ::= '{' EOLN <stmts> '}' EOLN
//
static bool parser_body(struct TokenQueue* tokens)
{

  return true;
}


//
// <else> ::= elif <expr> ':' EOLN <body> [<else>]
//          | else ':' EOLN <body>
//
static bool parser_else(struct TokenQueue* tokens)
{

  return true;
}


//
// <if_then_else> ::= if <expr> ':' EOLN <body> [<else>]
//
static bool parser_if_then_else(struct TokenQueue* tokens)
{
  if (!match(tokens, nuPy_KEYW_IF, "if"))
    return false;

  if (!parser_expr(tokens))
    return false;

  if (!match(tokens, nuPy_COLON, ":"))
    return false;

  if (!match(tokens, nuPy_EOLN, "EOLN"))
    return false;

  if (!parser_body(tokens))
    return false;

  // is the optional <else> present?
  struct Token curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == nuPy_KEYW_ELIF || curToken.id == nuPy_KEYW_ELSE)
  {
    bool result = parser_else(tokens);
    return result;
  }
  else
  {
    // <else> is optional, missing => do nothing and return success:
    return true;
  }
}


// 
// <pass_stmt> ::= pass EOLN
//
static bool parser_pass_stmt(struct TokenQueue* tokens)
{
  if (!match(tokens, nuPy_KEYW_PASS, "pass"))
    return false;

  if (!match(tokens, nuPy_EOLN, "EOLN"))
    return false;

  return true;
}


// 
// <empty_stmt> ::= EOLN
//
static bool parser_empty_stmt(struct TokenQueue* tokens)
{
  if (!match(tokens, nuPy_EOLN, "EOLN"))
    return false;

  return true;
}


//
// startOfStmt
//
// Returns true if the next token denotes the start of a stmt,
// and false if not.
//
static bool startOfStmt(struct TokenQueue* tokens)
{
  struct Token curToken = tokenqueue_peekToken(tokens);



  if (curToken.id == nuPy_KEYW_PASS ||
      curToken.id == nuPy_EOLN) {
    return true;
  }
  else {
    return false;
  }
}


//
// <stmt> ::= <assignment>
//          | <if_then_else>
//          | <while_loop>
//          | <call_stmt>
//          | <pass_stmt>
//          | <empty_stmt>
//
static bool parser_stmt(struct TokenQueue* tokens)
{
  //
  // TODO: for now we just accept a program consisting of a
  // single "pass" or "empty" statement.
  //
  if (!startOfStmt(tokens)) {
    struct Token curToken = tokenqueue_peekToken(tokens);
    char* curValue = tokenqueue_peekValue(tokens);

    errorMsg("start of a statement", curValue, curToken);
    return false;
  }

  //
  // we have the start of a stmt, but which one?
  //
  struct Token curToken = tokenqueue_peekToken(tokens);

  if (curToken.id == nuPy_KEYW_PASS) {
    bool result = parser_pass_stmt(tokens);
    return result;
  }
  else if (curToken.id == nuPy_EOLN) {
    bool result = parser_empty_stmt(tokens);
    return result;
  }
  else {
    printf("**INTERNAL ERROR: unknown stmt (parser_stmt)\n");
    return false;
  }
}



//
// <stmts> ::= <stmt> [<stmts>]
//
static bool parser_stmts(struct TokenQueue* tokens)
{
  //
  // TODO: for now we just accept a program consisting of a
  // single statement.
  //
  if (!parser_stmt(tokens))
    return false;
  
  struct Token nextToken = tokenqueue_peekToken(tokens); 
  struct Token nextnextToken = tokenqueue_peekToken(tokens); 

  if ( // check if the optional stmt is there, handle all 6 stmt types using next and nextnext tokens 
      nextToken.id == nuPy_IDENTIFIER || 
      (nextToken.id == nuPy_ASTERISK && nextnextToken.id == nuPy_IDENTIFIER) || 
      nextToken.id == nuPy_KEYW_IF || 
      nextToken.id == nuPy_KEYW_WHILE || 
      nextToken.id == nuPy_KEYW_PASS || 
      nextToken.id == nuPy_EOLN
  ) {
    return parser_stmts(tokens); // it is there, so recursively parse 
  }

  return true; // optional stmt not there, still valid so return true 
}


//
// <program> ::= <stmts> EOS
//
static bool parser_program(struct TokenQueue* tokens)
{
  if (!parser_stmts(tokens))
    return false;

  if (!match(tokens, nuPy_EOS, "$"))
    return false;

  return true;
}


//
// public functions:
//

//
// parser_parse
//
// Given an input stream, uses the scanner to obtain the tokens
// and then checks the syntax of the input against the BNF rules
// of the language
//
struct TokenQueue* parser_parse(FILE* input)
{
  if (input == NULL) {
    printf("**INTERNAL ERROR: input stream is NULL (parser_parse)\n");
    return NULL;
  }

  //
  // First, let's get all the tokens and store them
  // into a queue:
  //
  int lineNumber, colNumber;
  char value[256];
  struct Token token;
  struct TokenQueue* tokens;

  scanner_init(&lineNumber, &colNumber, value);

  token = scanner_nextToken(input, &lineNumber, &colNumber, value);
  tokens = tokenqueue_create();

  while (token.id != nuPy_EOS)
  {
    tokenqueue_enqueue(tokens, token, value);

    token = scanner_nextToken(input, &lineNumber, &colNumber, value);
  }

  // enqueue the final token:
  tokenqueue_enqueue(tokens, token, value);

  //
  // now duplicate the tokens so that we have a copy after the
  // parsing process is over --- we need a copy so we can return
  // in case the parsing is successful. The tokens are then used
  // for analysis and execution.
  //
  struct TokenQueue* duplicate;

  duplicate = tokenqueue_duplicate(tokens);

  //
  // okay, now let's parse the input tokens:
  //
  bool result = parser_program(tokens);

  //
  // When we are done parsing, we are going to 
  // execute (assuming the parse was successful).
  // If the input is coming from the keyboard, 
  // consume the rest of the input after the $ 
  // before we start executing the python which
  // may do it's own input from the keyboard:
  //
  if (result && input == stdin) {
    int c = fgetc(stdin);
    while (c != '\n' && c != EOF)
      c = fgetc(stdin);
  }

  //
  // done, free memory and return tokens or NULL:
  //
  tokenqueue_destroy(tokens);

  if (result) // parse was successful
  {
    return duplicate;
  }
  else  // syntax error, nothing to execute:
  {
    tokenqueue_destroy(duplicate);

    return NULL;
  }
}
