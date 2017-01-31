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

#include <strings.h>
#include "error.h"
#include "printer.h"

void print_sexp(struct datum *d, FILE *fp)
{
        switch (get_type(d)) {
        case T_CLOSURE:
                fputs("#<closure>", fp);
                goto pair;
        case T_ERROR:
                fputs("#<error>", fp);
                goto pair;
        pair: case T_PAIR:
                fputc('(', fp);
                print_sexp(get_pair_first(d), fp);
                d = get_pair_second(d);
                while (get_type(d) == T_PAIR) {
                        fputc(' ', fp);
                        print_sexp(get_pair_first(d), fp);
                        d = get_pair_second(d);
                }
                if (!is_NIL(d)) {
                        fputs(" . ", fp);
                        print_sexp(d, fp);
                }
                fputc(')', fp);
                return;
        case T_NUMBER:
                fprintf(fp, "%lg", get_numeric_value(d));
                return;
        case T_SYMBOL:
                fputs(get_symbol_name(d), fp);
                return;
        case T_PRIMITIVE:
                fputs("#<primitive>", fp);
        }
        NOTREACHED;
}
