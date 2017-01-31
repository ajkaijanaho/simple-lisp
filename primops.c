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


#include "error.h"
#include "primops.h"

static struct datum *prim_EQ(struct datum *d)
{
        if (is_NIL(d)) return make_T();
        if (get_type(d) != T_PAIR) {
                return make_error(d, "EQ: Improper parameter.");
        }
        struct datum *prev = get_pair_first(d);
        struct datum *it;
        for (it = get_pair_second(d);
             get_type(it) == T_PAIR;
             it = get_pair_second(it)) {
                struct datum *cur = get_pair_first(it);
                if (get_type(prev) != get_type(cur)) return make_NIL();
                switch (get_type(cur)) {
                case T_NUMBER:
                        if (get_numeric_value(prev) !=
                            get_numeric_value(cur)) return make_NIL();
                        break;
                case T_SYMBOL:
                        if (!is_this_symbol(cur, get_symbol_name(prev))) {
                                return make_NIL();
                        }
                        break;
                case T_PAIR: case T_PRIMITIVE: case T_CLOSURE:
                        return make_NIL();
                case T_ERROR:
                        NOTREACHED;
                }
        }
        if (!is_NIL(it)) {
                return make_error(d, "EQ: Improper parameter.");
        }
        return make_T();
}

static struct datum *prim_ATOM(struct datum *d)
{
        if (get_type(d) != T_PAIR) return make_NIL();
        enum data_type ty = get_type(get_pair_first(d));
        return ty == T_SYMBOL || ty == T_NUMBER ? make_T() : make_NIL();
}

static struct datum *prim_CONS(struct datum *d)
{
        struct list_data dd = get_list_data(d);
        if (dd.n != 2 || !is_NIL(dd.terminator)) {
                make_error(d, "CONS: incorrect parameter list");
        }
        return make_pair(dd.vec[0], dd.vec[1]);
}

static struct datum *prim_CAR(struct datum *d)
{
        struct list_data dd = get_list_data(d);
        if (dd.n != 1 || !is_NIL(dd.terminator)) {
                make_error(d, "CAR: incorrect parameter list");
        }
        if (get_type(dd.vec[0]) != T_PAIR) {
                make_error(d, "CAR: not a pair");
        }
        return get_pair_first(dd.vec[0]);
}

static struct datum *prim_CDR(struct datum *d)
{
        struct list_data dd = get_list_data(d);
        if (dd.n != 1 || !is_NIL(dd.terminator)) {
                make_error(d, "CDR: incorrect parameter list");
        }
        if (get_type(dd.vec[0]) != T_PAIR) {
                make_error(d, "CDR: not a pair");
        }
        return get_pair_second(dd.vec[0]);
}

static struct datum *prim_ADD(struct datum *d)
{
        struct list_data dd = get_list_data(d);
        if (dd.n != 2 || !is_NIL(dd.terminator)) {
                make_error(d, "ADD: incorrect parameter list");
        }
        if (get_type(dd.vec[0]) != T_NUMBER ||
            get_type(dd.vec[1]) != T_NUMBER) {
                make_error(d, "ADD: type error");
        }
        return make_numeric_atom(get_numeric_value(dd.vec[0])
                                 +
                                 get_numeric_value(dd.vec[1]));
}
        
static struct datum *prim_SUB(struct datum *d)
{
        struct list_data dd = get_list_data(d);
        if (dd.n != 2 || !is_NIL(dd.terminator)) {
                make_error(d, "SUB: incorrect parameter list");
        }
        if (get_type(dd.vec[0]) != T_NUMBER ||
            get_type(dd.vec[1]) != T_NUMBER) {
                make_error(d, "SUB: type error");
        }
        return make_numeric_atom(get_numeric_value(dd.vec[0])
                                 -
                                 get_numeric_value(dd.vec[1]));
}
        
static struct datum *prim_MUL(struct datum *d)
{
        struct list_data dd = get_list_data(d);
        if (dd.n != 2 || !is_NIL(dd.terminator)) {
                make_error(d, "MUL: incorrect parameter list");
        }
        if (get_type(dd.vec[0]) != T_NUMBER ||
            get_type(dd.vec[1]) != T_NUMBER) {
                make_error(d, "MUL: type error");
        }
        return make_numeric_atom(get_numeric_value(dd.vec[0])
                                 *
                                 get_numeric_value(dd.vec[1]));
}

static struct datum *prim_DIV(struct datum *d)
{
        struct list_data dd = get_list_data(d);
        if (dd.n != 2 || !is_NIL(dd.terminator)) {
                make_error(d, "DIV: incorrect parameter list");
        }
        if (get_type(dd.vec[0]) != T_NUMBER ||
            get_type(dd.vec[1]) != T_NUMBER) {
                make_error(d, "DIV: type error");
        }
        return make_numeric_atom(get_numeric_value(dd.vec[0])
                                 /
                                 get_numeric_value(dd.vec[1]));
}



struct primop {
        const char *name;
        prim_fun fun;
};
static const struct primop primops[] = {
        { "EQ", prim_EQ },
        { "ATOM", prim_ATOM },
        { "CONS", prim_CONS },
        { "CAR", prim_CAR },
        { "CDR", prim_CDR },
        { "ADD", prim_ADD },
        { "SUB", prim_SUB },
        { "MUL", prim_MUL },
        { "DIV", prim_DIV },
};

struct datum *get_primops_alist(void)
{
        struct datum *rv = make_NIL();
        for (size_t i = 0; i < sizeof primops / sizeof *primops; i++) {
                rv = make_pair
                        (make_pair
                         (make_symbolic_atom_cstr(primops[i].name),
                          make_primitive(primops[i].fun)),
                         rv);
        }
        return rv;
}
