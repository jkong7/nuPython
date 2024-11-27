/*main.c*/


#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>   // strcspn


#include "token.h"    // token defs
#include "scanner.h"  // scanner
#include "util.h"     // panic




//
// main
//
int main(int argc, char* argv[])
{
  // Ask the user for a filename, if they don't enter one
  // then we'll take input from the keyboard:
  char filename[64]; 

  printf("Enter nuPython file (press ENTER to input from keyboard)>\n");
  
  fgets(filename, 64, stdin);  // read the user input, enter generates \n which automatically ends the input 
  filename[strcspn(filename, "\r\n")] = '\0';  // replace EOL char with \0 in filename string


  FILE* input = NULL;
  bool  keyboardInput = false;

  if (strlen(filename) == 0) { // aka only enter was pressed 
    // input from the keyboard (stdin)
    input = stdin;
    keyboardInput = true;
  }
  else {
    input = fopen(filename, "r"); // otherwise, a filename was provided, try to search and open it 

    if (input == NULL) // couldn't open file
    {
      printf("**ERROR: unable to open input file '%s' for input.\n", filename);
      return 0;
    }


    keyboardInput = false;
  }


  // provide an interactive i/o stream to print-visualize the scanner:
  // input the tokens, either from keyboard or the given nuPython 
  // file; the "input" variable controls the source. the scanner will
  // stop and return EOS when the user enters $ or we reach EOF on
  // the nuPython file:

  int lineNumber = -1;
  int colNumber = -1;
  char value[256] = "";
  struct Token T;


  // setup lineNumber, colNumber, and value to start scanning:
  scanner_init(&lineNumber, &colNumber, value);

  if (keyboardInput)  // take input from keyboard 
  {
    printf("nuPython input (enter $ when you're done)>\n");
  }


  // call scanner to process input token by token until we see ; or $
  T = scanner_nextToken(input, &lineNumber, &colNumber, value);

  // print tokens!!
  while (T.id != nuPy_EOS)
  {
    printf("Token %d ('%s') @ (%d, %d)\n", T.id, value, T.line, T.col);


    T = scanner_nextToken(input, &lineNumber, &colNumber, value);
  }


  // output that last token
  printf("Token %d ('%s') @ (%d, %d)\n", T.id, value, T.line, T.col);


  // done
  if (!keyboardInput)
    fclose(input);


  return 0;
}
