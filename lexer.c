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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include "data.h"
#include "y.tab.h"
#include "lexer.h"

enum { UNDETERMINED, NOTTY, TTY } input_type = UNDETERMINED;
static char *line = NULL;
static size_t line_len = 0;
static size_t line_inx = 0;
static size_t open_parens = 0;

int yylex(void)
{
        if (input_type == UNDETERMINED) {
                if (isatty(STDIN_FILENO)) {
                        input_type = TTY;
                        return INTERACTIVE;
                } else {
                        input_type = NOTTY;
                        return NONINTERACTIVE;
                }
                
        }
        while (line_inx < line_len && isspace(line[line_inx])) line_inx++;
        while (line_inx >= line_len) {
                if (input_type == TTY) {
                        free(line);
                        line = readline(open_parens > 0 ? ": " : "> ");
                        if (line == NULL) return 0;
                        line_len = strlen(line);
                } else {
                        static char buf[2048];
                        line_len = fread(buf, sizeof *buf, sizeof buf, stdin);
                        if (feof(stdin)) {
                                return 0;
                        } else if (ferror(stdin)) {
                                fprintf(stderr, "Input error.\n");
                                exit(EXIT_FAILURE);
                        }
                        line = buf;
                }
                line_inx = 0;
                while (line_inx < line_len && isspace(line[line_inx])) {
                        line_inx++;
                }
        }

        switch (line[line_inx]) {
        case '(':
                open_parens++;
                line_inx++;
                return '(';
        case ')':
                if (open_parens > 0) open_parens--;
                line_inx++;
                return ')';
        case '.': case '\'':
                return line[line_inx++];
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        {
                long val = 0;
                while (line_inx < line_len && isdigit(line[line_inx])) {
                        val = 10*val + (line[line_inx] - '0');
                        line_inx++;
                }
                yylval = make_numeric_atom(val);
                return ATOM;
        }
        default:
        {
                int start = line_inx;
                while (line_inx < line_len && !isspace(line[line_inx])) {
                        char c = line[line_inx];
                        if (c == '(' || c == ')') break;
                        line_inx++;
                }
                yylval = make_symbolic_atom(line + start, line_inx - start);
                return ATOM;
        }
        }
        
}

void yyerror(const char *msg)
{
        fprintf(stderr, "%s\n", msg);
}
