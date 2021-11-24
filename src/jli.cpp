/**
 * @file jli.c
 * @author Joe Wingbermuehle
 * @author Klaus Zerbe 
 * 
 * This is a REPL for interfacing with JL
 * - accesible via OS commandline of GPIO port (USB or UART) of RP2040
 *
 */

#ifdef RP2040
#include <pico/stdlib.h>
#endif

#include "jl.h"

#include <stdio.h>
#include <cstdlib>   // can't use C-stdlib.h for RP2040 SDK
#include <string.h>   // use C++ libs for RP2040 SDK

const uint startLineLength = 8; // the linebuffer will automatically grow for longer lines
const char eof = 255;           // EOF in stdio.h -is -1, but getchar returns int 255 to avoid blocking

#ifdef RP2040
#define LINE_END '\r'
#define DUPLEX true
#else 
#define LINE_END '\n'
#define DUPLEX false
#endif

/*
 *  read a line of any  length from stdio (can grow to memory available)
 *
 *  @param fullDuplex input will echo on entry (terminal mode) when false
 *  @param linebreak defaults to "\n", but "\r" may be needed for terminals
 *  @return entered line on heap - don't forget calling free() to get memory back
 */
static char * getLine(bool fullDuplex = DUPLEX, char lineBreak = LINE_END) {
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
         } else if (c == '\b' && (pPos > pStart)) {
            --pPos;
            if (fullDuplex) {
               printf("\b \b");               
            }
            continue;
         }

        if (fullDuplex) {
            putchar(c); // echo for fullDuplex terminals
        }

        if (--len == 0) { // allow larger buffer
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

    *pPos = '\0';   // set string end
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

#ifdef RP2040
   stdio_init_all();
#endif
   printf("Pico JL Interpreter v%d.%d\n", JL_VERSION_MAJOR, JL_VERSION_MINOR);
   printf("Type ^D to exit\n");

   context = JLCreateContext();
   JLDefineSpecial(context, "print", PrintFunc, NULL);

   for(;;) {
      printf("> "); 
      fflush(stdout);
      const ssize_t len = 0;

      line = getLine();
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
