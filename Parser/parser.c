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

// parsing functions: 

//
// <op> ::= '+'
//        | '-'
//        | '*'
//        | '**'
//        | '%'
//        | '/'
//        | '=='
//        | '!='
//        | '<'
//        | '<='
//        | '>'
//        | '>='
//        | is
//        | in
//
static bool parser_op(struct TokenQueue* tokens) {
  struct Token nextToken = tokenqueue_peekToken(tokens); 
  if (
      nextToken.id == nuPy_PLUS || 
      nextToken.id == nuPy_MINUS || 
      nextToken.id == nuPy_ASTERISK || 
      nextToken.id == nuPy_POWER || 
      nextToken.id == nuPy_PERCENT || 
      nextToken.id == nuPy_SLASH || 
      nextToken.id == nuPy_EQUALEQUAL || 
      nextToken.id == nuPy_NOTEQUAL || 
      nextToken.id == nuPy_LT || 
      nextToken.id == nuPy_LTE || 
      nextToken.id == nuPy_GT || 
      nextToken.id == nuPy_GTE || 
      nextToken.id == nuPy_KEYW_IS || 
      nextToken.id == nuPy_KEYW_IN
  ) {
    return true;
  }

  return false; 
}

//
// <element> ::= IDENTIFIER
//             | INT_LITERAL
//             | REAL_LITERAL
//             | STR_LITERAL
//             | True
//             | False
//             | None
//
static bool parser_element(struct TokenQueue* tokens) {
  struct Token nextToken = tokenqueue_peekToken(tokens);

  if (
      nextToken.id == nuPy_IDENTIFIER || 
      nextToken.id == nuPy_INT_LITERAL || 
      nextToken.id == nuPy_REAL_LITERAL || 
      nextToken.id == nuPy_STR_LITERAL || 
      nextToken.id == nuPy_KEYW_TRUE || 
      nextToken.id == nuPy_KEYW_FALSE || 
      nextToken.id == nuPy_KEYW_NONE
  ) {
    return true; 
  }

  return false; 
}

//
// <unary_expr> ::= '*' IDENTIFIER
//                | '&' IDENTIFIER
//                | '+' (IDENTIFIER | INT_LITERAL | REAL_LITERAL)
//                | '-' (IDENTIFIER | INT_LITERAL | REAL_LITERAL)
//                | <element>
//
static bool parser_unary_expr(struct TokenQueue* tokens) {
  struct Token nextToken = tokenqueue_peekToken(tokens); 
  struct Token nextnextToken = tokenqueue_peek2Token(tokens); 
  
  if (
      (nextToken.id == nuPy_ASTERISK && nextnextToken.id == nuPy_IDENTIFIER) || 
      (nextToken.id == nuPy_AMPERSAND && nextnextToken.id == nuPy_IDENTIFIER) || 
      ((nextToken.id == nuPy_PLUS || nextToken.id == nuPy_MINUS) && 
       (nextnextToken.id == nuPy_IDENTIFIER || 
        nextnextToken.id == nuPy_INT_LITERAL || 
        nextnextToken.id == nuPy_REAL_LITERAL))
  ) {
    return true; 
  } 
  
  bool result = parser_element(tokens); // if we get here, return true only if next encounter is an element 
  return result; 
}


//
// <expr> ::= <unary_expr> [<op> <unary_expr>]
//
static bool parser_expr(struct TokenQueue* tokens)
{
  if (!parser_unary_expr(tokens)) {
    return false; 
  }

  if (parser_op(tokens)) {
    if (!parser_unary_expr(tokens)) {
      return false; 
    }
  }

  return true;
}

//
// <function_call> ::= IDENTIFIER '(' [<element>] ')'
//
static bool parser_function_call(struct TokenQueue* tokens) {
  if (!match(tokens, nuPy_IDENTIFIER, "identifier")) {
    return false; 
  }

  if (!match(tokens, nuPy_LEFT_PAREN, '(')) {
    return false; 
  }

  if (parser_element(tokens)) { //handle optional element 

  }

  if (!match(tokens, nuPy_RIGHT_PAREN, ")")) {
    return false; 
  }

  return true; 
}


//
// <body> ::= '{' EOLN <stmts> '}' EOLN
//
static bool parser_body(struct TokenQueue* tokens)
{
  if (!match(tokens, nuPy_LEFT_BRACE, "{")) {
    return false; 
  }

  if (!match(tokens, nuPy_EOLN, "EOLN")) {
    return false; 
  }

  if (!parser_stmts(tokens)) {
    return false; 
  }

  if (!match(tokens, nuPy_RIGHT_BRACE, "}")) {
    return false; 
  }

  if (!match(tokens, nuPy_EOLN, "EOLN")) {
    return false; 
  }

  return true;
}



//
// <else> ::= elif <expr> ':' EOLN <body> [<else>]
//          | else ':' EOLN <body>
//
static bool parser_else(struct TokenQueue* tokens)
{ 
  struct Token nextToken = tokenqueue_peekToken(tokens); 
  char* nextValue = tokenqueue_peekValue(tokens);

  if (nextToken.id == nuPy_KEYW_ELIF) {
    tokenqueue_dequeue(tokens); // move on from elif 

    if (!parser_expr(tokens)) {
      return false; 
    }

    if (!match(tokens, nuPy_COLON, ":")) {
      return false; 
    }

    if (!match(tokens, nuPy_EOLN, "EOLN")) {
      return false; 
    }

    if (!parser_body(tokens)) {
      return false; 
    }

    struct Token optionalelse = tokenqueue_peekToken(tokens); //optional else handling 
    if (optionalelse.id == nuPy_KEYW_ELSE || optionalelse.id == nuPy_KEYW_ELIF) {
      bool result = parser_else(tokens); 
      return result; 
    }

    return true; 
  } else if (nextToken.id == nuPy_KEYW_ELSE) {
    tokenqueue_dequeue(tokens); //move on from else 

    if (!match(tokens, nuPy_COLON, ":")) {
      return false; 
    }

    if (!match(tokens, nuPy_EOLN, "EOLN")) {
      return false; 
    }

    if (!parser_body(tokens)) {
      return false; 
    }

    return true; 
  } else {
    errorMsg("else or elif", nextValue, nextToken); // if token wasn't else of elif => error 
    return false; 
  }
}

//
// <value> ::= <expr>
//           | <function_call>
//
static bool parser_value(struct TokenQueue* tokens) {
  struct Token nextToken = tokenqueue_peekToken(tokens); 
  struct Token nextnextToken = tokenqueue_peek2Token(tokens); 
  if (nextToken.id == nuPy_IDENTIFIER && nextnextToken.id == nuPy_LEFT_PAREN) {
    bool result = parser_function_call(tokens); 
    return result; 
  }
  bool result = parser_expr(tokens); 
  return result; 

}

//
// <assignment> ::= ['*'] IDENTIFIER '=' <value> EOLN
//
static bool parser_assignment(struct TokenQueue* tokens) {
  struct Token nextToken = tokenqueue_peekToken(tokens); 

  if (nextToken.id == nuPy_ASTERISK) {
    tokenqueue_dequeue(tokens); // optional * is present, advance to next token 
  }
  // either way, tokens should now be on the identifier 

  if (!match(tokens, nuPy_IDENTIFIER, "identifier")) {
    return false; 
  }
  
  if (!match(tokens, nuPy_EQUAL, "=")) {
    return false; 
  }

  if (!parser_value(tokens)) {
    return false; 
  }

  if (!match(tokens, nuPy_EOLN, "EOLN")) {
    return false; 
  }

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

  //
  // is the optional <else> present?
  //
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
// <while_loop> ::= while <expr> ':' EOLN <body>
// 
static bool parser_while_loop(struct TokenQueue* tokens) {
  if (!match(tokens, nuPy_KEYW_WHILE, "while")) {
    return false; 
  }

  if (!parser_expr(tokens)) {
    return false; 
  }

  if (!match(tokens, nuPy_COLON, ":")) {
    return false; 
  }

  if (!match(tokens, nuPy_EOLN, "EOLN")) {
    return false; 
  }

  if (!parser_body(tokens)) {
    return false; 
  }

  return true; 
}

//
// <call_stmt> ::= <function_call> EOLN
// 
static bool parser_call_stmt(struct TokenQueue* tokens) {
  if (!parser_function_call(tokens)) {
    return false; 
  }

  if (!match(tokens, nuPy_EOLN, "EOLN")) {
    return false; 
  }

  return true; 
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
  struct Token nextToken = tokenqueue_peekToken(tokens);
  struct Token nextnextToken = tokenqueue_peek2Token(tokens);


  if ( // handle all 6 stmt types using next and nextnext tokens 
      nextToken.id == nuPy_IDENTIFIER || 
      (nextToken.id == nuPy_ASTERISK && nextnextToken.id == nuPy_IDENTIFIER) || 
      nextToken.id == nuPy_KEYW_IF || 
      nextToken.id == nuPy_KEYW_WHILE || 
      nextToken.id == nuPy_KEYW_PASS || 
      nextToken.id == nuPy_EOLN
  ) {
    return true; 
  } else {
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
  if (!startOfStmt(tokens)) {
    struct Token curToken = tokenqueue_peekToken(tokens);
    char* curValue = tokenqueue_peekValue(tokens);

    errorMsg("start of a statement", curValue, curToken);
    return false;
  } // not a start of stmt

  // we have the start of a stmt, not branch into the correct one 
  struct Token nextToken = tokenqueue_peekToken(tokens);
  struct Token nextnextToken = tokenqueue_peek2Token(tokens); 

  if (nextToken.id == nuPy_ASTERISK && nextnextToken.id==nuPy_IDENTIFIER) {
    bool result = parser_assignment(tokens); 
    return result; 
  } else if (nextToken.id == nuPy_IDENTIFIER) {
    if (nextnextToken.id == nuPy_LEFT_PAREN) {
      bool result = parser_call_stmt(tokens); 
      return result; 
    } else if (nextnextToken.id == nuPy_EQUAL) {
      bool result = parser_assignment(tokens); 
      return result; 
    }
  } else if (nextToken.id == nuPy_KEYW_IF) {
    bool result = parser_if_then_else(tokens); 
    return result; 
  } else if (nextToken.id == nuPy_KEYW_WHILE) {
    bool result = parser_while_loop(tokens); 
    return result; 
  } else if (nextToken.id == nuPy_KEYW_PASS) {
    bool result = parser_pass_stmt(tokens);
    return result;
  } else if (nextToken.id == nuPy_EOLN) {
    bool result = parser_empty_stmt(tokens);
    return result;
  } else {
    printf("**INTERNAL ERROR: unknown stmt (parser_stmt)\n");
    return false;
  }
}



//
// <stmts> ::= <stmt> [<stmts>]
//
static bool parser_stmts(struct TokenQueue* tokens)
{
  if (!parser_stmt(tokens)) {
    return false; 
  }
  
  if (startOfStmt(tokens)) {
    bool result = parser_stmts(tokens); // optional stmt is there, so recursively parse
    return result; 
  }

  return true; // optional stmt not there, success => true
  
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
// for the subset of Python we are supporting. 
//
// Returns NULL if a syntax error was found; in this case 
// an error message was output. Returns a pointer to a list
// of tokens -- a Token Queue -- if no syntax errors were 
// detected. This queue contains the complete input in token
// form for further analysis.
//
// NOTE: it is the callers responsibility to free the resources
// used by the Token Queue.
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
