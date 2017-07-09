#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ktu_create.h"


static void execute_if_statement(Statement* stmt)
{
	VALUE cond = eval_expression(stmt->u.if_s.condition);

	if (cond.u.long_val) {

		ktu_execute_statement_list(stmt->u.if_s.true_stmt_list);
	}
	else {
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
