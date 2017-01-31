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

#include <gc.h>
#include <string.h>
#include "error.h"
#include "strvec.h"

struct str_vec {
        size_t n;
        size_t maxn;
        const char **vec;
};

// returns an empty string vector
struct str_vec *str_vec_new(void)
{
        struct str_vec *rv = GC_malloc(sizeof *rv);
        if (rv == NULL) enomem();
        rv->n = 0;
        rv->maxn = 0;
        rv->vec = NULL;
        return rv;
}

// adds a string to the end of a string vector
// (str is appended as is, no copy is made)
void str_vec_append(struct str_vec *v, const char *str)
{
        if (v->n >= v->maxn) {
                // double the capacity of the vector (starting with 2)
                v->maxn = v->maxn > 0 ? 2*v->maxn : 2;
                v->vec = GC_realloc(v->vec, v->maxn * sizeof *v->vec);
                if (v->vec == NULL) enomem();
        }
        v->vec[v->n++] = str;
}

// returns the number of strings in the string vector
size_t str_vec_len(struct str_vec *v)
{
        return v->n;
}

// returns a GC_malloc'd array corresponding to the string vector
const char **str_vec_to_array(struct str_vec *v)
{
        // We try to make a copy so that we do not have to keep around
        // (in the worst case) nearly double the needed memory
        // (usually v will be garbage after this function)
        const char **rv = GC_malloc(v->n * sizeof *rv);
        if (rv == NULL) return v->vec;
        memcpy(rv, v->vec, v->n * sizeof *rv);
        return rv;
}

