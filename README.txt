SIMPLE LISP
===========

This aims to be a demonstration implementation of a minimal classic
Lisp, basically equivalent in power to McCarthy's 1960 CACM paper
original, but with a bit more modern feel.  Particularly, no macros
are supported at this time.

The language is specified in
  http://users.jyu.fi/~antkaij/opetus/okp/2017/lambda-calculus3.pdf

There are the following extensions to that language:

  (DEFINE (var def) ... (var def))

  binds each var to the corresponding def in the current environment.
  The definitions can be mutually recursive.

  (PRINT sexp)

  Prints the sexp to stdout followed by newline.

The implementation language is C99 and should compile on any modern C
compiler.  I have tested it using gcc 6.2.0 and clang 3.8.1 on Ubuntu
16.10.  I recommend using clang.

There is a single dependency to POSIX: lexer.c uses isatty, which is
not a standard C function, but is a part of POSIX.

A yacc is required.  I have tested byacc on Ubuntu 16.10.

Nonstandard libraries required (Ubuntu package name in parentheses)
  - GNU Readline (libreadline-dev) [BSD libedit may also work, not tested]
  - Boehm-Demers-Weiser garbage collector (libgc-dev)

Once you have clang, byacc, libreadline-dev and libgc-dev (or
equivalents on other systems) installed, just type "make" to the shell
command prompt.  If you use gcc, you need to edit the Makefile first.


By
Antti-Juhani Kaijanaho (antti-juhani.kaijanaho@jyu.fi)
University of Jyväskylä, Faculty of Information Technology
