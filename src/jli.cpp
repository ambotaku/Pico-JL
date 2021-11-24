/**
 * @file jli.c
 * @authoer Joe Wingbermuehle
 *
 * This is a REPL for interfacing with JL on the command line.
 *
 */

#include <pico/stdlib.h>

#include "jl.h"

#include <stdio.h>
#include <cstdlib>
#include <string.h>

const uint startLineLength = 8; // the linebuffer will automatically grow for longer lines
const char eof = 255;           // EOF in stdio.h -is -1, but getchar returns int 255 to avoid blocking

static char * getLine(bool fullDuplex = true, char lineBreak = '\n') {
    // th line buffer
    // will allocated by pico_malloc module if <cstdlib> gets included
    char * pStart = (char*)malloc(startLineLength); 
    char * pPos = pStart;  // next character position
    size_t maxLen = startLineLength; // current max buffer size
    size_t len = maxLen; // current max length
    int c;

    if(!pStart) {
        return NULL; // out of memory or dysfunctional heap
    }

    while(1) {
        c = getchar(); // expect next character entry
        if(c == eof || c == lineBreak) {
            break;     // non blocking exit
        }

        if (fullDuplex) {
            putchar(c); // echo for fullDuplex terminals
        }

        if(--len == 0) { // allow larger buffer
            len = maxLen;
            // double the current line buffer size
            char *pNew  = (char*)realloc(pStart, maxLen *= 2);
            if(!pNew) {
                free(pStart);
                return NULL; // out of memory abort
            }
            // fix pointer for new buffer
            pPos = pNew + (pPos - pStart);
            pStart = pNew;
        }

        // stop reading if lineBreak character entered 
        if((*pPos++ = c) == lineBreak) {
            break;
        }
    }

    *pPos = '\0';   // set string end mark
    return pStart;
}


static struct JLValue *PrintFunc(struct JLContext *context,
                                 struct JLValue *args,
                                 void *extra)
{
   struct JLValue *vp;
   for(vp = JLGetNext(args); vp; vp = JLGetNext(vp)) {
      struct JLValue *result = JLEvaluate(context, vp);
      if(JLIsString(result)) {
         printf("%s", JLGetString(result));
      } else {
         JLPrint(context, result);
      }
      JLRelease(context, result);
   }
   return NULL;
}

static struct JLValue *ProcessBuffer(struct JLContext *context,
                                     const char *line)
{
   struct JLValue *result = NULL;
   while(*line) {
      struct JLValue *value = JLParse(context, &line);
      if(value) {
         JLRelease(context, result);
         result = JLEvaluate(context, value);
         JLRelease(context, value);
      }
   }
   return result;
}

int main(int argc, char *argv[])
{
   struct JLContext *context;
   struct JLValue *result;
   char *line = NULL;
   size_t cap = 0;
   char *filename = NULL;

   stdio_init_all();
   printf("Pico JL Interpreter v%d.%d\n", JL_VERSION_MAJOR, JL_VERSION_MINOR);
   printf("Type ^D to exit\n");

   context = JLCreateContext();
   JLDefineSpecial(context, "print", PrintFunc, NULL);

   for(;;) {
      printf("> "); 
      fflush(stdout);
      const ssize_t len = 0;

      line = getLine(true, '\r');
      if(strlen(line) > 0) {
         printf("\r\n");
         
         result = ProcessBuffer(context, line);
         free(line);

         printf("=> ");
         JLPrint(context, result);
         printf("\n");
         JLRelease(context, result);
      }
   }

   JLDestroyContext(context);
   return 0;
}
