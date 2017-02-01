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
 

#ifndef GUARD_DATA_H
#define GUARD_DATA_H

#include <stddef.h>
#include "config.h"

struct datum;
struct env;

// prim_fun is the name of the type of functions from struct datum *
// to struct datum *
typedef struct datum *(*prim_fun)(struct datum *);

enum data_type {
        T_PAIR,
        T_NUMBER,
        T_SYMBOL,
        // internal data types
        T_ERROR,
        T_PRIMITIVE,
        T_CLOSURE,
};

struct datum *make_pair(struct datum *, struct datum *);
struct datum *make_numeric_atom(double);
struct datum *make_symbolic_atom(const char *name, size_t len);
struct datum *make_symbolic_atom_cstr(const char *name);
FORMAT(struct datum *make_error(struct datum *where, const char *fmt,
                                ...), printf, 2, 3);
struct datum *make_primitive(prim_fun fun);
struct datum *make_closure(struct datum *body,
                           struct env *env);
struct datum *make_T(void);
struct datum *make_NIL(void);
struct datum *make_QUOTE(void);

struct datum *make_deep_copy(struct datum *);

enum data_type get_type(struct datum *);
_Bool is_NIL(struct datum *);

struct datum *apply_primitive(struct datum *prim,
                              struct datum *arg);

struct datum *get_pair_first(struct datum *);
struct datum *get_pair_second(struct datum *);

struct datum *get_closure_fun(struct datum *);
struct env *get_closure_env(struct datum *);

struct list_data {
        size_t n;
        struct datum **vec;
        struct datum *terminator;
};
struct list_data get_list_data(struct datum *);

void set_pair_second(struct datum *pair, struct datum *replacement);

double get_numeric_value(struct datum *);

const char *get_symbol_name(struct datum *);
_Bool is_this_symbol(struct datum *, const char *);

#endif /* GUARD_DATA_H */
