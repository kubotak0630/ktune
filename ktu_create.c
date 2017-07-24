
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
    expr->u.operand_expr = operand;
    
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
Expression* ktu_create_assign_scale_widget(char* ident, Expression* a1, Expression* a2, Expression* a3, Expression* a4, int auto_flg)
{
	Expression* expr;
	expr = ktu_alloc_expression(SCALE_WIDGET_ASSIGN_EXPRESSION);

	expr->u.scale_widget_assign_expr.str_name = ident;
	expr->u.scale_widget_assign_expr.expr_min = a1;
	expr->u.scale_widget_assign_expr.expr_max = a2;
	expr->u.scale_widget_assign_expr.expr_def = a3;
	expr->u.scale_widget_assign_expr.expr_shift = a4;
	expr->u.scale_widget_assign_expr.auto_flg = auto_flg;


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


Expression* ktu_create_page(Expression* expr_ope)
{
	Expression* expr;

	expr = ktu_alloc_expression(PAGE_EXPRESSION);
	expr->u.operand_expr = expr_ope;


	return expr;

}


ExprList* ktu_create_expression_list(Expression* expr)
{


	/*
	//error check
	if (!(expr->type == STRING_EXPRESSION || expr->type == IDENT_EXPRESSION)) {
		fprintf(stderr, "syntax error, line near : %d, allow STRING_EXPRESSION or IDENT_EXPRESSION \n", expr->line_number);
		exit(1);
	}
	*/

	//listを作成
	ExprList* list = (ExprList*)malloc(sizeof(ExprList));
	list->expression = expr;
	list->next = NULL;


	return list;

}


ExprList* ktu_chain_expression_list(ExprList* expr_list, Expression* expr)
{

	ExprList* pos;


	pos = expr_list;

	//リストの末尾を探す
	while (pos->next != NULL) {

		pos = pos->next;
	}

	ExprList* list = (ExprList*) malloc(sizeof(ExprList));
	list->expression = expr;
	list->next = NULL;

	//add list
	pos->next = list;


	return expr_list;
	
}

Expression* ktu_create_assign_enum_widget(ExprList* expr_list ,char* ident, widgetType type)
{

	Expression* expr;
	expr = ktu_alloc_expression(ENUM_WIDGET_ASSIGN_EXPRESSION);

	expr->u.enum_widget_assign_expr.widget_type = type;
	expr->u.enum_widget_assign_expr.str_name = ident;
	expr->u.enum_widget_assign_expr.expr_list = expr_list;

	return expr;
}

Expression* ktu_create_sturct_init_assign_expression(ExprList* expr_list, char* ident, RegType reg_type)
{

	Expression* expr;
	expr = ktu_alloc_expression(ASSIGN_STRUCT_INIT_EXPRESSION);

	expr->u.assign_struct_init_expr.expr_list = expr_list;
	expr->u.assign_struct_init_expr.str_name = ident;
	expr->u.assign_struct_init_expr.reg_type = reg_type;

	return expr;

}

