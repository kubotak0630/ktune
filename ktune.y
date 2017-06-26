%{
#include <stdio.h>
#include <stdlib.h>
#include "ktu_create.h"
#define YYDEBUG 1

//temp
extern StatementList* g_st_list;

%}
%union {
    char *ident;
    Expression *expression;
    //Statement *statement;
    int int_value;
    //double double_value;
}
//%token <double_value> DOUBLE_LITERAL
//%token <expression> INT_LITERAL
%token <int_value> INT_LITERAL 
%token <ident> STRING_LITERAL
%token <ident> IDENT IDENT_MEMBER
//token <ident> WIDGET_SCALE_LITERAL
%token ADD SUB MUL DIV LP RP ASSIGN CR BIT_AND BIT_OR BIT_L_SHIFT BIT_R_SHIFT
%token C_LBR R_LBR LBR RBR COMMA
%token REG_DATA16 REG_DATA32 REG_DATA64 REG_DATA16R REG_DATA32R REG_DATA64R GTK_PAGE
%type <expression> calc_expr expr primary register_declare  page_create 
%type <expression> widget_scalse_assign widget_combo_assign widget_radio_assign

%left BIT_AND BIT_OR
%left BIT_L_SHIFT BIT_R_SHIFT
%left ADD SUB
%left MUL DIV
%nonassoc UMINUS

%%
line_list
  : line
  | line_list line
  ;
line
  : expr CR
  {
      //printf(">>%d\n", $1);
      Statement* st_0;
      st_0 = ktu_create_expression_statement($1);

      
      if (g_st_list == NULL) {
          g_st_list = ktu_create_statement_list(st_0);
      }
      else {
          ktu_chain_statement_list(g_st_list, st_0);
      }
  }
  | CR
  ;

expr
  : calc_expr
  | IDENT ASSIGN calc_expr
  {
      $$ = ktu_create_assign_expression($1, $3, 0);
      
  }
  | IDENT_MEMBER ASSIGN calc_expr
  {
      $$ = ktu_create_assign_expression($1, $3, 1);
      
  }
  | register_declare
  | widget_scalse_assign
  | page_create
  | widget_combo_assign
  | widget_radio_assign
  ;




calc_expr
  : primary
  | calc_expr ADD calc_expr 
  { 
    $$ = ktu_create_binary_expression(ADD_EXPRESSION, $1, $3);
  }
  | calc_expr SUB calc_expr 
  { 
    $$ = ktu_create_binary_expression(SUB_EXPRESSION, $1, $3); 
  }
  | calc_expr MUL calc_expr
  {
    $$ = ktu_create_binary_expression(MUL_EXPRESSION, $1, $3); 
  }
  | calc_expr DIV calc_expr
  {
    $$ = ktu_create_binary_expression(DIV_EXPRESSION, $1, $3); 
  }
  | calc_expr BIT_AND calc_expr
  {
  	$$ = ktu_create_binary_expression(BIT_AND_EXPRESSION, $1, $3);
  }
  | calc_expr BIT_OR calc_expr
  {
  	$$ = ktu_create_binary_expression(BIT_OR_EXPRESSION, $1, $3);
  }
  | calc_expr BIT_L_SHIFT calc_expr
  {
  	$$ = ktu_create_binary_expression(BIT_L_SHIFT_EXPRESSION, $1, $3);
  }
  | calc_expr BIT_R_SHIFT calc_expr
  {
  	$$ = ktu_create_binary_expression(BIT_R_SHIFT_EXPRESSION, $1, $3);
  }
  ;

primary
  : INT_LITERAL
  {
      $$ = ktu_create_int_expression(INT_EXPRESSION, $1);
  }
  | STRING_LITERAL
  {
      $$ = ktu_create_string_expression($1);
  }
  | IDENT
  {
      
      $$ = ktu_create_identifier_expression($1);
      printf("create_identifier_expression: %s\n", $1);
  }
  | LP calc_expr RP 
  { 
      $$ = $2;
  }
  | SUB calc_expr %prec UMINUS

  { 
      $$ = ktu_create_minus_expression($2);
  }
  ;

register_declare
  : REG_DATA16 IDENT
  {

      $$ = ktu_create_declare_expression($2, REG16);
      printf("data16_declare\n"); 
  }
  | REG_DATA32 IDENT
  {
      $$ = ktu_create_declare_expression($2, REG32);
      printf("data32_declare\n");
  }
  | REG_DATA64 IDENT
  {
      $$ = ktu_create_declare_expression($2, REG64);
      printf("data64_declare\n");
  }
  | REG_DATA16R IDENT
  {

      $$ = ktu_create_declare_expression($2, REG16R);
      printf("data16r_declare\n"); 
  }
  | REG_DATA32R IDENT
  {
      $$ = ktu_create_declare_expression($2, REG32R);
      printf("data32r_declare\n");
  }
  | REG_DATA64R IDENT
  {
      $$ = ktu_create_declare_expression($2, REG64R);
      printf("data64r_declare\n");
  }
  ;

//val = [expr, expr, expr, expr]
widget_scalse_assign
  : IDENT ASSIGN LBR calc_expr COMMA calc_expr COMMA calc_expr COMMA calc_expr RBR
  {
  	  $$ = ktu_create_assign_scale_widget($1, $4, $6, $8, $10);
  }
  ;
  
page_create
  : GTK_PAGE IDENT
  {
      $$ = ktu_create_page($2);
  }
  ;

//c["str0, "str1" "str2", def]
widget_combo_assign
  : IDENT ASSIGN C_LBR valiable_list RBR
  {
  	$$ = ktu_create_assign_enum_widget($1, COMBO_WIDGET);
  }
  ;

//r["str0, "str1" "str2", def]
widget_radio_assign
  : IDENT ASSIGN R_LBR valiable_list RBR
  {
  	$$ = ktu_create_assign_enum_widget($1, RADIO_WIDGET);
  }
  ;


valiable_list
  : calc_expr {
    ktu_create_valiable_length_val($1);
  	printf("*** variable list_create\n");
  }
  | valiable_list COMMA calc_expr
  {
    ktu_add_valiable_length_val($3);
  	printf("*** variable list_bbbbb\n");
  }
  ;

%%
int yyerror(char const *str)
{
    extern char *yytext;
    
    StatementList* pos;

    //リストの末尾を探す
    pos = g_st_list;
    if (pos != NULL) {
    	while (pos->next != NULL) {
    		pos = pos->next;
    	}
    }
   
    
    int error_line = (pos == NULL) ? 1 : pos->statement->line_number;
    
   	 
    
    fprintf(stderr, "line :%d, parser error near %s\n", error_line, yytext);
    return 0;
}


