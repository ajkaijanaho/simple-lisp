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


#ifndef GUARD_AST_H
#define GUARD_AST_H

#include "data.h"

enum term_type {
        TT_OTHER,
        TT_DATA,
        TT_VAR,
        //TT_NUMERIC_ATOM, //replaced by TT_DATA
        //TT_SYMBOLIC_ATOM,  //replaced by TT_DATA
        TT_APP,
        TT_ABS,
        TT_MU,
        //TT_PAIR,  //replaced by TT_DATA
        TT_GUARDED,
        // extensions
        TT_DEFINE,
};


struct term;

// used for both variables
struct var_term {
        const char *name;
};

// used for data
struct data_term {
        struct datum *d;
};

struct app_term {
        struct datum *left;
        struct datum *right;
};

/*
  (lambda x ...) ->
     num_params == 0
     params == NULL
     rest_param_name is x

  (lambda (x y) ...) ->
     num_params == 2
     params[0] is x
     params[1] is y
     rest_param_name == NULL

  (lambda (x y . z) ...) ->
     num_params == 2
     params[0] is x
     params[1] is y
     rest_param_name is z
 */
struct abs_term {
        size_t num_params;
        const char **params;
        const char *rest_param_name;
        struct datum *body;
};

struct mu_term {
        const char *var;
        struct datum *body;
};

struct guarded_term {
        struct datum *guard;
        struct datum *term;
        struct guarded_term *next;
};

struct define_term {
        const char *name;
        struct datum *binding;
        struct define_term *next;
}; 

struct term *parse_sexp_as_term(struct datum *);

enum term_type get_term_type(struct term *);

struct datum *get_original_sexp(struct term *);

// defined for TT_VAR
struct var_term *term_as_var_term(struct term *);

// defined for TT_DATA
struct data_term *term_as_data_term(struct term *);

// defined for TT_APP
struct app_term *term_as_app_term(struct term *);

// defined for TT_ABS
struct abs_term *term_as_abs_term(struct term *);

// defined for TT_MU
struct mu_term *term_as_mu_term(struct term *);

// defined for TT_GUARDED
struct guarded_term *term_as_guarded_term(struct term *);

// defined for TT_DEFINE
struct define_term *term_as_define_term(struct term *);

#endif /* GUARD_AST_H */
