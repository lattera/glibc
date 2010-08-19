/*      @(#)rpc_scan.h  1.3  90/08/29  */

/*
 * Copyright (c) 2010, Oracle America, Inc.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name of the "Oracle America, Inc." nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * rpc_scan.h, Definitions for the RPCL scanner
 */

/*
 * kinds of tokens
 */
enum tok_kind {
	TOK_IDENT,
	TOK_CHARCONST,
	TOK_STRCONST,
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_LBRACE,
	TOK_RBRACE,
	TOK_LBRACKET,
	TOK_RBRACKET,
	TOK_LANGLE,
	TOK_RANGLE,
	TOK_STAR,
	TOK_COMMA,
	TOK_EQUAL,
	TOK_COLON,
	TOK_SEMICOLON,
	TOK_CONST,
	TOK_STRUCT,
	TOK_UNION,
	TOK_SWITCH,
	TOK_CASE,
	TOK_DEFAULT,
	TOK_ENUM,
	TOK_TYPEDEF,
	TOK_INT,
	TOK_SHORT,
	TOK_LONG,
	TOK_HYPER,
	TOK_UNSIGNED,
	TOK_FLOAT,
	TOK_DOUBLE,
	TOK_OPAQUE,
	TOK_CHAR,
	TOK_STRING,
	TOK_BOOL,
	TOK_VOID,
	TOK_PROGRAM,
	TOK_VERSION,
	TOK_EOF
};
typedef enum tok_kind tok_kind;

/*
 * a token
 */
struct token {
	tok_kind kind;
	const char *str;
};
typedef struct token token;


/*
 * routine interface
 */
void scan(tok_kind expect, token *tokp);
void scan2(tok_kind expect1, tok_kind expect2, token *tokp);
void scan3(tok_kind expect1, tok_kind expect2, tok_kind expect3, token *tokp);
void scan_num(token *tokp);
void peek(token *tokp);
int peekscan(tok_kind expect, token *tokp);
void get_token(token *tokp);
void expected1(tok_kind exp1) __attribute__ ((noreturn));
void expected2(tok_kind exp1, tok_kind exp2)  __attribute__ ((noreturn));
void expected3(tok_kind exp1, tok_kind exp2, tok_kind exp3)
     __attribute__ ((noreturn));
