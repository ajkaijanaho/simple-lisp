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

#include <assert.h>
#include <gc.h>
#include <string.h>
#include <strings.h>

#include "data.h"
#include "error.h"

struct datum {
        enum data_type type;
        union {
                struct {
                        struct datum *first;
                        struct datum *second;
                } pair;
                double number;
                const char *symbol;
        } u;
};

struct datum *make_pair(struct datum *first, struct datum *second)
{
        struct datum *rv = GC_malloc(sizeof *rv);
        if (rv == 0) enomem();
        rv->type = T_PAIR;
        rv->u.pair.first = first;
        rv->u.pair.second = second;
        return rv;
}
struct datum *make_numeric_atom(double val)
{
        struct datum *rv = GC_malloc(sizeof *rv);
        if (rv == 0) enomem();
        rv->type = T_NUMBER;
        rv->u.number = val;
        return rv;
}
        
struct datum *make_symbolic_atom(const char *name, size_t len)
{
        if (len == 3 && strncasecmp(name, "NIL", len) == 0) return make_NIL();
        struct datum *rv = GC_malloc(sizeof *rv);
        if (rv == 0) enomem();
        rv->type = T_SYMBOL;
        char * s = GC_malloc_atomic(len+1);
        if (s == 0) enomem();
        memcpy(s, name, len);
        s[len] = '\0';
        rv->u.symbol = s;
        return rv;
}

struct datum *make_NIL(void) {
        return NULL;
}

struct datum *make_QUOTE(void) {
        struct datum *rv = GC_malloc(sizeof *rv);
        if (rv == 0) enomem();
        rv->type = T_SYMBOL;
        rv->u.symbol = "QUOTE";
        return rv;
}

_Bool is_NIL(struct datum *d)
{
        return d == NULL;
}

enum data_type get_type(struct datum *d)
{
        if (d == NULL) return T_SYMBOL;
        return d->type;
}

struct datum *get_pair_first(struct datum *d)
{
        assert(d->type == T_PAIR);
        return d->u.pair.first;
}
struct datum *get_pair_second(struct datum *d)
{
        assert(d->type == T_PAIR);
        return d->u.pair.second;
}

void set_pair_second(struct datum *d, struct datum *replacement)
{
        assert(d->type == T_PAIR);
        d->u.pair.second = replacement;
}

double get_numeric_value(struct datum *d)
{
        assert(d->type == T_NUMBER);
        return d->u.number;
}

const char *get_symbol_name(struct datum *d)
{
        if (d == NULL) return "NIL";
        assert(d->type == T_SYMBOL);
        return d->u.symbol;
}
