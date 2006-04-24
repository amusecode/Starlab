/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     PARTICLE = 258,
     LOG = 259,
     DYNAMICS = 260,
     HYDRO = 261,
     STAR = 262,
     KEYWORD = 263,
     LOG_STORY = 264,
     STRING = 265,
     NUMBER = 266
   };
#endif
#define PARTICLE 258
#define LOG 259
#define DYNAMICS 260
#define HYDRO 261
#define STAR 262
#define KEYWORD 263
#define LOG_STORY 264
#define STRING 265
#define NUMBER 266




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 44 "dyn_parser.yy"
typedef union YYSTYPE {
  double real;
  char* string;
  dyn* dyn_ptr;
  vector<double>* realvec_ptr;
} YYSTYPE;
/* Line 1318 of yacc.c.  */
#line 66 "y.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



