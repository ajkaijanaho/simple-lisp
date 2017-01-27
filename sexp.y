
%{
#include <stdio.h>
#include <strings.h>

#include "data.h"
#include "lexer.h"
#include "printer.h"

#define YYSTYPE struct datum *
%}

%token ATOM

%%

script : /**/
       | script sexp              { print_sexp($2, stdout); putchar('\n'); }

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
