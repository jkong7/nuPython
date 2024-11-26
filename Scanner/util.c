/*util.c*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // strcpy, strcspn
#include <ctype.h>   // tolower

#include "util.h"


//
// panic
//
// Outputs the given error message to stdout and then
// exits the program with error code of -1. As a result
// this function never returns.
//
void panic(char* msg)
{
   printf("**PANIC\n");
   printf("**PANIC: %s\n", msg);
   printf("**PANIC\n");

   exit(-1);
}


//
// dupString
// 
// Duplicates the given string and returns a pointer
// to the copy.
// 
// NOTE: this function allocates memory for the copy,
// the caller takes ownership of the copy and must
// eventually free that memory.
//
char* dupString(char* s)
{
  if (s == NULL) panic("s is NULL (dupString)");

  //
  // be sure to include extra location for null terminator:
  //
  char* copy = (char*)malloc(sizeof(char) * (strlen(s) + 1));
  if (copy == NULL) panic("out of memory (dupString)");

  strcpy(copy, s);

  return copy;
}


//
// dupStrings
// 
// Given 2 strings, makes a copy by concatenating 
// them together, and returns the copy.
// 
// NOTE: this function allocates memory for the copy,
// the caller takes ownership of the copy and must
// eventually free that memory.
//
char* dupStrings(char* s1, char *s2)
{
  if (s1 == NULL) panic("s1 is NULL (dupStrings)");
  if (s2 == NULL) panic("s2 is NULL (dupStrings)");

  //
  // be sure to include extra location for null terminator:
  //
  char* copy = (char*)malloc(sizeof(char) * (strlen(s1) + strlen(s2) + 1));
  if (copy == NULL) panic("out of memory (dupStrings)");

  strcpy(copy, s1);
  strcat(copy, s2);

  return copy;
}


//
// dupAndStripEOLN
// 
// Duplicates the given string and returns a pointer
// to the copy; any EOLN characters (\n, \r, etc.)
// are also removed.
// 
// NOTE: this function allocates memory for the copy,
// the caller takes ownership of the copy and must
// eventually free that memory.
//
char* dupAndStripEOLN(char* s)
{
  if (s == NULL) panic("s is NULL (dupAndStripEOLN)");

  //
  // be sure to include extra location for null terminator:
  //
  char* copy = (char*)malloc(sizeof(char) * (strlen(s) + 1));
  if (copy == NULL) panic("out of memory (dupAndStripEOLN)");

  strcpy(copy, s);

  // delete EOL chars LF, CR, CRLF, LFCR, etc.
  copy[strcspn(copy, "\r\n")] = '\0';

  return copy;
}


//
// icmpStrings
//
// case-insensitive comparison of strings s1 and s2.
// Like strcmp, returns 0 if s1 == s2 and returns a
// non-zero value if s1 != s2.
// 
// Example: icmpStrings("apple", "APPLE") returns 0
//
int icmpStrings(char* s1, char* s2)
{
  if (s1 == NULL) panic("s1 is NULL (icmpStrings)");
  if (s2 == NULL) panic("s2 is NULL (icmpStrings)");

  size_t L1 = strlen(s1);
  size_t L2 = strlen(s2);
  if (L1 != L2)
    return 1;    // anything non-zero => not equal

  for (size_t i = 0; i < L1; i++)
  {
    if (s1[i] == s2[i])
      continue;
    //
    // we know the chars diff, but could just be case:
    //
    if (tolower(s1[i]) == tolower(s2[i]))
      continue;
    //
    // if get here, mis-match
    //
    return 1;    // anything non-zero => not equal
  }

  // if get here, match!
  return 0;
}
