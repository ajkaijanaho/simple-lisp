/* simple-lisp - A simple demonstration interpreter for a tiny Lisp-like language */
/* Copyright © 2017 Antti-Juhani Kaijanaho */

/*     Redistribution and use in source and binary forms, with or without */
/*     modification, are permitted provided that the following conditions */
/*     are met: */
 
/*      * Redistributions of source code must retain the above copyright */
/*        notice, this list of conditions and the following disclaimer. */
 
/*      * Redistributions in binary form must reproduce the above */
/*        copyright notice, this list of conditions and the following */
/*        disclaimer in the documentation and/or other materials provided */
/*        with the distribution. */
 
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS */
/*    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT */
/*    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS */
/*    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE */
/*    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, */
/*    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, */
/*    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; */
/*    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER */
/*    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT */
/*    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN */
/*    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE */
/*    POSSIBILITY OF SUCH DAMAGE. */
 
%{
#include <stdio.h>
#include <strings.h>

#include "data.h"
#include "eval.h"
#include "lexer.h"
#include "printer.h"

#define YYSTYPE struct datum *
%}

%token ATOM
%token INTERACTIVE NONINTERACTIVE

%%

input : /**/
      | input INTERACTIVE session
      | input NONINTERACTIVE script

session : /**/
        | session sexp { print_sexp(eval($2), stdout); putchar('\n'); }

script : /**/
       | script sexp   { eval($2); }

sexp : ATOM                       { $$ = $1; }
     | '(' ')'                    { $$ = make_NIL(); }
     | '(' sexp_list ')'          { $$ = $2; }
     | '\'' sexp                  { $$ = make_pair(make_QUOTE(),
                                                   make_pair($2, make_NIL())); }

sexp_list : sexp_list_init          { $$ = get_pair_first($1); }
          | sexp_list_init '.' sexp { set_pair_second(get_pair_second($1), $3);
                                      $$ = get_pair_first($1); }
            
sexp_list_init :
  sexp                { $$ = make_pair($1, make_NIL());
                        $$ = make_pair($$, $$); }
| sexp_list_init sexp { struct datum *n = make_pair($2, make_NIL());
                        set_pair_second(get_pair_second($1), n);
                        set_pair_second($1, n);
                        $$ = $1;
                       }
