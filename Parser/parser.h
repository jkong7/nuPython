/*parser.h*/


#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  

#include "tokenqueue.h"


//
// parser_parse
//
// Given an input stream, uses the scanner to obtain the tokens
// and then checks the syntax of the input against the BNF rules
// of the language 
struct TokenQueue* parser_parse(FILE* input);
