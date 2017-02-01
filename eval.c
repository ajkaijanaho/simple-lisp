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
#include "env.h"
#include "eval.h"
#include "primops.h"


static struct datum *eval_datum(struct datum *d, struct env *env);

/* Applies the given function to the given argument.

   The function is assumed to be a fully evaluated datum, and the
   argument is assumed to be a list of fully evaluated data.  The
   function will thus not be source code (lambda or like) but a
   T_PRIMITIVE or T_CLOSURE value.

   The result is the result value of the function applied to the
   argument.
 */
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
                struct env *env = env_clone(get_closure_env(fun));
                struct abs_term *abs = term_as_abs_term(parse_sexp_as_term
                                                        (get_closure_fun(fun)));
                struct datum *it = arg;
                for (size_t i = 0; i < abs->num_params; i++) {
                        if (get_type(it) != T_PAIR) {
                                return make_error(make_pair(fun, arg),
                                                  "Missing a parameter for %s",
                                                  abs->params[i]);
                        }
                        env_bind(env, abs->params[i], get_pair_first(it));
                        it = get_pair_second(it);
                }
                if (abs->rest_param_name != NULL) {
                        env_bind(env, abs->rest_param_name, it);
                } else if (!is_NIL(it)) {
                        return make_error(make_pair(fun, arg),
                                          "Too many parameters");
                }
                return eval_datum(abs->body, env);
        }
        }
        NOTREACHED;
}


/*  Evaluates a Lisp term, which has already been parsed using
    parse_sexp_as_term (in ast.h and ast.c), in the environment given.
    The result is a Lisp datum representing the value of the term.
 */
static struct datum *eval_term(struct term *t,
                               struct env *env)
{
        switch (get_term_type(t)) {
        case TT_OTHER:
                // TT_OTHER indicates that this is not a term as we
                // understand it here
                return make_error(get_original_sexp(t),
                                  "ERROR: Cannot evaluate");
        case TT_DATA:
                // TT_DATA evaluates to itself
                return get_original_sexp(t);
        case TT_VAR:
                // we look up the binding of the variable in the environment
        {
                const char *name = term_as_var_term(t)->name;
                struct datum *def;
                if (!env_lookup(env, name, &def)) {
                        return make_error(get_original_sexp(t),
                                          "ERROR: Undefined variable");
                }
                return def;
        }
        case TT_APP:
                // This is an application of the form (f a1 a2 ... an).
                // We will fully evaluate the function and each of the
                // arguments, and then call apply.
        {
                struct app_term *at = term_as_app_term(t);
                // evaluate function
                struct datum *fun = eval_datum(at->left, env);
                if (get_type(fun) == T_ERROR) return fun;
                // evaluate arguments
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
                        // the argument list is improper; evaluate
                        // the terminator and construct the argument value
                        // list as improper too
                        struct datum *res = eval_datum(get_pair_first(it), env);
                        if (get_type(res) == T_ERROR) return res;
                        set_pair_second(last, res);
                }
                return apply(fun, arg);
        }
        case TT_MU:
                // recursion term (LABEL f ...).
                // This is a bit tricky.  We bind the recursion variable
                // to a blackhole before evaluation and replace the blackhole
                // with the real value afterward.  If the blackhole is ever
                // evaluated, this means that there is an infinite recursion.
                // Normally, references to the recursion variable are protected
                // from evaluation by a lambda.
        {
                struct mu_term *mt = term_as_mu_term(t);
                struct env *nenv = env_clone(env);
                env_bind(nenv, mt->var, make_error(get_original_sexp(t),
                                                   "Infinite recursion."));
                struct datum *rv = eval_datum(mt->body, nenv);
                env_bind(nenv, mt->var, rv);
                return rv;
        }
        case TT_ABS:
                // Here we evaluate an abstraction.  The result is a
                // closure, that is, essentially a pair of the
                // original lambda term together with the environment
                // here.  Thus, any free variables in the function get
                // their values from here and not from the call site.
                // This solves the environment (funarg) problem, and
                // avoids variable capture.
                return make_closure(get_original_sexp(t), env);
        case TT_GUARDED:
                // (COND ...)
        {
                struct guarded_term *gt = term_as_guarded_term(t);
                do {
                        struct datum *test_result = eval_datum(gt->guard, env);
                        if (!is_NIL(test_result)) {
                                return eval_datum(gt->term, env);
                        }
                        gt = gt->next;
                } while (gt != NULL);
                return make_NIL();
        }
        }
        NOTREACHED;
}        

/*  Evaluates a Lisp term, represented as S-expression data, in the
    environment given.  The result is a Lisp datum representing the
    value of the term.
 */
static struct datum *eval_datum(struct datum *d, struct env *env)
{
        return eval_term(parse_sexp_as_term(d), env);
}
struct datum *eval(struct datum *d)
{
        static struct env *global_env = NULL;
        if (global_env == NULL) global_env = get_primops_env();
        return eval_datum(d, global_env);
}
