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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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
                prim_fun primitive;
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
struct datum *make_closure(struct datum *body,
                           struct datum *env)
{
        struct datum *rv = make_pair(body, env);
        rv->type = T_CLOSURE;
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
        
struct datum *make_primitive(prim_fun fun)
{
        struct datum *rv = GC_malloc(sizeof *rv);
        if (rv == 0) enomem();
        rv->type = T_PRIMITIVE;
        rv->u.primitive = fun;
        return rv;
}

static struct datum *make_symbolic_atom_reusing_name(const char *name,
                                                     size_t len)
{
        if (len == 3 && strncasecmp(name, "NIL", len) == 0) return make_NIL();
        struct datum *rv = GC_malloc(sizeof *rv);
        if (rv == 0) enomem();
        rv->type = T_SYMBOL;
        rv->u.symbol = name;
        return rv;
}

struct datum *make_symbolic_atom(const char *name, size_t len)
{
        if (len == 3 && strncasecmp(name, "NIL", len) == 0) return make_NIL();
        char * s = GC_malloc_atomic(len+1);
        if (s == 0) enomem();
        memcpy(s, name, len);
        s[len] = '\0';
        return make_symbolic_atom_reusing_name(s, len);
}

struct datum *make_symbolic_atom_cstr(const char *name)
{
        return make_symbolic_atom(name, strlen(name));
}

struct datum *make_error(struct datum *where, const char *fmt, ...)
{
        va_list ap, ap1;
        va_start(ap, fmt);
        va_copy(ap1, ap);
        int n = vsnprintf(NULL, 0, fmt, ap);
        if (n < 0) NOTREACHED;
        char *s = GC_malloc_atomic(n+1);
        if (s == NULL) enomem();
        int m = vsnprintf(s, n+1, fmt, ap1);
        if (m < 0) NOTREACHED;
        va_end(ap1);
        va_end(ap);

        struct datum *rv = GC_malloc(sizeof *rv);
        if (rv == NULL) enomem();
        rv->type = T_ERROR;
        rv->u.pair.first = make_symbolic_atom_reusing_name(s, n);
        rv->u.pair.second = make_pair(where, make_NIL());
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

struct datum *make_T(void) {
        struct datum *rv = GC_malloc(sizeof *rv);
        if (rv == 0) enomem();
        rv->type = T_SYMBOL;
        rv->u.symbol = "T";
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

struct datum *apply_primitive(struct datum *prim,
                              struct datum *arg)
{
        assert(prim->type == T_PRIMITIVE);
        return prim->u.primitive(arg);
}

struct datum *get_pair_first(struct datum *d)
{
        assert(d->type == T_PAIR || d->type == T_CLOSURE || d->type == T_ERROR);
        return d->u.pair.first;
}
struct datum *get_pair_second(struct datum *d)
{
        assert(d->type == T_PAIR || d->type == T_CLOSURE || d->type == T_ERROR);
        return d->u.pair.second;
}
 
struct list_data get_list_data(struct datum *d)
{
        size_t maxn = 0;
        size_t n = 0;
        struct datum **vec = NULL;
        while (d != NULL && d->type == T_PAIR) {
                if (maxn >= n) {
                        maxn = maxn == 0 ? 2 : 2*maxn;
                        vec = GC_realloc(vec, maxn * sizeof *vec);
                        if (vec == NULL) enomem();
                }
                vec[n++] = d->u.pair.first;
                d = d->u.pair.second;
        }
        return (struct list_data) { .n = n, .vec = vec, .terminator = d };
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

_Bool is_this_symbol(struct datum *d, const char *name)
{
        if (d == NULL) return strcasecmp("NIL", name) == 0;
        if (d->type != T_SYMBOL) return 0;
        return strcasecmp(d->u.symbol, name) == 0;
}

struct datum *make_deep_copy(struct datum *d)
{
        if (d == NULL) return NULL;
        switch (d->type) {
        case T_PAIR:
                return make_pair(make_deep_copy(d->u.pair.first),
                                 make_deep_copy(d->u.pair.second));
        case T_NUMBER:
                return make_numeric_atom(d->u.number);
        case T_SYMBOL:
                return make_symbolic_atom_reusing_name(d->u.symbol,
                                                       strlen(d->u.symbol));
        default:
                fprintf(stderr,
                        "Internal error in make_deep_copy (%d)",
                        d->type);
                exit(EXIT_FAILURE);
        }
}
