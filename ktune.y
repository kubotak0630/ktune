%{
#include <stdio.h>
#include <stdlib.h>
#include "ktu_create.h"
#define YYDEBUG 1

extern int g_line_number;

//temp
extern StatementList* g_st_list;

%}
%union {
    char *ident;
    Expression *expr;
    ExprList *expr_list;
    Statement *stmt;
    StatementList *stmt_list;
    Elsif *elsif;
    int int_value;

}

%token <int_value> INT_LITERAL 
%token <ident> STRING_LITERAL
%token <ident> IDENT IDENT_MEMBER MEMBER
%token IF ELSE ELSIF ADD SUB MUL DIV LP RP LC RC ASSIGN EQ BIT_AND BIT_OR BIT_L_SHIFT BIT_R_SHIFT EOL
%token C_LBR R_LBR SPIN_LBR SCALE_LBR SCALE_AUTO_LBR RBR COMMA
%token REG_DATA16 REG_DATA32 REG_DATA64 REG_DATA16B REG_DATA32B REG_DATA64B GTK_PAGE
%type <expr> calc_expr expression primary register_declare  page_create member_assign struct_member_block_assign
%type <expr> widget_scalse_assign widget_combo_assign widget_radio_assign widget_spin_assign
%type <stmt> statement if_statement expression_statement block_item
%type <stmt_list> statement_list block_item_list block
%type <elsif> elsif_item elsif_list
%type <expr_list> valiable_list member_assign_list

%left BIT_AND BIT_OR
%nonassoc EQ
%left BIT_L_SHIFT BIT_R_SHIFT
%left ADD SUB
%left MUL DIV
%right UMINUS


%%

translation_unit
  : statement_list
  ;

statement_list
  : statement
  {
    //printf("ktu_create_statement_list\n");
    g_st_list = ktu_create_statement_list($1);
  }
  | statement_list statement
  {
    //printf("ktu_chain_statement_list\n");
    ktu_chain_statement_list(g_st_list, $2);
  }
  ;
  

statement
  : expression_statement
  | if_statement
  ;


if_statement
  : IF LP expression RP block
  {
  	printf("--------- if_stmt --------------\n");
  	$$ = ktu_create_if_statement($3, $5, NULL, NULL);
  }
  | IF LP expression RP block ELSE block
  {
     $$ = ktu_create_if_statement($3, $5, NULL, $7);
  }
  | IF LP expression RP block elsif_list
  {
    $$ = ktu_create_if_statement($3, $5, $6, NULL);
  
  }
  | IF LP expression RP block elsif_list ELSE block
  {
    $$ = ktu_create_if_statement($3, $5, $6, $8);
  }
  ;


elsif_list
  : elsif_item
  | elsif_list elsif_item
  {
    printf("elsif_list: ktu_chain_elsif_list\n");
    $$ = ktu_chain_elsif_list($1, $2);
  }
  ;

elsif_item
  : ELSIF LP expression RP block
  {
  
    printf("elsif_item: ktu_create_elsif\n");
    $$ = ktu_create_elsif($3, $5);
  }
  ;

 
block
//	: LC block_item_list RC
	: _LC_ block_item_list _RC_
	{
		$$ = $2;
		printf("block\n");
	}
	;

block_item_list
	: block_item 
	{
	  printf("new block item list --ktu_create_statement_list\n");
	  $$ = ktu_create_statement_list($1);
	}
	| block_item_list block_item 
	{
	  printf("add block item list --ktu_create_statement_list\n");
	  $$ = ktu_chain_statement_list($1, $2);
	}
	;

block_item
	: statement
	{
		$$ = $1;
		printf("block_item\n");
	}
	;


expression_statement
  : expression EOL {
  	$$ = ktu_create_expression_statement($1);
  }
  | struct_member_block_assign {
    $$ = ktu_create_expression_statement($1);
  }
  ;

expression
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
  | widget_spin_assign
  | page_create
  | widget_combo_assign
  | widget_radio_assign
    //  | struct_member_block_assign
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
  | calc_expr EQ calc_expr
  {
    $$ = ktu_create_binary_expression(EQ_EXPRESSION, $1, $3);
  }
/*
  | SUB calc_expr %prec UMINUS
  {
    $$ = ktu_create_minus_expression($2);
  }
  | LP calc_expr RP 
  { 
      $$ = $2;
  }
*/
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
  | SUB primary %prec UMINUS
  { 
      $$ = ktu_create_minus_expression($2);
  }
  ;


_LC_
  : eol_opt LC eol_opt
  ;

_RC_
  : eol_opt RC eol_opt
  ;

eol_opt
  : 
  | EOL
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
  | REG_DATA16B IDENT
  {
      $$ = ktu_create_declare_expression($2, REG16B);
      printf("data16r_declare\n"); 
  }
  | REG_DATA32B IDENT
  {
      $$ = ktu_create_declare_expression($2, REG32B);
      printf("data32r_declare\n");
  }
  | REG_DATA64B IDENT
  {
      $$ = ktu_create_declare_expression($2, REG64B);
      printf("data64r_declare\n");
  }
  ;

//val = [expr, expr, expr, expr]
widget_scalse_assign
  : IDENT ASSIGN SCALE_LBR calc_expr COMMA calc_expr COMMA calc_expr COMMA calc_expr RBR
  {
  	  $$ = ktu_create_assign_scale_widget($1, $4, $6, $8, $10, 1);
  }
  |
  IDENT ASSIGN SCALE_AUTO_LBR calc_expr COMMA calc_expr COMMA calc_expr COMMA calc_expr RBR
  {
  	  $$ = ktu_create_assign_scale_widget($1, $4, $6, $8, $10, 0);
  }
  ;

//val = [expr, expr, expr, expr]
widget_spin_assign
  : IDENT ASSIGN SPIN_LBR calc_expr COMMA calc_expr COMMA calc_expr COMMA calc_expr RBR
  {
  	  $$ = ktu_create_assign_spin_widget($1, $4, $6, $8, $10);
  }
  ;

 
 
page_create
  : GTK_PAGE calc_expr
  {
      $$ = ktu_create_page($2);
  }
  ;

//c["str0, "str1" "str2", def]
widget_combo_assign
  : IDENT ASSIGN C_LBR valiable_list RBR
  {
  	$$ = ktu_create_assign_enum_widget($4, $1, COMBO_WIDGET);
  }
  ;

//r["str0, "str1" "str2", def]
widget_radio_assign
  : IDENT ASSIGN R_LBR valiable_list RBR
  {
  	$$ = ktu_create_assign_enum_widget($4, $1, RADIO_WIDGET);
  }
  ;


valiable_list
  : calc_expr {
    $$ = ktu_create_expression_list($1);
  }
  | valiable_list COMMA calc_expr
  {
    $$ = ktu_chain_expression_list($1, $3);
  }
  ;


//構造体のブロック初期化代入
struct_member_block_assign
  : REG_DATA16 IDENT ASSIGN _LC_ member_assign_list _RC_
  {
    printf("*** member_block_assign ******\n");
    $$ = ktu_create_sturct_init_assign_expression($5, $2, REG16);
  }
  | REG_DATA32 IDENT ASSIGN _LC_ member_assign_list _RC_
  {
    printf("*** member_block_assign ******\n");
    $$ = ktu_create_sturct_init_assign_expression($5, $2, REG32);
  }
  | REG_DATA64 IDENT ASSIGN _LC_ member_assign_list _RC_
  {
    printf("*** member_block_assign ******\n");
    $$ = ktu_create_sturct_init_assign_expression($5, $2, REG64);
  }
  | REG_DATA16B IDENT ASSIGN _LC_ member_assign_list _RC_
  {
    printf("*** member_block_assign ******\n");
    $$ = ktu_create_sturct_init_assign_expression($5, $2, REG16B);
  }
  | REG_DATA32B IDENT ASSIGN _LC_ member_assign_list _RC_
  {
    printf("*** member_block_assign ******\n");
    $$ = ktu_create_sturct_init_assign_expression($5, $2, REG32B);
  }
  | REG_DATA64B IDENT ASSIGN _LC_ member_assign_list _RC_
  {
    printf("*** member_block_assign ******\n");
    $$ = ktu_create_sturct_init_assign_expression($5, $2, REG64B);
  };

member_assign_list
  : member_assign {
  	$$ = ktu_create_expression_list($1);
  }
  | member_assign_list member_assign {
    $$ = ktu_chain_expression_list($1, $2);
  }
  ;

//.data = expr
member_assign
  : MEMBER ASSIGN calc_expr EOL {
  	printf("****** member_assign %s ***********\n", $1);
  	$$ = ktu_create_assign_expression($1, $3, 0);
  }
  ;


%%
int yyerror(char const *str)
{
    extern char *yytext;
  
   	 
    fprintf(stderr, "line :%d, parser error near %s\n", g_line_number, yytext);
    return 0;
}


