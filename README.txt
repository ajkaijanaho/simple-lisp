SIMPLE LISP
===========

This aims to be a demonstration implementation of a minimal classic
Lisp, basically equivalent in power to McCarthy's 1960 CACM paper
original, but with a bit more modern feel.  Particularly, no macros
are supported at this time.

The language is specified in
  http://users.jyu.fi/~antkaij/opetus/okp/2017/lambda-calculus3.pdf

The implementation language is C99 and should compile on any modern C
compiler.  I have tested it using gcc 6.2.0 on Ubuntu 16.10.

A yacc is required.  I have tested byacc on Ubuntu 16.10.

Nonstandard libraries required (Ubuntu package name in parentheses)
  - GNU Readline (libreadline-dev) [BSD libedit may also work, not tested]
  - Boehm-Demers-Weiser garbage collector (libgc-dev)

Once you have gcc, byacc, libreadline-dev and libgc-dev (or
equivalents on other systems) installed, just type "make" to the shell
command prompt.


By
Antti-Juhani Kaijanaho (antti-juhani.kaijanaho@jyu.fi)
University of Jyväskylä, Faculty of Information Technology
