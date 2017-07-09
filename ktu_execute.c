#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ktu_create.h"

/*** 戻り値 elsifのリストが実行された場合は1、実行されなければ0を返す**/
static int execute_elsif(Elsif* elsif)
{
	Elsif* pos;
	VALUE cond;
	int ret_val = 0;

	for (pos = elsif; pos != NULL; pos = pos->next) {
		cond = eval_expression(pos->condition);

		if (cond.u.long_val) {
			ktu_execute_statement_list(pos->block);
			ret_val = 1;
			break;
		}
	}

	return ret_val;
}

static void execute_if_statement(Statement* stmt)
{
	VALUE cond = eval_expression(stmt->u.if_s.condition);

	int ret_elsif;

	if (cond.u.long_val) {

		ktu_execute_statement_list(stmt->u.if_s.true_stmt_list);
	}
	else {
		ret_elsif = execute_elsif(stmt->u.if_s.elsif);
	}

	if (!ret_elsif) {
		ktu_execute_statement_list(stmt->u.if_s.else_stmt_list);
	}

}

static void execute_statement(Statement* stmt)
{


    switch (stmt->type) {
    case EXPRESSION_STATEMENT:
        printf("execute EXPRESSION_STATEMENT\n\n");
        eval_expression(stmt->u.expr_s);
        break;

    case IF_STATEMENT:
    	printf("execute IF_STATEMENT\n\n");
    	execute_if_statement(stmt);

    	break;

    default:
        printf("execute_statement error\n");
        exit(1);
    }

}


void ktu_execute_statement_list(StatementList* list)
{

    StatementList* pos;

    if (list == NULL) {
        printf("execute_statement_list() error, list is NULL\n");
        exit(1);
    }

    for (pos = list; pos != NULL; pos = pos->next) {
        execute_statement(pos->statement);
    }

}
