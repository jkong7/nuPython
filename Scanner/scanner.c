/*scanner.c*/

#include <stdio.h>
#include <stdbool.h>  // true, false
#include <ctype.h>    // isspace, isdigit, isalpha
#include <string.h>   // strcmp
#include <assert.h>   // assert

#include "util.h"
#include "scanner.h"


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


// SCANNER HELPERS: 


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
  value[i] = '\0'; 

  return;
}

//
// collect_string_literal
//
// Given the start of a string literal, collects the entire string, 
// prints termination error if there is a quote mismatch or there is 
// no termination (new line or EOF)
//
static void collect_string_literal(FILE* input, int c, int* lineNumber, int* colNumber, char* value) 
{
  assert(c == '\'' || c == '"'); // c should be start of string literal 

  int i = 0; 

  char start = c; // mark which quote c is at the start

  int col = *colNumber; 

  while (true) {
    c = fgetc(input); 

    // new line or EOF, string wasn't terminated properly 
    if (c == EOF || c == '\n') {
      printf("**WARNING: string literal @ (%d, %d) not terminated properly\n", *lineNumber, col); 
      ungetc(c, input); // push back new line or EOF 
      break; 
    }

    // string was terminated properly (consume quote, don't push back!)
    if (c == start) {
      (*colNumber)++; 
      break; 
    }

    // quote mismatch, string wasn't terminated properly 
    if (c == '\'' || c=='"') {
      printf("**WARNING: string literal @ (%d, %d) not terminated properly\n", *lineNumber, col); 
      ungetc(c, input); // push back the mismatched quote (this can be the start of another quote, don't consume now)
      break; 
    }

    // store string literal 
    value[i]=(char)c; 
    i++; 
    (*colNumber)++; 
  }

  value[i]='\0'; 
  return; 
}

//
// collect_int_or_real_literal
//
// Given the start of a string literal, collects the entire string, 
// prints termination error if there is a quote mismatch or there is 
// no termination (new line or EOF)
//
static void collect_int_or_real_literal(FILE* input, int c, int* colNumber, char* value, int* type, bool proceeding) {
  assert (isdigit(c)); //c should be start of int or real literal 


  int i = (proceeding) ? 1 : 0; // this function is called either after a + or -, or just starting from a digit, use proceeding bool input 
                                // to determine which case it is, if it is proceeding a + or -, i starts at 1 since value[0] is already filled with the sign

  while (true) {
    if (c == '.') {
      *type=1; // real literal, so let caller know 
      value[i]=(char)c; // consume and advance past .
      i++; 
      (*colNumber)++; 
      c=fgetc(input); 
      while (isdigit(c)) { // collect digits to the right of .
        value[i]=(char)c; 
        i++; 
        (*colNumber)++; 
        c=fgetc(input);  
      }
      break; 
    }
    if (!isdigit(c)) {
      break; 
    }
    value[i]=(char)c; 
    i++; 
    (*colNumber)++; 
    c=fgetc(input); 
  }
  ungetc(c, input); // push back c (went beyond last digit)
  value[i]='\0'; 
  return; 
}

// SCANNER: 

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

      char* keywords[] = {"and", "break", "continue", "def", "elif", "else", "False", "for", "if", 
                          "in", "is", "None", "not", "or", "pass", "return", "True", "while"};

      int keywordTokens[] = {nuPy_KEYW_AND, nuPy_KEYW_BREAK, nuPy_KEYW_CONTINUE, nuPy_KEYW_DEF, nuPy_KEYW_ELIF, 
                            nuPy_KEYW_ELSE, nuPy_KEYW_FALSE, nuPy_KEYW_FOR, nuPy_KEYW_IF, nuPy_KEYW_IN, 
                            nuPy_KEYW_IS, nuPy_KEYW_NONE, nuPy_KEYW_NOT, nuPy_KEYW_OR, nuPy_KEYW_PASS, 
                            nuPy_KEYW_RETURN, nuPy_KEYW_TRUE, nuPy_KEYW_WHILE};     

      for (int i=0; i<18; i++) {
        if (strcmp(value, keywords[i])==0) {
          T.id=keywordTokens[i]; 
          break; 
        }
      }
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
    } 
    else if (c == '+') 
    {
      T.id = nuPy_PLUS; 
      T.line = *lineNumber; 
      T.col = *colNumber; 

      (*colNumber)++; 

      value[0]=(char)c; 
      value[1]='\0'; 

      c = fgetc(input); 

      if (isdigit(c)) {
        int type = 0; 
        collect_int_or_real_literal(input, c, colNumber, value, &type, true); 
        if (type == 0) {
          T.id = nuPy_INT_LITERAL; 
        } else {
          T.id = nuPy_REAL_LITERAL; 
        }
      }

      return T; 
    }
    else if (c == '-') 
    {
      T.id = nuPy_MINUS; 
      T.line = *lineNumber; 
      T.col = *colNumber; 

      (*colNumber)++;

      value[0]=(char)c; 
      value[1]='\0'; 

      c=fgetc(input); 

      if (isdigit(c)) {
        int type = 0; 
        collect_int_or_real_literal(input, c, colNumber, value, &type, true); 
        if (type == 0) {
          T.id = nuPy_INT_LITERAL; 
        } else {
          T.id = nuPy_REAL_LITERAL; 
        }
      }

      return T; 
    }
    else if (c == '/') 
    {
      T.id = nuPy_SLASH; 
      T.line = *lineNumber; 
      T.col = *colNumber; 

      (*colNumber)++;

      value[0]=(char)c; 
      value[1]='\0'; 

      return T; 
    } 
    else if (c == '%') 
    {
      T.id = nuPy_PERCENT; 
      T.line = *lineNumber; 
      T.col = *colNumber; 

      (*colNumber)++;

      value[0]=(char)c; 
      value[1]='\0'; 

      return T; 
    } 
    else if (c == '=') // handles = and ==
    {
      T.id=nuPy_EQUAL; 
      T.line=*lineNumber; 
      T.col=*colNumber; 

      (*colNumber)++;

      value[0]=(char)c; 
      value[1]='\0'; 

      c=fgetc(input); 

      if (c == '=') {
        T.id=nuPy_EQUALEQUAL; 
        (*colNumber)++;
        value[1]=(char)c; 
        value[2]='\0'; 
        return T; 
      }
      ungetc(c, input); 
      return T; 
    }
    else if (c == '!') {
      T.id=nuPy_NOTEQUAL; 
      T.line=*lineNumber; 
      T.col=*colNumber; 

      (*colNumber)++; 

      value[0]=(char)c; 
      value[1]='\0'; 

      c=fgetc(input); 

      if (c == '=') {
        (*colNumber)++;
        value[1]=(char)c; 
        value[2]='\0'; 
        return T; 
      }

      ungetc(c, input); 
      T.id=nuPy_UNKNOWN; 
      return T; 
    }
    else if (c == '<') //handles < and <=
    {
      T.id=nuPy_LT; 
      T.line=*lineNumber; 
      T.col=*colNumber; 

      (*colNumber)++;

      value[0]=(char)c; 
      value[1]='\0'; 

      c=fgetc(input); 

      if (c == '=') {
        T.id=nuPy_LTE; 
        (*colNumber)++;
        value[1]=(char)c; 
        value[2]='\0'; 
        return T; 
      }
      ungetc(c, input); 
      return T; 
    }
    else if (c == '>') //handles > and >=
    {
      T.id=nuPy_GT; 
      T.line=*lineNumber; 
      T.col=*colNumber; 

      (*colNumber)++;

      value[0]=(char)c; 
      value[1]='\0'; 

      c=fgetc(input); 

      if (c == '=') {
        T.id=nuPy_GTE; 
        (*colNumber)++; 
        value[1]=(char)c; 
        value[2]='\0'; 
        return T; 
      }
      ungetc(c, input); 
      return T; 
    }
    else if (c == '&') 
    {
      T.id = nuPy_AMPERSAND; 
      T.line = *lineNumber; 
      T.col = *colNumber; 

      (*colNumber)++; 

      value[0]=(char)c; 
      value[1]='\0'; 

      return T; 
    }
    else if (c == ':') 
    {
      T.id = nuPy_COLON; 
      T.line = *lineNumber; 
      T.col = *colNumber; 

      (*colNumber)++; 

      value[0]=(char)c; 
      value[1]='\0'; 

      return T; 
    }
    else if (c == '[') 
    {
      T.id = nuPy_LEFT_BRACKET; 
      T.line = *lineNumber; 
      T.col = *colNumber; 

      (*colNumber)++; 

      value[0]=(char)c; 
      value[1]='\0'; 

      return T; 
    }
    else if (c == ']') 
    {
      T.id = nuPy_RIGHT_BRACKET; 
      T.line = *lineNumber; 
      T.col = *colNumber; 

      (*colNumber)++; 

      value[0]=(char)c; 
      value[1]='\0'; 

      return T; 
    }
    else if (c == '{') 
    {
      T.id = nuPy_LEFT_BRACE; 
      T.line = *lineNumber; 
      T.col = *colNumber; 

      (*colNumber)++; 

      value[0]=(char)c; 
      value[1]='\0'; 

      return T; 
    }
    else if (c == '}') 
    {
      T.id = nuPy_RIGHT_BRACE; 
      T.line = *lineNumber; 
      T.col = *colNumber; 

      (*colNumber)++; 

      value[0]=(char)c; 
      value[1]='\0'; 

      return T; 
    }
    else if (c == '\'' || c == '"') { // string literal
      T.id = nuPy_STR_LITERAL; 
      T.line = *lineNumber; 
      T.col = *colNumber; 

      collect_string_literal(input, c, lineNumber, colNumber, value); // collect the string literal value

      return T; 
    }
    else if (isdigit(c)) { // int or real literal 
      int type = 0; 
      T.line = *lineNumber; 
      T.col = *colNumber; 
      collect_int_or_real_literal(input, c, colNumber, value, &type, false); // collect the int or real value
      if (type == 0) { //Either an int or real literal 
        T.id = nuPy_INT_LITERAL; 
      } else {
        T.id = nuPy_REAL_LITERAL; 
      }
      return T; 
    }
    else if (c == '#') {
      (*lineNumber)++; 
      *colNumber = 1; 
      while (c != '\n' && c != EOF) {
        c = fgetc(input); 
      }
      continue; 
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
  // execution should never get here, return occurs
  // from within loop
}
