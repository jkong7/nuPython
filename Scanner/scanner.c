/*scanner.c*/

#include <stdio.h>
#include <stdbool.h>  // true, false
#include <ctype.h>    // isspace, isdigit, isalpha
#include <string.h>   // strcmp
#include <assert.h>   // assert

#include "util.h"
#include "scanner.h"


//
// collect_identifier
//
// Given the start of an identifier, collects the rest into value
// while advancing the column number.
//
static void collect_identifier(FILE* input, int c, int* colNumber, char* value)
{
  assert(isalpha(c) || c == '_');  // c should be start of identifier

  int i = 0;

  while (isalnum(c) || c == '_')  // letter, digit, or underscore
  {
    value[i] = (char)c; 
    i++;

    (*colNumber)++; 

    c = fgetc(input); 
  }

  // at this point we found the end of the identifer, so put
  // that last char back for processing later:
  ungetc(c, input);

  // turn the value into a string, and let's see if we have a keyword:
  value[i] = '\0';  // build C-style string:

  return;
}


//
// scanner_init
//
// Initializes line number, column number, and value before
// the start of the processing the next input stream. Scanner 
// uses same lineNumber and colNumber variables by passing by 
// reference so the current line and col numbers are always 
// used. 
//
void scanner_init(int* lineNumber, int* colNumber, char* value)
{
  if (lineNumber == NULL || colNumber == NULL || value == NULL)
    panic("one or more parameters are NULL (scanner_init)");

  *lineNumber = 1;
  *colNumber = 1;
  value[0] = '\0';  // empty string
}

//
// scanner_nextToken
//
// Returns the next token in the given input stream, advancing the line
// number and column number as appropriate. The token's string-based 
// value is returned via the "value" parameter. For example, if the 
// token returned is an integer literal, then the value returned is
// the actual literal in string form, e.g. "123". For an identifer,
// the value is the identifer itself, e.g. "print" or "x". For a 
// string literal such as 'hi there', the value is the contents of the 
// string literal without the quotes.
//
struct Token scanner_nextToken(FILE* input, int* lineNumber, int* colNumber, char* value)
{
  if (input == NULL)
    panic("input stream is NULL (scanner_nextToken)");
  if (lineNumber == NULL || colNumber == NULL || value == NULL)
    panic("one or more parameters are NULL (scanner_nextToken)");

  struct Token T;

  // repeatedly input characters one by one until a token is found:
  while (true)
  {
    // Get the next input character:
    int c = fgetc(input);

    // scan c!!
    if (c == EOF)  // no more input, return EOS:
    {
      T.id = nuPy_EOS;
      T.line = *lineNumber;
      T.col = *colNumber;

      value[0] = '$';
      value[1] = '\0';

      return T;
    }
    else if (c == '$')  // this is also EOS
    {
      T.id = nuPy_EOS;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; 

      value[0] = '$';
      value[1] = '\0';

      return T;
    }
    else if (c == '\n')  // end of line, keep going:
    {
      (*lineNumber)++;  // next line, restart column:
      *colNumber = 1;
      continue;
    }
    else if (isspace(c))  // other form of whitespace, skip
    {
      (*colNumber)++;  
      continue;
    }
    else if (c == '(')
    {
      T.id = nuPy_LEFT_PAREN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; 

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }
    else if (c == ')')
    {
      T.id = nuPy_RIGHT_PAREN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;  

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }
    else if (isalpha(c) || c == '_') 
    {
      // start of identifier or keyword, let's assume identifier for now:
      T.id = nuPy_IDENTIFIER;
      T.line = *lineNumber;
      T.col = *colNumber;

      collect_identifier(input, c, colNumber, value);

      // Is the identifier a keyword? If so, return that
      // token id instead.

      return T;
    }
    else if (c == '*')
    {
      // could be * or **, let's assume * for now:
      T.id = nuPy_ASTERISK;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; 

      value[0] = '*';
      value[1] = '\0';

      // now let's read the next char and see what we have:
      c = fgetc(input);

      if (c == '*')  // it's **
      {
        T.id = nuPy_POWER;

        (*colNumber)++;  // advance col # past char

        value[1] = '*';
        value[2] = '\0';

        return T;
      }

      // if we get here, then next char did not 
      // form a token, so we need to put the char
      // back to be processed on the next call:
      ungetc(c, input);

      return T;
    } //TODO: 
    else if (c == '+') {

    }
    else if (c == '-') {

    }
    else if (c == '/') {

    } 
    else if (c == '%') {

    } 
    else if (c == '=') {

    }
    else if (c == '==') {

    }
    else if (c == '!=') {

    }
    else if (c == '<') {

    }
    else if (c == '<=') {
      
    }
    else if (c == '>') {
      
    }
    else if (c == '>=') {
      
    }
    else if (c == '&') {
      
    }
    else if (c == ':') {
      
    }
    else if (c == '[') {
      
    }
    else if (c == ']') {
      
    }
    else if (c == '{') {
      
    }
    else if (c == '}') {
      
    }
    
    else
    {
      // if we get here, then char denotes an UNKNOWN token:
      T.id = nuPy_UNKNOWN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }

  }

  //
  // execution should never get here, return occurs
  // from within loop
  //
}
