#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ktu_create.h"

int g_line_number = 1;

void increment_line_number()
{
	g_line_number++;
}

/****** statement ********************************************/
static Statement* alloc_statement(StatementType type)
{
    Statement *st;

    st = malloc(sizeof(Statement));
    st->type = type;
    st->line_number = g_line_number;

    return st;
}

Statement* ktu_create_expression_statement(Expression* expr)
{
    Statement *st;
    printf("call ktu_create_expression_statement\n");
    st = alloc_statement(EXPRESSION_STATEMENT);
    st->u.expr_s = expr;

    return st;

}


Statement* ktu_create_if_statement(Expression* condition, StatementList* true_stmt_list, Elsif* elsif, StatementList* else_stmt_list)
{
	Statement* st;

	st = alloc_statement(IF_STATEMENT);
	st->u.if_s.condition = condition;
	st->u.if_s.true_stmt_list = true_stmt_list;
	st->u.if_s.elsif = elsif;
	st->u.if_s.else_stmt_list = else_stmt_list;

	return st;
}


Elsif* ktu_chain_elsif_list(Elsif* list, Elsif* add)
{

	Elsif *pos;

	//リストの末尾を探す
	pos = list;
	while (pos->next != NULL) {
	   pos = pos->next;
	}

	//add list
	pos->next = add;

	return list;


}

Elsif* ktu_create_elsif(Expression *expr, StatementList* stmt_list)
{
	Elsif *elsif;

	elsif = malloc(sizeof(Elsif));

	elsif->condition = expr;
	elsif->block = stmt_list;
	elsif->next = NULL;

	return elsif;


}

StatementList* ktu_create_statement_list(Statement* statement) 
{
    StatementList* st_list;

    //printf("call ktu_create_statement_list\n");
    st_list = malloc(sizeof(StatementList));
    st_list->statement = statement;
    st_list->next = NULL;

    return st_list;

}

StatementList* ktu_chain_statement_list(StatementList* list, Statement* statement)
{
    StatementList* pos;


    //リストの末尾を探す
    pos = list;
    while (pos->next != NULL) {

    	pos = pos->next;
    }

    //add list
    pos->next = ktu_create_statement_list(statement);

    return list;

}

/**** expression ******************************************************/

Expression* ktu_alloc_expression(ExprType type)
{
    Expression *expr;
    
    expr = malloc(sizeof(Expression));
    expr->type = type;
    expr->line_number = g_line_number;

    return expr;

}

Expression* ktu_create_int_expression(ExprType type, int val)
{
    Expression *expr;
    expr = ktu_alloc_expression(type);
    expr->u.int_value = val;


    return expr;
}

Expression* ktu_create_string_expression(char* str)
{

	Expression *expr;
	expr = ktu_alloc_expression(STRING_EXPRESSION);

	/** str = "hoge" のダブルクォテーションを削除(空の文字列はstr[0] = \0 となる) ***/
	int i = 0;
	int index = 0;
	while (str[i] != '\0') {
		if (str[i] != '"') {
			str[index++] = str[i];
		}
		i++;
	}

	str[index] = '\0';


	expr->u.ident = str;

	printf("ktu_create_string_expression = %s\n", str);

	return expr;

}
/*
Expression* ktu_create_widget_expression(ExprType type, char* str)
{
	Expression *expr;
	expr = ktu_alloc_expression(type);
	expr->u.int_value = val;

	return expr;


}
*/
Expression* ktu_create_binary_expression(ExprType operator,
                                         Expression* left, Expression* right)
{

    Expression *expr;
    expr = ktu_alloc_expression(operator);
    expr->u.binary_expr.left = left;
    expr->u.binary_expr.right = right;

    return expr;

}

/*** 通常変数の代入と特殊変数のメンバの代入の区別はis_register_flg でする
 * eval関数が違う
 */
Expression* ktu_create_assign_expression(char* variable, Expression* operand, int is_register_flg)
{
    Expression* expr;

    expr = ktu_alloc_expression(ASSIGN_EXPRESSION);
    expr->u.assign_expr.variable = variable;
    expr->u.assign_expr.operand = operand;


    expr->u.assign_expr.is_register_flg = is_register_flg;


    return expr;
}





Expression* ktu_create_identifier_expression(char* str)
{
    Expression* expr;

    expr = ktu_alloc_expression(IDENT_EXPRESSION);
    expr->u.ident = str;

    return expr;
}

Expression* ktu_create_minus_expression(Expression* operand)
{
    Expression* expr;
    
    expr = ktu_alloc_expression(MINUS_EXPRESSION);
    expr->u.minus_expr = operand;
    
    return expr;

}

// $data32 hoge 宣言で呼ばれる
//str : hoge, type :REG32 etc
Expression* ktu_create_declare_expression(char* str, RegType type)
{
    Expression* expr;

	expr = ktu_alloc_expression(REGX_DECLARE);
	expr->u.reg_declare_expr.regType = type;
	expr->u.reg_declare_expr.ident = str;

	return expr;

}



//scale widgetの代入, val = [min, max, def, shift]
Expression* ktu_create_assign_scale_widget(char* ident, Expression* a1, Expression* a2, Expression* a3, Expression* a4)
{
	Expression* expr;
	expr = ktu_alloc_expression(SCALE_WIDGET_ASSIGN_EXPRESSION);

	expr->u.scale_widget_assign_expr.str_name = ident;
	expr->u.scale_widget_assign_expr.expr_min = a1;
	expr->u.scale_widget_assign_expr.expr_max = a2;
	expr->u.scale_widget_assign_expr.expr_def = a3;
	expr->u.scale_widget_assign_expr.expr_shift = a4;


	return expr;

}

//Spin Widget
Expression* ktu_create_assign_spin_widget(char* ident, Expression* a1, Expression* a2, Expression* a3, Expression* a4)
{
	Expression* expr;
	expr = ktu_alloc_expression(SPIN_WIDGET_ASSIGN_EXPRESSION);

	expr->u.scale_widget_assign_expr.str_name = ident;
	expr->u.scale_widget_assign_expr.expr_min = a1;
	expr->u.scale_widget_assign_expr.expr_max = a2;
	expr->u.scale_widget_assign_expr.expr_def = a3;
	expr->u.scale_widget_assign_expr.expr_shift = a4;


	return expr;

}


Expression* ktu_create_page(char* ident)
{
	Expression* expr;

	expr = ktu_alloc_expression(PAGE_EXPRESSION);
	expr->u.ident = ident;


	return expr;

}

char* g_len_val_lsit[20];
int g_len_val_size = 0;
int g_list_def = 0;

/** 可変長リストの作成 *********/
void ktu_create_valiable_length_val(Expression* expr)
{

	if (expr->type == STRING_EXPRESSION) {
		g_len_val_lsit[0] = expr->u.ident;
		g_len_val_size = 1;
		g_list_def = 0;
	}
	else {
		fprintf(stderr, "syntax error, line near : %d\n", expr->line_number);
		exit(1);
	}
}

/** 可変長リストへ追加 *********/
/* 文字列か整数型かでdefault設定を判断 */
void ktu_add_valiable_length_val(Expression* expr)
{

	//個数の最大値のエラーチェック
	if (g_len_val_size > 19) {
		fprintf(stderr, "error ktu_add_valiable_length_val(), over 20\n");
		exit(1);
	}

	if (expr->type == STRING_EXPRESSION) {

		g_len_val_lsit[g_len_val_size] = expr->u.ident;
		g_len_val_size++;
		printf("add_valiable_length_val = %d\n", expr->u.int_value);
	}
	//INT型はdefault指定と解釈
	else if (expr->type == INT_EXPRESSION) {
		if (expr->u.int_value < g_len_val_size) {
			g_list_def = expr->u.int_value;
		}
		else {
			fprintf(stderr, "error, line near :%d, def_val=%d is over list size\n", expr->line_number, expr->u.int_value);
			fprintf(stderr, "set def_val = 0-%d\n", g_len_val_size-1);
			exit(1);
		}
	}


}

Expression* ktu_create_assign_enum_widget(char* ident, widgetType type)
{

	Expression* expr;
	expr = ktu_alloc_expression(ENUM_WIDGET_ASSIGN_EXPRESSION);

	//可変長リストの領域を確保(グローバル変数からコピー)
	int i;
	for (i = 0; i < g_len_val_size; i++) {

		expr->u.enum_widget_assign_expr.len_val_list[i] = malloc(strlen(g_len_val_lsit[i])+1);

		strcpy(expr->u.enum_widget_assign_expr.len_val_list[i], g_len_val_lsit[i]);
		printf("g_len_val_lsit = %s\n", expr->u.enum_widget_assign_expr.len_val_list[i]);

	}

	expr->u.enum_widget_assign_expr.widget_type = type;
	expr->u.enum_widget_assign_expr.list_size = g_len_val_size;
	expr->u.enum_widget_assign_expr.str_name = ident;
	expr->u.enum_widget_assign_expr.def = g_list_def;

	return expr;
}




