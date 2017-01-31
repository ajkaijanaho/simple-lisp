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
#include <string.h>
#include "ast.h"
#include "error.h"
#include "eval.h"

// env is assumed to be a (proper or imprpert) list of pairs; we find
// the first pair whose left component is the name symbol and return
// its right component;
// - returns  NIL if not found;
// - if an element of the list is not a pair, it is skipped
// - the list terminator is ignored
static struct datum *lookup(struct datum *env, const char *name) {
        for (struct datum *it = env;
             get_type(it) == T_PAIR;
             it = get_pair_second(it)) {
                struct datum *p = get_pair_first(it);
                if (get_type(p) != T_PAIR) continue;
                if (is_this_symbol(get_pair_first(p), name)) {
                        return get_pair_second(p);
                }
        }
        return make_NIL();
}

static struct datum *eval_datum(struct datum *d, struct datum *env);

static struct datum *apply(struct datum *fun,
                           struct datum *arg)
{
        switch (get_type(fun)) {
        case T_ERROR:
                NOTREACHED;
        case T_PAIR: case T_NUMBER: case T_SYMBOL:
                return make_error(make_pair(fun, arg),
                                  "ERROR: Cannot apply");
        case T_PRIMITIVE:
                return apply_primitive(fun, arg);
        case T_CLOSURE:
        {
                struct datum *env = get_pair_second(fun);
                struct abs_term *abs = term_as_abs_term(parse_sexp_as_term
                                                        (get_pair_first(fun)));
                struct datum *it = arg;
                for (size_t i = 0; i < abs->num_params; i++) {
                        if (get_type(it) != T_PAIR) {
                                return make_error(make_pair(fun, arg),
                                                  "Missing a parameter for %s",
                                                  abs->params[i]);
                        }
                        env = make_pair
                                (make_pair
                                 (make_symbolic_atom_cstr(abs->params[i]),
                                  get_pair_first(it)),
                                 env);
                        it = get_pair_second(it);
                }
                if (abs->rest_param_name != NULL) {
                        env = make_pair
                                (make_pair
                                 (make_symbolic_atom_cstr(abs->rest_param_name),
                                  it),
                                 env);
                } else if (!is_NIL(it)) {
                        return make_error(make_pair(fun, arg),
                                          "Too many parameters");
                }
                return eval_datum(abs->body, env);
        }
        }
        NOTREACHED;
}


static struct datum *eval_term(struct term *t,
                               struct datum *env)
{
        switch (get_term_type(t)) {
        case TT_OTHER:
                return make_error(get_original_sexp(t),
                                  "ERROR: Cannot evaluate");
        case TT_DATA:
                return get_original_sexp(t);
        case TT_VAR:
        {
                const char *name = term_as_var_term(t)->name;
                struct datum *def = lookup(env, name);
                if (is_NIL(def)) {
                        return make_error(get_original_sexp(t),
                                          "ERROR: Undefined variable");
                }
                return def;
        }
        case TT_APP:
        {
                struct app_term *at = term_as_app_term(t);
                struct datum *fun = eval_datum(at->left, env);
                if (get_type(fun) == T_ERROR) return fun;
                struct datum *arg = make_NIL();
                struct datum *last = make_NIL();
                struct datum *it;
                for (it = at->right;
                     get_type(it) == T_PAIR;
                     it = get_pair_second(it)) {
                        struct datum *res = eval_datum(get_pair_first(it), env);
                        if (get_type(res) == T_ERROR) return res;
                        struct datum *n = make_pair(res, make_NIL());
                        if (is_NIL(arg)) {
                                assert(is_NIL(last));
                                arg = n;
                                last = n;
                        } else {
                                assert(!is_NIL(last));
                                set_pair_second(last, n);
                                last = n;
                        }
                }
                if (!is_NIL(it)) {
                        struct datum *res = eval_datum(get_pair_first(it), env);
                        if (get_type(res) == T_ERROR) return res;
                        set_pair_second(last, res);
                }
                return apply(fun, arg);
        }
        case TT_MU:
        {
                struct mu_term *mt = term_as_mu_term(t);
                return eval_datum
                        (mt->body, make_pair(make_symbolic_atom_cstr(mt->var),
                                             mt->body));
        }
        case TT_ABS:
                return make_closure(get_original_sexp(t), env);
        }
        NOTREACHED;
}        

static struct datum *eval_datum(struct datum *d, struct datum *env)
{
        return eval_term(parse_sexp_as_term(d), env);
}
struct datum *eval(struct datum *d)
{
        return eval_datum(d, make_NIL());
}
