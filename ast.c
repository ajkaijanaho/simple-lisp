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

#include "ast.h"
#include "error.h"
#include "strvec.h"

struct term {
        enum term_type type;
        struct datum *orig;
        union {
                struct var_term var;
                struct data_term data;
                struct app_term app;
                struct abs_term abs;
                struct mu_term mu;
                struct guarded_term *guarded;
        } u;
};

static struct term *new_term(enum term_type tt, struct datum *d)
{
        struct term *rv = GC_malloc(sizeof *rv);
        if (rv == NULL) enomem();
        rv->type = tt;
        rv->orig = d;
        return rv;
}

static struct term *parse_list(struct datum *d)
{
        struct datum *head = get_pair_first(d);
        struct datum *rest = get_pair_second(d);

        if (get_type(head) == T_SYMBOL) {
                if (is_this_symbol(head, "QUOTE")) {
                        struct term *rv = new_term(TT_DATA, d);
                        rv->u.data.d = rest;
                        return rv;
                }
                if (is_this_symbol(head, "LABEL")) {
                        struct list_data restd = get_list_data(rest);
                        if (restd.n != 2 || !is_NIL(restd.terminator)) {
                                return new_term(TT_OTHER, d);
                        }
                        struct datum *var = restd.vec[0];
                        if (get_type(var) != T_SYMBOL) {
                                return new_term(TT_OTHER, d);
                        }
                        struct term *rv = new_term(TT_MU, d);
                        rv->u.mu.var = get_symbol_name(var);
                        rv->u.mu.body = restd.vec[1]; 
                        return rv;
                }
                if (is_this_symbol(head, "LAMBDA")) {
                        struct list_data restd = get_list_data(rest);
                        if (restd.n != 2 || !is_NIL(restd.terminator)) {
                                return new_term(TT_OTHER, d);
                        }
                        struct term *rv = new_term(TT_ABS, d);
                        rv->u.abs.body = restd.vec[1];
                        // parse the parameter spec:
                        // ( params . rest_param )
                        struct str_vec *sv = str_vec_new();
                        struct datum *vd = restd.vec[0];
                        while (get_type(vd) == T_PAIR) {
                                struct datum *var = get_pair_first(vd);
                                if (get_type(var) != T_SYMBOL) {
                                        return new_term(TT_OTHER, d);
                                }
                                str_vec_append(sv, get_symbol_name(var));
                                vd = get_pair_second(vd);
                        }
                        rv->u.abs.num_params = str_vec_len(sv);
                        rv->u.abs.params = str_vec_to_array(sv);
                        if (is_NIL(vd)) {
                                rv->u.abs.rest_param_name = NULL;
                        } else if (get_type(vd) == T_SYMBOL) {
                                rv->u.abs.rest_param_name = get_symbol_name(vd);
                        } else {
                                return new_term(TT_OTHER, d);
                        }
                        return rv;
                }
                if (is_this_symbol(head, "COND")) {
                        struct guarded_term *first = NULL;
                        struct guarded_term *last = NULL;
                        struct datum *it;
                        for (it = rest;
                             get_type(it) == T_PAIR;
                             it = get_pair_second(it)) {
                                struct datum *p = get_pair_first(it);
                                struct list_data pd = get_list_data(p);
                                if (pd.n != 2 || !is_NIL(pd.terminator)) {
                                        return new_term(TT_OTHER, d);
                                }
                                struct guarded_term *gt = GC_malloc(sizeof *gt);
                                if (gt == NULL) enomem();
                                gt->guard = pd.vec[0];
                                gt->term = pd.vec[1];
                                gt->next = NULL;
                                if (first == NULL) {
                                        assert(last == NULL);
                                        first = gt;
                                } else {
                                        assert(last != NULL);
                                        last->next = gt;
                                }
                                last = gt;
                        }
                        if (!is_NIL(it)) return new_term(TT_OTHER, d);
                        struct term *rv = new_term(TT_GUARDED, d);
                        rv->u.guarded = first;
                        return rv;
                }
        }
        struct term *rv = new_term(TT_APP, d);
        rv->u.app.left = head;
        rv->u.app.right = rest;
        return rv;
}

struct term *parse_sexp_as_term(struct datum *d)
{
        struct term *rv;
        switch (get_type(d)) {
        case T_ERROR: case T_PRIMITIVE: case T_NUMBER: case T_CLOSURE:
                rv = new_term(TT_DATA, d);
                rv->u.data.d = d;
                return rv;
        case T_SYMBOL:
                rv = new_term(TT_VAR, d);
                rv->u.var.name = get_symbol_name(d);
                return rv;
        case T_PAIR:
                return parse_list(d);
        }
        NOTREACHED;
}

enum term_type get_term_type(struct term *t)
{
        return t->type;
}

struct datum *get_original_sexp(struct term *t)
{
        return t->orig;
}


struct data_term *term_as_data_term(struct term *t)
{
        assert(t->type == TT_DATA);
        return &t->u.data;
}

struct var_term *term_as_var_term(struct term *t)
{
        assert(t->type == TT_VAR);
        return &t->u.var;
}

struct app_term *term_as_app_term(struct term *t)
{
        assert(t->type == TT_APP);
        return &t->u.app;
}

// defined for TT_ABS
struct abs_term *term_as_abs_term(struct term *t)
{
        assert(t->type == TT_ABS);
        return &t->u.abs;
}

// defined for TT_MU
struct mu_term *term_as_mu_term(struct term *t)
{
        assert(t->type == TT_MU);
        return &t->u.mu;
}

// defined for TT_GUARDED
struct guarded_term *term_as_guarded_term(struct term *t)
{
        assert(t->type == TT_GUARDED);
        return t->u.guarded;
}
