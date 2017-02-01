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
#include <strings.h>
#include "env.h"
#include "error.h"

/* I use here simple binary search trees.  Since I have cloned trees
   share storage, no node will be modified after initial insertion;
   insertions to the tree must make a copy of the relevant parents.

   TODO: red-black trees
*/

struct node {
        const char *name;
        struct datum *binding;
        struct node *left;
        struct node *right;
};

struct env {
        struct node *root;
};

struct env *make_empty_env(void)
{
        struct env *rv = GC_malloc(sizeof *rv);
        if (rv == NULL) enomem();
        rv->root = NULL;
        return rv;
}

bool env_lookup(struct env *env, const char *name, struct datum **out)
{
        struct node *n = env->root;
        while (true) {
                if (n == NULL) return false;
                int cmp = strcasecmp(name, n->name);
                if (cmp > 0) {
                        n = n->right;
                } else if (cmp < 0) {
                        n = n->left;
                } else {
                        *out = n->binding;
                        return true;
                }
        }
}

static struct node *insert(struct node *n,
                           const char *name,
                           struct datum *binding)
{
        struct node *rv = GC_malloc(sizeof *rv);
        if (rv == NULL) enomem();
        if (n == NULL) {
                rv->name = name;
                rv->binding = binding;
                rv->left = NULL;
                rv->right = NULL;
        } else {
                rv->name = n->name;
                rv->binding = n->binding;
                rv->left = n->left;
                rv->right = n->right;
        }
        int cmp = strcasecmp(name, rv->name);
        if (cmp < 0) {
                rv->left = insert(rv->left, name, binding);
        } else if (cmp > 0) {
                rv->right = insert(n->right, name, binding);
        } else {
                rv->binding = binding;
        }
        return rv;
}
        
void env_bind(struct env *env, const char *name, struct datum *binding)
{
        env->root = insert(env->root, name, binding);
}

struct env *env_clone(struct env *env)
{
        struct env *rv = GC_malloc(sizeof *rv);
        if (rv == NULL) enomem();
        rv->root = env->root;
        return rv;
}
