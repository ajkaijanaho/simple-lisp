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
 
#ifndef GUARD_CONFIG_H
#define GUARD_CONFIG_H

#ifndef __has_attribute
# define __has_attribute(x) 0
#endif

#if defined(__GNUC__) || __has_attribute(noreturn)
#  define NORETURN(x) x __attribute__((noreturn))
#else
#  define NORETURN(x) x
#endif

#if defined(__GNUC__) || __has_attribute(format)
#  define FORMAT(f,x,y,z) f __attribute__((format(x,y,z)))
#else
#  define FORMAT(f,x,y,z) f
#endif

#endif /* GUARD_CONFIG_H */
