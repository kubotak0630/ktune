%{
#include <stdio.h>
#include <string.h>
#include "ktu_create.h"
#include "y.tab.h"

extern int g_line_number;

int yywrap(void)
{
  return 1;
}
%}
%state COMMENT BLOCK_COMMENT
%%


<INITIAL>"if"    return IF;
<INITIAL>"else"  return ELSE;
<INITIAL>"elsif"  return ELSIF;
<INITIAL>"+"     return ADD;
<INITIAL>"-"     return SUB;
<INITIAL>"*"     return MUL;
<INITIAL>"/"     return DIV;
<INITIAL>"|"     return BIT_OR;
<INITIAL>"&"     return BIT_AND;
<INITIAL>"<<"    return BIT_L_SHIFT;
<INITIAL>">>"    return BIT_R_SHIFT;
<INITIAL>"("     return LP;
<INITIAL>")"     return RP;
<INITIAL>"{"     return LC;
<INITIAL>"}"     return RC;
<INITIAL>"="     return ASSIGN;
<INITIAL>"=="    return EQ;
<INITIAL>"c["    return C_LBR;  //combo widget
<INITIAL>"r["    return R_LBR;  //radio widget
<INITIAL>"spin[" return SPIN_LBR;
<INITIAL>"["     return LBR;
<INITIAL>"]"     return RBR;
<INITIAL>","     return COMMA;
<INITIAL>^[ \t]*"\n" {
   increment_line_number();
}  //ingnore blank line
<INITIAL>"\n" {
   increment_line_number();
   return EOL;
}
<INITIAL>"$data16"  return REG_DATA16;
<INITIAL>"$data32"  return REG_DATA32;
<INITIAL>"$data64"  return REG_DATA64;
<INITIAL>"$data16b"  return REG_DATA16B;
<INITIAL>"$data32b"  return REG_DATA32B;
<INITIAL>"$data64b"  return REG_DATA64B;
<INITIAL>"$page"     return GTK_PAGE;



<INITIAL>[A-Za-z][A-Za-z_0-9]*.(addr|data|misk) {
  yylval.ident = alloc_string(yytext);
  return IDENT_MEMBER;
}

<INITIAL>\.[A-Za-z][A-Za-z_0-9]* {
  yylval.ident = alloc_string(yytext);
  return MEMBER;
}

<INITIAL>[A-Za-z][A-Za-z_0-9]* {
  yylval.ident = alloc_string(yytext);
  return IDENT;
}

<INITIAL>([1-9][0-9]*)|"0" {
  int temp;
  sscanf(yytext, "%d", &temp);
  yylval.int_value = temp;
  return INT_LITERAL;
}

<INITIAL>0[xX][0-9A-Fa-f]+ {

  int temp;
  temp = strtol(yytext, NULL, 0);
  
  yylval.int_value = temp;
  return INT_LITERAL;
 
}

<INITIAL>\"[^\"]*\" {
  yylval.ident = alloc_string(yytext);
  return STRING_LITERAL;
}

<INITIAL>[ \t]  ; //ingnore tab or space


<INITIAL># {BEGIN COMMENT;}

<COMMENT>\n {
    increment_line_number();
    BEGIN INITIAL;
}
<COMMENT>.      ; //skip comment


<INITIAL>\/\* {
	//printf("***** start block_comment\n");
	BEGIN BLOCK_COMMENT;
}
<BLOCK_COMMENT>. ;
<BLOCK_COMMENT>\n {
  increment_line_number();
  //printf("find CR in Block_comment\n");
};
   /** 終わりがスペース、タブ、改行で終る時 **/
<BLOCK_COMMENT>\*\/[ \t]*\n {
	//printf("***** end block_comment\n");
	BEGIN INITIAL;
}

<BLOCK_COMMENT>\*\/ {
	//printf("***** end block_comment\n");
	BEGIN INITIAL;
}



<INITIAL>. {
  printf("line: %d, lex syntax error!\n", g_line_number);
  exit(1);
  }
  


%%
