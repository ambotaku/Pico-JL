
Pico-JL
==============================================================================
This is a fork of Joe Wingbermühle's JL interpreter for Raspberry Pi Pico.

The Raspberry Pi Pico is mostly used with a powerful (for that kind of device) MicroPython interpreter, but I think a Lisp (or Scheme) interpreter like JL allows evaluation of more programming paradigms and solutions where Python is reaching some limits or gets uncomfortable at least.

Joe is calling JL a configuration language and that is exactly my purpose, too.
But what I need is something to configure and control the 26 GPIO-ports and communication
channels (serial, parallel, i2c, SPI) of that microcontroller.

For that purpose I will replace or add decimal number support for 32/64 bit integers and all needed logical
bit operators, shifts etc. Unfortunately the Pico SDK does not support a filesystem for the 2 MB Flash
free flash memory, but maybe I can add file acces via SPI to a SD-card with FAT filesystem in future.
Until that, any applications need to be uploaded & downloaded via serial.

I needed to rearrange Joe's code basis (now complete in src-folder and CMake based) and port it to C++ (somewhat), because the Pico SDK supports no C standard-libraries, especially malloc, realloc and free from stdlib.h don't work and stdio.h is missing line input functions (e.g. getline), but my current commit got JL to work on the Pico at least.

So Joe's following language description is the current state for RP2040 microcontrollers, too.

---------------------------------------------------------------------

This is a small, embeddable LISP-like language.  The intended use is for
configuration files where it is desirable to be able to have complex
configurations (JWM, for example).

This is still a work in progress: there are still more functions to
be implemented and the functionality of existing functions may change.

Syntax
------------------------------------------------------------------------------
Like other LISP languages, JL uses s-expressions.  For example:

  (list 1 2 3)

calls the "list" function, passing 1, 2, and 3 as arguments.

Data Types
------------------------------------------------------------------------------
There are 6 data types:

 1. Numbers (floating point numbers)
 2. Strings
 3. Variables
 4. Lambdas (functions defined within the language)
 5. Lists
 6. Special functions

For comparisons, 0 and nil (the empty list) are considered false and all
other values are considered true.

Functions
------------------------------------------------------------------------------
The following built-in functions are available:

 - <        Test if less than
 - >        Test if greater than
 - <=       Test if less than or equal to
 - >=       Test if greater than or equal to
 - =        Test if equal
 - !=       Test if not equal
 - +        Return the sum of a list
 - -        Subtract
 - *        Return the produce of a list
 - /        Divide
 - %        Modulus
 - and      Logical AND.
 - concat   Concatenate strings.
 - cons     Prepend an item to a list.
 - begin    Execute a sequence of functions, return the value of the last.
 - define   Insert a binding into the current namespace.
 - head     Return the first element of a list
 - if       Test a condition and evaluate and return the second argument
            if true, otherwise evaluate and return the third argument.
 - lambda   Declare a function.
 - list     Create a list
 - list?    Determine if a value is a list.
 - not      Logical NOT.
 - null?    Determine if a value is nil.
 - number?  Determine if a value is a number.
 - or       Logical OR.
 - rest     Return all but the first element of a list
 - string?  Determine if a value is a string.
 - substr   Return a substring of a string.

Examples
------------------------------------------------------------------------------
Here are some example programs.  See the "examples" directory for more.

Return the factorial of a number:
<code><pre>
   (define fact (lambda (n)
      (if n
         (\* (fact (- n 1)) n)
         1)))
   (fact 5)
</pre></code>

Find the nth item of a list:
<code><pre>
   (define nth (lambda (n lst)
      (if (<= n 1)
         (head lst)
         (nth (- n 1) (rest lst)))))
   (nth 2 (list 1 2 3))
</pre></code>

Find nth Fibonacci number:
<code><pre>
   (define fib (lambda (n)
      (if (> n 1)
         (+ (fib (- n 1)) (fib (- n 2)))
         1)))
   (fib 10)
</pre></code>

The map function:
<code><pre>
   (define map (lambda (f lst)
      (if lst
         (cons (f (head lst)) (map f (rest lst)))
         (list))))
   (map (lambda (x) (+ x 1)) (list 1 2 3 4))
</pre></code>

The foldl function:
<code><pre>
   (define foldl (lambda (f i lst)
      (if lst
         (foldl f (f i (head lst)) (rest lst))
         i)))
   (foldl (lambda (a b) (+ a b)) 0 (list 1 2 3 4))
</pre></code>

The reverse function implemented in terms of foldl:
<code><pre>
   (define reverse (lambda (lst) (foldl (lambda (a b) (cons b a)) (list) lst)))
   (reverse (list 1 2 3 4))
</pre></code>

License
------------------------------------------------------------------------------
JL uses the BSD 2-clause license.  See LICENSE for more information.

