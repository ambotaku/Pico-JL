/**
 * @file jl-func.h
 * @author Joe Wingbermuehle
 */

#include "jl.h"
#include "jl-func.h"
#include "jl-value.h"
#include "jl-context.h"
#include "jl-scope.h"

#include <stdio.h>
#include <cstdlib>
#include <cstring>

typedef struct InternalFunctionNode {
   const char *name;
   JLFunction function;
} InternalFunctionNode;

static char CheckCondition(JLContext *context, JLValue *value);
static void InvalidArgumentError(JLContext *context, JLValue *args);
static void TooManyArgumentsError(JLContext *context, JLValue *args);
static void TooFewArgumentsError(JLContext *context, JLValue *args);

static JLValue *CompareFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *AddFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *SubFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *MulFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *DivFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *ModFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *BitAndFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *BitOrFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *BitXorFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *BitNotFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *BitShiftLeftFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *BitShiftRightFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *AndFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *OrFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *NotFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *BeginFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *ConsFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *DefineFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *HeadFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *IfFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *LambdaFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *ListFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *RestFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *SubstrFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *ConcatFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *IsNumberFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *IsStringFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *IsListFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *IsNullFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *StrToIntFunc(JLContext *context, JLValue *args, void *extra);
static JLValue *IntToStrFunc(JLContext *context, JLValue *args, void *extra);
static char* itoa(NUMBER_TYPE num, int base);


static InternalFunctionNode INTERNAL_FUNCTIONS[] = {
   { "=",         CompareFunc    },
   { "!=",        CompareFunc    },
   { ">",         CompareFunc    },
   { ">=",        CompareFunc    },
   { "<",         CompareFunc    },
   { "<=",        CompareFunc    },
   { "+",         AddFunc        },
   { "-",         SubFunc        },
   { "*",         MulFunc        },
   { "/",         DivFunc        },
   { "%",       ModFunc        },
   { "and",       AndFunc        },
   { "or",        OrFunc         },
   { "not",       NotFunc        },
   { "&",         BitAndFunc     },
   { "|",         BitOrFunc      },
   { "^",         BitXorFunc     },
   { "~",         BitNotFunc     },
   { "<<",        BitShiftLeftFunc },
   { ">>",        BitShiftRightFunc },
   { "int",       StrToIntFunc   },
   { "str",       IntToStrFunc   },
   { "begin",     BeginFunc      },
   { "cons",      ConsFunc       },
   { "define",    DefineFunc     },
   { "head",      HeadFunc       },
   { "if",        IfFunc         },
   { "lambda",    LambdaFunc     },
   { "list",      ListFunc       },
   { "rest",      RestFunc       },
   { "substr",    SubstrFunc     },
   { "concat",    ConcatFunc     },
   { "number?",   IsNumberFunc   },
   { "string?",   IsStringFunc   },
   { "list?",     IsListFunc     },
   { "null?",     IsNullFunc     }
};
static size_t INTERNAL_FUNCTION_COUNT = sizeof(INTERNAL_FUNCTIONS)
                                      / sizeof(InternalFunctionNode);

char CheckCondition(JLContext *context, JLValue *value)
{
   JLValue *cond = JLEvaluate(context, value);
   char rc = 0;
   if(cond) {
      switch(cond->tag) {
      case JLVALUE_NUMBER:
         rc = cond->value.number != 0.0;
         break;
      case JLVALUE_LIST:
         rc = cond->value.lst != NULL;
         break;
      default:
         rc = 1;
         break;
      }
      JLRelease(context, cond);
   }
   return rc;
}

void InvalidArgumentError(JLContext *context, JLValue *args)
{
   Error(context, "invalid argument to %s", args->value.str);
}

void TooManyArgumentsError(JLContext *context, JLValue *args)
{
   Error(context, "too many arguments to %s", args->value.str);
}

void TooFewArgumentsError(JLContext *context, JLValue *args)
{
   Error(context, "too few arguments to %s", args->value.str);
}

JLValue *CompareFunc(JLContext *context, JLValue *args, void *extra)
{
   const char *op = args->value.str;
   JLValue *va = NULL;
   JLValue *vb = NULL;
   JLValue *result = NULL;
   char cond = 0;
   if(args->next == NULL || args->next->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   va = JLEvaluate(context, args->next);
   vb = JLEvaluate(context, args->next->next);
   if(va == NULL || vb == NULL || va->tag != vb->tag) {

      if(op[0] == '=') {
         cond = va == vb;
      } else if(op[0] == '!') {
         cond = va != vb;
      } else {
         InvalidArgumentError(context, args);
      }

   } else {

      /* Here we know that va and vb are not nil and are of the same type. */
      NUMBER_TYPE diff = 0;
      if(va->tag == JLVALUE_NUMBER) {
         diff = va->value.number - vb->value.number;
      } else if(va->tag == JLVALUE_STRING) {
         diff = strcmp(va->value.str, vb->value.str);
      } else {
         InvalidArgumentError(context, args);
      }

      if(op[0] == '=') {
         cond = diff == 0.0;
      } else if(op[0] == '!') {
         cond = diff != 0.0;
      } else if(op[0] == '<' && op[1] == 0) {
         cond = diff < 0.0;
      } else if(op[0] == '<' && op[1] == '=') {
         cond = diff <= 0.0;
      } else if(op[0] == '>' && op[1] == 0) {
         cond = diff > 0.0;
      } else if(op[0] == '>' && op[1] == '=') {
         cond = diff >= 0.0;
      }

   }

   if(cond) {
      result = JLDefineNumber(context, NULL, 1);
   }

   JLRelease(context, va);
   JLRelease(context, vb);
   return result;
}

#define DEFINE_BIT_ARITHMETIC(name, opr, init)\
JLValue *name(JLContext *context, JLValue *args, void *extra) {\
   JLValue *vp;\
   NUMBER_TYPE sum = init;\
   for(vp = args->next; vp; vp = vp->next) {\
      JLValue *arg = JLEvaluate(context, vp);\
      if(arg == NULL || arg->tag != JLVALUE_NUMBER) {\
         InvalidArgumentError(context, args);\
         JLRelease(context, arg);\
         return NULL;\
      }\
      sum opr arg->value.number;\
      JLRelease(context, arg);\
   }\
   return JLDefineNumber(context, NULL, sum);\
};

DEFINE_BIT_ARITHMETIC(AddFunc, +=, 0)
DEFINE_BIT_ARITHMETIC(BitAndFunc, &=, -1)
DEFINE_BIT_ARITHMETIC(BitOrFunc, |=, 0)
DEFINE_BIT_ARITHMETIC(BitXorFunc, ^=, 0)


JLValue *SubFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *vp = args->next;
   JLValue *arg = NULL;
   NUMBER_TYPE total = 0;

   arg = JLEvaluate(context, vp);
   if(arg == NULL || arg->tag != JLVALUE_NUMBER) {
      InvalidArgumentError(context, args);
      JLRelease(context, arg);
      return NULL;
   }
   total = arg->value.number;
   JLRelease(context, arg);

   for(vp = vp->next; vp; vp = vp->next) {
      arg = JLEvaluate(context, vp);
      if(arg == NULL || arg->tag != JLVALUE_NUMBER) {
         InvalidArgumentError(context, args);
         JLRelease(context, arg);
         return NULL;
      }
      total -= arg->value.number;
      JLRelease(context, arg);
   }

   return JLDefineNumber(context, NULL, total);
}

JLValue *MulFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *vp;
   NUMBER_TYPE product = 1;
   for(vp = args->next; vp; vp = vp->next) {
      JLValue *arg = JLEvaluate(context, vp);
      if(arg == NULL || arg->tag != JLVALUE_NUMBER) {
         InvalidArgumentError(context, args);
         JLRelease(context, arg);
         return NULL;
      }
      product *= arg->value.number;
      JLRelease(context, arg);
   }
   return JLDefineNumber(context, NULL, product);
}

#define DEFINE_DIV_ARITHMETIC(name, opr)\
JLValue *name(JLContext *context, JLValue *args, void *extra)\
{\
   JLValue *va = NULL;\
   JLValue *vb = NULL;\
   JLValue *result = NULL;\
   va = JLEvaluate(context, args->next);\
   if(va == NULL || va->tag != JLVALUE_NUMBER) {\
      InvalidArgumentError(context, args);\
      goto div_done;\
   }\
   vb = JLEvaluate(context, args->next->next);\
   if(vb == NULL || vb->tag != JLVALUE_NUMBER) {\
      InvalidArgumentError(context, args);\
      goto div_done;\
   }\
   if(args->next->next->next) {\
      TooManyArgumentsError(context, args);\
      goto div_done;\
   }\
   result = JLDefineNumber(context, NULL, va->value.number opr vb->value.number);\
div_done:\
   JLRelease(context, va);\
   JLRelease(context, vb);\
   return result;\
}

DEFINE_DIV_ARITHMETIC(DivFunc, /)
DEFINE_DIV_ARITHMETIC(ModFunc, %)
DEFINE_DIV_ARITHMETIC(BitShiftLeftFunc, <<)
DEFINE_DIV_ARITHMETIC(BitShiftRightFunc, >>)

JLValue *AndFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *vp;
   for(vp = args->next; vp; vp = vp->next) {
      if(!CheckCondition(context, vp)) {
         return NULL;
      }
   }
   return JLDefineNumber(context, NULL, 1);
}

JLValue *OrFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *vp;
   for(vp = args->next; vp; vp = vp->next) {
      if(CheckCondition(context, vp)) {
         return JLDefineNumber(context, NULL, 1);
      }
   }
   return NULL;
}

JLValue *NotFunc(JLContext *context, JLValue *args, void *extra)
{
   if(args->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next != NULL) {
      TooManyArgumentsError(context, args);
      return NULL;
   }
   if(!CheckCondition(context, args->next)) {
      return JLDefineNumber(context, NULL, 1);
   } else {
      return NULL;
   }
}

JLValue *BitNotFunc(JLContext *context, JLValue *args, void *extra)
{
   if(args->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next != NULL) {
      TooManyArgumentsError(context, args);
      return NULL;
   }
   JLValue *va = NULL;
   JLValue *result = NULL;
   va = JLEvaluate(context, args->next);
   if(va == NULL || va->tag != JLVALUE_NUMBER) {
      InvalidArgumentError(context, args);
      JLRelease(context, va);
      return NULL;
   }
   result = JLDefineNumber(context, NULL, ~va->value.number);
   return result;
}

JLValue *StrToIntFunc(JLContext *context, JLValue *args, void *extra) {
   if(args->next == NULL || args->next->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }

   JLValue *va = JLEvaluate(context, args->next);
   JLValue *vb = JLEvaluate(context, args->next->next);
   JLValue *result = NULL;
   char *pEnd = NULL;

   if (!va || va->tag != JLVALUE_STRING || !vb || vb->tag != JLVALUE_NUMBER) {
      InvalidArgumentError(context, args);
      JLRelease(context, va);
      return NULL;
   }

   result = JLDefineNumber(context, NULL, strtol(va->value.str, &pEnd, vb->value.number));
   return result;
}

JLValue *IntToStrFunc(JLContext *context, JLValue *args, void *extra) {
   if(args->next == NULL || args->next->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }

   JLValue *va = JLEvaluate(context, args->next);
   JLValue *vb = JLEvaluate(context, args->next->next);
   JLValue *result = NULL;
   char *pEnd = NULL;

   if (!va || va->tag != JLVALUE_NUMBER || !vb || vb->tag != JLVALUE_NUMBER) {
      InvalidArgumentError(context, args);
      JLRelease(context, va);
      return NULL;
   }
   
   result = CreateValue(context, NULL, JLVALUE_STRING);
   result->value.str = itoa(va->value.number, vb->value.number);
   return result;
}

JLValue *BeginFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *vp;
   JLValue *result = NULL;
   JLEnterScope(context);
   for(vp = args->next; vp; vp = vp->next) {
      JLRelease(context, result);
      result = JLEvaluate(context, vp);
   }
   JLLeaveScope(context);
   return result;
}

JLValue *ConsFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *head = NULL;
   JLValue *rest = NULL;
   JLValue *temp = NULL;
   JLValue *result = NULL;

   if(args->next == NULL || args->next->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   rest = JLEvaluate(context, args->next->next);
   if(rest != NULL && rest->tag != JLVALUE_LIST) {
      InvalidArgumentError(context, args);
      JLRelease(context, rest);
      return NULL;
   }

   temp = JLEvaluate(context, args->next);
   head = CopyValue(context, temp);
   JLRelease(context, temp);

   result = CreateValue(context, NULL, JLVALUE_LIST);
   if(rest) {
      head->next = rest->value.lst;
      JLRetain(context, rest->value.lst);
      JLRelease(context, rest);
   }
   result->value.lst = head;

   return result;
}

JLValue *DefineFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *vp = args->next;
   JLValue *result = NULL;
   if(vp == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(vp->tag != JLVALUE_VARIABLE) {
      InvalidArgumentError(context, args);
      return NULL;
   }
   result = JLEvaluate(context, vp->next);
   JLDefineValue(context, vp->value.str, result);
   return result;
}

JLValue *HeadFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *result = NULL;
   JLValue *vp = JLEvaluate(context, args->next);

   if(vp == NULL || vp->tag != JLVALUE_LIST) {
      InvalidArgumentError(context, args);
      goto head_done;
   }

   result = vp->value.lst;
   JLRetain(context, result);

head_done:

   JLRelease(context, vp);
   return result;
}

JLValue *IfFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *vp = args->next;
   if(CheckCondition(context, vp)) {
      return JLEvaluate(context, vp->next);
   } else if(vp->next) {
      return JLEvaluate(context, vp->next->next);
   }
   return NULL;
}

JLValue *LambdaFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *result;
   JLValue *scope;

   if(args->next == NULL || args->next->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }

   scope = CreateValue(context, NULL, JLVALUE_SCOPE);
   scope->value.scope = context->scope;
   context->scope->count += 1;

   result = CreateValue(context, NULL, JLVALUE_LAMBDA);
   result->value.lst = scope;
   result->value.lst->next = args->next;
   JLRetain(context, args->next);

   return result;
}

JLValue *ListFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *result = NULL;
   if(args->next) {
      result = CreateValue(context, NULL, JLVALUE_LIST);
      JLValue **item = &result->value.lst;
      JLValue *vp;
      for(vp = args->next; vp; vp = vp->next) {
         JLValue *arg = JLEvaluate(context, vp);
         JLValue *temp = CopyValue(context, arg);
         *item = temp;
         item = &temp->next;
         JLRelease(context, arg);
      }
   }
   return result;
}

JLValue *RestFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *result = NULL;
   JLValue *vp = JLEvaluate(context, args->next);

   if(vp == NULL || vp->tag != JLVALUE_LIST) {
      InvalidArgumentError(context, args);
      goto rest_done;
   }

   if(vp->value.lst && vp->value.lst->next) {
      result = CreateValue(context, NULL, JLVALUE_LIST);
      result->value.lst = vp->value.lst->next;
      JLRetain(context, result->value.lst);
   }

rest_done:

   JLRelease(context, vp);
   return result;
}

JLValue *SubstrFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *result   = NULL;
   JLValue *str      = NULL;
   JLValue *sval     = NULL;
   JLValue *lval     = NULL;
   size_t start      = 0;
   size_t len        = (size_t)-1;
   size_t slen;

   str = JLEvaluate(context, args->next);
   if(!str || str->tag != JLVALUE_STRING) {
      InvalidArgumentError(context, args);
      goto substr_done;
   }

   sval = JLEvaluate(context, args->next->next);
   if(sval) {
      if(sval->tag != JLVALUE_NUMBER) {
         InvalidArgumentError(context, args);
         goto substr_done;
      }
      start = (size_t)sval->value.number;
   }

   if(args->next->next) {
      if(args->next->next->next && args->next->next->next->next) {
         TooManyArgumentsError(context, args);
         goto substr_done;
      }
      lval = JLEvaluate(context, args->next->next->next);
      if(lval) {
         if(lval->tag != JLVALUE_NUMBER) {
            InvalidArgumentError(context, args);
            goto substr_done;
         }
         len = (size_t)lval->value.number;
      }
   }

   slen = strlen(str->value.str);
   if(start < slen && len > 0) {
      len = slen - start > len ? len : slen - start;
      result = CreateValue(context, NULL, JLVALUE_STRING);
      result->value.str = (char*)malloc(len + 1);
      memcpy(result->value.str, &str->value.str[start], len);
      result->value.str[len] = 0;
   }

substr_done:

   JLRelease(context, str);
   JLRelease(context, sval);
   JLRelease(context, lval);

   return result;

}

JLValue *ConcatFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *result = CreateValue(context, NULL, JLVALUE_STRING);
   JLValue *vp;
   size_t len = 0;
   size_t max_len = 8;
   result->value.str = (char*)malloc(max_len);
   for(vp = args->next; vp; vp = vp->next) {
      JLValue *arg = JLEvaluate(context, vp);
      if(arg == NULL || arg->tag != JLVALUE_STRING) {
         InvalidArgumentError(context, args);
         JLRelease(context, arg);
         JLRelease(context, result);
         return NULL;
      } else {
         const size_t l = strlen(arg->value.str);
         const size_t new_len = len + l;
         if(new_len >= max_len) {
            max_len = new_len + 1;
            result->value.str = (char*)realloc(result->value.str, max_len);
         }
         memcpy(&result->value.str[len], arg->value.str, l);
         len = new_len;
      }
      JLRelease(context, arg);
   }
   result->value.str[len] = 0;
   return result;
}

JLValue *IsNumberFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *arg = NULL;
   JLValue *result = NULL;

   if(args->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   arg = JLEvaluate(context, args->next);
   if(arg && arg->tag == JLVALUE_NUMBER) {
      result = JLDefineNumber(context, NULL, 1);
   }
   JLRelease(context, arg);
   return result;
}

JLValue *IsStringFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *arg = NULL;
   JLValue *result = NULL;

   if(args->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   arg = JLEvaluate(context, args->next);
   if(arg && arg->tag == JLVALUE_STRING) {
      result = JLDefineNumber(context, NULL, 1);
   }
   JLRelease(context, arg);
   return result;

}

JLValue *IsListFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *arg = NULL;
   JLValue *result = NULL;

   if(args->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   arg = JLEvaluate(context, args->next);
   if(arg && arg->tag == JLVALUE_LIST) {
      result = JLDefineNumber(context, NULL, 1);
   }
   JLRelease(context, arg);
   return result;
}

JLValue *IsNullFunc(JLContext *context, JLValue *args, void *extra)
{
   JLValue *arg = NULL;

   if(args->next == NULL) {
      TooFewArgumentsError(context, args);
      return NULL;
   }
   if(args->next->next) {
      TooManyArgumentsError(context, args);
      return NULL;
   }

   arg = JLEvaluate(context, args->next);
   if(arg == NULL) {
      return JLDefineNumber(context, NULL, 1);
   } else {
      JLRelease(context, arg);
      return NULL;
   }
}

void RegisterFunctions(JLContext *context)
{
   size_t i;
   for(i = 0; i < INTERNAL_FUNCTION_COUNT; i++) {
      JLDefineSpecial(context, INTERNAL_FUNCTIONS[i].name,
                      INTERNAL_FUNCTIONS[i].function, NULL);
   }
}

char* itoa(NUMBER_TYPE num, int base) {
   uint bufMaxSize = 16;
   char *pBuf = (char*) malloc(bufMaxSize);
   bool neg = false;
   char *pPos = pBuf;

   if (base == 10 && num <0) {
      num *= -1;
      neg = true;
   }
   for (unsigned rest = num; rest > 1; rest /= base) {
      if (pPos - pBuf >= bufMaxSize) {
         char *pNew = (char*) realloc(pBuf, bufMaxSize *= 2);
         pPos = pNew + (pPos - pBuf);
         pBuf = pNew; 
      }

      uint digit = rest % base;
      if (digit < 10) {
         *pPos++ = 0x30 + digit;
      } else {
         *pPos++ = 0x40 + digit - 9;
      }
   }

   if (neg) {
      *pPos++ = '-';
   }
   
   *pPos = '\0';
   // reverse string
   char c;
   for (int i = 0, j = strlen(pBuf)-1; i<j; i++, j--) {
      c = pBuf[i];
      pBuf[i] = pBuf[j];
      pBuf[j] = c;
   }

   return pBuf;
}

