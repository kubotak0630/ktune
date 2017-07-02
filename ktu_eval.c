#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ktu_create.h"


int g_search_bingo = 0;
char g_search_str[64];

/*** prototype declare ***********************/
VALUE get_variable_from_list(char* name, int line_num);
VALUE eval_expression(Expression* expr);
void set_reg_variable(char* ident, char* member, void* pData);



//return :0 INT, return 1:REG32
// ex) hoge.data -> str_out1:hoge, str_out:data, return 0
// ex) hoge10 ->  str_out1:hoge10,               return 1 
int str_split(char* str, char* str_out1, char* str_out2)
{
    
    int i = 0;
    int state = 0;
    int ret_val = 1;
    int index = 0;
    //printf("before_split: %s\n", str);
    for (i = 0; str[i] != '\0'; i++) {
        if (state == 0) {
            if (str[i] != '.') {
                str_out1[i] = str[i];
            }
            else {
                str_out1[i] = '\0';
                state = 1;
                ret_val = 0;
            }
        }
        else {
            str_out2[index++] = str[i];
        }
    }

    str_out1[i] = '\0';
    str_out2[index] = '\0';

    return ret_val;

}

int eval_assign_expression(char* ident, int is_register_flg, Expression* expr)
{

    char str0[128];
    char str_mem[16];

    //int ret_val = eval_expression(expr);
    int ret_val = 0;


    //INT型変数への代入
    if (is_register_flg == 0) {


    	ret_val = eval_expression(expr).u.long_val;
    	add_variable_list(ident, (void*)&ret_val, VARIABLE_INT, 0);
    }
    //REGX型のメンバ変数への代入
    else {

		str_split(ident, str0, str_mem);

		printf("bunri_ident = %s\n", str0);
		printf("mem = %s\n", str_mem);

		/* .dataはexpressin(構文木)のアドレスを保存する。*/
		if (strcmp(str_mem, "data") == 0) {
			set_reg_variable(str0, str_mem, (void*) expr);
		}
		//
		else if (strcmp(str_mem, "addr") == 0 || strcmp(str_mem, "misk") == 0) {

			ret_val = eval_expression(expr).u.long_val;
			set_reg_variable(str0, str_mem, (void*) &ret_val);
		} else {
			fprintf(stderr, "error eval_assign_expression\n");
			exit(1);
		}

    }


    printf("eval_assign_expr: %s\n", ident);
    

    return ret_val;

}


int64_t eval_identifier_expression(Expression* expr)
{

	VALUE val;
	val = get_variable_from_list(expr->u.ident, expr->line_number);

	int64_t ret_val = 0;

	if (val.type == VARIABLE_INT) {

		ret_val = val.u.int_val;
	}
	else if (val.type == VARIABLE_WIDGET) {

		ret_val = val.u.widget.val;

		ret_val = (ret_val << val.u.widget.shift);

		if (strcmp(g_search_str, expr->u.ident) == 0) {
			g_search_bingo = 1;
		}
	}
	else {

		fprintf(stderr, "error eval_identifier_expression\n");
		exit(1);
	}





	return ret_val;
}


int eval_minus_expression(Expression* expr)
{

    int ret_val;

    ret_val = -1 * eval_expression(expr).u.long_val;

    return ret_val;

}

/** min==0, max==1の時はBUTTON_WIDGETとする *************/
int eval_scale_widet_assign_expression(Expression* expr)
{


	Widget widgetData;


	widgetData.min = eval_expression(expr->u.scale_widget_assign_expr.expr_min).u.long_val;
	widgetData.max = eval_expression(expr->u.scale_widget_assign_expr.expr_max).u.long_val;
	widgetData.def = eval_expression(expr->u.scale_widget_assign_expr.expr_def).u.long_val;
	widgetData.shift = eval_expression(expr->u.scale_widget_assign_expr.expr_shift).u.long_val;
	widgetData.val = widgetData.def; //ここではvalの値はdefをいれておく

	if (widgetData.min == 0 && widgetData.max == 1) {
		widgetData.type = BUTTON_WIDGET;
	}
	else {
		widgetData.type = SCALE_WIDGET;
	}


	//scale型変数の作成
	add_variable_list(expr->u.scale_widget_assign_expr.str_name, (void*)&widgetData, VARIABLE_WIDGET, 0);

	return 0;


}


int eval_spin_widet_assign_expression(Expression* expr)
{


	Widget widgetData;


	widgetData.min = eval_expression(expr->u.scale_widget_assign_expr.expr_min).u.long_val;
	widgetData.max = eval_expression(expr->u.scale_widget_assign_expr.expr_max).u.long_val;
	widgetData.def = eval_expression(expr->u.scale_widget_assign_expr.expr_def).u.long_val;
	widgetData.shift = eval_expression(expr->u.scale_widget_assign_expr.expr_shift).u.long_val;
	widgetData.val = widgetData.def; //ここではvalの値はdefをいれておく


	widgetData.type = SPIN_WIDGET;

	//scale型変数の作成
	add_variable_list(expr->u.scale_widget_assign_expr.str_name, (void*)&widgetData, VARIABLE_WIDGET, 0);

	return 0;

}


int eval_enum_widet_assign_expression(Expression* expr)
{


	Widget widgetData;

	widgetData.type = expr->u.enum_widget_assign_expr.widget_type;
	widgetData.str_list = expr->u.enum_widget_assign_expr.len_val_list;
	widgetData.list_size = expr->u.enum_widget_assign_expr.list_size;
	widgetData.def = expr->u.enum_widget_assign_expr.def;

	widgetData.val = widgetData.def; //ここではvalの値はdefをいれておく


	//scale型変数の作成
	add_variable_list(expr->u.enum_widget_assign_expr.str_name, (void*)&widgetData, VARIABLE_WIDGET, 0);

	return 0;


}

void debug_convert_expr_type(ExprType type, char* str){

    switch (type) {
    case INT_EXPRESSION:
        strcpy(str, "int_expr");
        break;
    case IDENT_EXPRESSION:
        strcpy(str, "ident_expr");
        break;
    case ASSIGN_EXPRESSION:
        strcpy(str, "=");
        break;
    case ADD_EXPRESSION:
        strcpy(str, "+");
        break;
    case SUB_EXPRESSION:
        strcpy(str, "-");
        break;
    case MUL_EXPRESSION:
        strcpy(str, "*");
        break;
    case DIV_EXPRESSION:
        strcpy(str, "/");
        break;
    case MINUS_EXPRESSION:
        strcpy(str, "minus_expr");
        break;
    case REGX_DECLARE:
            strcpy(str, "regx_declare");
            break;

    default:
        strcpy(str, "**** debug_conv_error ******");
    }

}

VALUE eval_expression(Expression* expr)
{

    uint64_t reft_val;
    uint64_t right_val;

    VALUE v;


    //char str[32];
    //debug_convert_expr_type(expr->type, str);
    //printf("eval_expr: %s\n", str);

    switch(expr->type) {
    case INT_EXPRESSION:
    	v.type = VARIABLE_LONG;
    	v.u.long_val = expr->u.int_value;
        return v;

    case STRING_EXPRESSION:
    	//printf("STRING_EXPRESSION = %s\n", expr->u.ident);
    	v.type = VARIABLE_STRING;
    	v.u.str = expr->u.ident;
    	return v;

    //変数を参照した時の動作 変数リストをサーチして値を返す
    case IDENT_EXPRESSION:
    	v.type = VARIABLE_LONG;
    	v.u.long_val = eval_identifier_expression(expr);
        return v;

    case WIDGET_EXPRESSION:

    	return v;

    case ASSIGN_EXPRESSION:
    	v.type = VARIABLE_INT;
    	v.u.int_val = eval_assign_expression(expr->u.assign_expr.variable, expr->u.assign_expr.is_register_flg, expr->u.assign_expr.operand);
    	return v;


    case ADD_EXPRESSION:
        reft_val = eval_expression(expr->u.binary_expr.left).u.long_val;
        right_val = eval_expression(expr->u.binary_expr.right).u.long_val;

        v.type = VARIABLE_LONG;
        v.u.long_val = reft_val + right_val;
        return v;

  
    case SUB_EXPRESSION:

        reft_val = eval_expression(expr->u.binary_expr.left).u.long_val;
        right_val = eval_expression(expr->u.binary_expr.right).u.long_val;

        v.type = VARIABLE_LONG;
        v.u.long_val = reft_val - right_val;
        return v;


    case MUL_EXPRESSION:

        reft_val = eval_expression(expr->u.binary_expr.left).u.long_val;
        right_val = eval_expression(expr->u.binary_expr.right).u.long_val;

        v.type = VARIABLE_LONG;
        v.u.long_val = reft_val * right_val;
        return v;


    case DIV_EXPRESSION:

        reft_val = eval_expression(expr->u.binary_expr.left).u.long_val;
        right_val = eval_expression(expr->u.binary_expr.right).u.long_val;

        v.type = VARIABLE_LONG;
        v.u.long_val = reft_val / right_val;
        return v;



    case BIT_AND_EXPRESSION:
    	reft_val = eval_expression(expr->u.binary_expr.left).u.long_val;
	    right_val = eval_expression(expr->u.binary_expr.right).u.long_val;

	    v.type = VARIABLE_LONG;
	    v.u.long_val = reft_val & right_val;
	    return v;

    case BIT_OR_EXPRESSION:
    	reft_val = eval_expression(expr->u.binary_expr.left).u.long_val;
 		right_val = eval_expression(expr->u.binary_expr.right).u.long_val;

 		v.type = VARIABLE_LONG;
 		v.u.long_val = reft_val | right_val;
 		return v;


    case BIT_L_SHIFT_EXPRESSION:
        reft_val = eval_expression(expr->u.binary_expr.left).u.long_val;
     	right_val = eval_expression(expr->u.binary_expr.right).u.long_val;

     	v.type = VARIABLE_LONG;
     	v.u.long_val = reft_val << right_val;
     	return v;


    case BIT_R_SHIFT_EXPRESSION:
        reft_val = eval_expression(expr->u.binary_expr.left).u.long_val;
        right_val = eval_expression(expr->u.binary_expr.right).u.long_val;

        v.type = VARIABLE_LONG;
        v.u.long_val = reft_val >> right_val;
        return v;


    case MINUS_EXPRESSION:

    	v.type = VARIABLE_LONG;
    	v.u.long_val = eval_minus_expression(expr->u.minus_expr);
    	return v;



    case REGX_DECLARE:
    	/*** $data32 data_hoge という特殊変数の宣言 *********
    	* 変数リストに変数を追加。データの値はセットしない。 */
    	add_variable_list(expr->u.reg_declare_expr.ident, NULL, VARIABLE_REGX, expr->u.reg_declare_expr.regType);

        return v;

    case SCALE_WIDGET_ASSIGN_EXPRESSION:

    	//変数を登録するだけ。xtuenの操作は後でやる
    	eval_scale_widet_assign_expression(expr);
    	return v;

    case SPIN_WIDGET_ASSIGN_EXPRESSION:

    	//変数を登録するだけ。xtuenの操作は後でやる
    	 eval_spin_widet_assign_expression(expr);
    	 return v;

    //combo_box or radio
    case ENUM_WIDGET_ASSIGN_EXPRESSION:

    	printf("ENUM_WIDGET_ASSIGN_EXPRESSION\n");
    	//変数を登録するだけ。xtuenの操作は後でやる
    	eval_enum_widet_assign_expression(expr);

    	return v;

    case PAGE_EXPRESSION:
    	add_variable_list(expr->u.ident, NULL, PAGE, 0);
    	return v;

    default:
        fprintf(stderr, "EvalExpr() error, Expression undefined\n");
        exit(1);


    }


    return v;

}




VarDictList *g_VarDictList = NULL;
#if 0
void show_gtk_val()
{
	int i;

	for (i = 0; i < g_tail; i++) {
		if (g_variable_list[i].val.type == VARIABLE_REGX) {
			printf("%s\n", g_variable_list[i].name);
		}
		else if (g_variable_list[i].val.type == VARIABLE_SCALE_WIDGET) {
			printf("%s\n", g_variable_list[i].name);
		}

	}



}
#endif

/* name という変数がリストになければ新規追加。既にある場合は上書き ********/
void add_variable_list(char* name, void* pData, ValType val_type, RegType reg_type)
{


	VarDictList* node;


	node = malloc(sizeof(VarDictList));
	node->next = NULL;

	node->VarDict.val.type = val_type;

	//変数名の領域の確保してセット
	int str_size = strlen(name) + 1;
	node->VarDict.name = (char*) malloc(str_size);
	strcpy(node->VarDict.name, name);

	/*** 変数の値をセット ***************************/
	if (val_type == VARIABLE_INT) {
		node->VarDict.val.u.int_val = *((int*)pData);
	}
	else if (val_type == VARIABLE_WIDGET) {

		Widget* p = (Widget*) pData;

		//共通の要素
		node->VarDict.val.u.widget.type = p->type;
		node->VarDict.val.u.widget.val = p->val;
		node->VarDict.val.u.widget.def = p->def;

		//SCALE Widget
		if (p->type == SCALE_WIDGET || p->type == BUTTON_WIDGET || p->type == SPIN_WIDGET) {

			//構造体のコピー
			node->VarDict.val.u.widget.min = p->min;
			node->VarDict.val.u.widget.max = p->max;
			node->VarDict.val.u.widget.shift = p->shift;

		}
		else if (p->type == COMBO_WIDGET || p->type == RADIO_WIDGET) {
			node->VarDict.val.u.widget.str_list = p->str_list;
			node->VarDict.val.u.widget.list_size = p->list_size;
		}
		else {

			printf("error add_vbariable_list(), widget typte unknow\n");
			exit(1);

		}

	}
	//REG32,16,64	は変数の宣言と代入が別のため、データのセットはeval_assign_expressionで実行
	else if (val_type == VARIABLE_REGX) {
		//ポインタをNULLで初期化しておく
		node->VarDict.val.u.regx.data_expr = NULL;
		node->VarDict.val.u.regx.p_gtk_label = NULL;

		node->VarDict.val.u.regx.reg_type = reg_type;

		//not data setting

	}
	else if (val_type == PAGE) {
	    	//NOP
	}
	else {

	   fprintf(stderr, "error add_variable_list()\n");
	   exit(1);
	}


	VarDictList* pos = g_VarDictList;
	int new_flg = 1;

	//リストが空の時
	if (g_VarDictList == NULL) {
		node->prev = NULL;
		g_VarDictList = node;
		return;

	} else {

		//変数が既にあるかどうかを調べる。ある場合は上書き,新規の変数名は末尾に追加

		for (pos = g_VarDictList; pos != NULL; pos = pos->next) {

			if (strcmp(pos->VarDict.name, name) == 0) {
				new_flg = 0;
				break;
			}
		}

		//変数がない場合は、末尾まで移動
		if (new_flg) {
			pos = g_VarDictList;
			//末尾まで移動
			while (pos->next != NULL) {
				pos = pos->next;
			}
		}

	}

	//変数リストの末尾に追加
	if (new_flg) {
		node->prev = pos;
		pos->next = node;
	}
	//上書き(古い変数を削除して入れ替える)
	else {
		VarDictList* temp = pos;

		//先頭の要素
		if (pos->prev == NULL) {

			g_VarDictList = node;

			free(temp);
		}
		else {
			pos->prev->next = node;
			node->prev = pos;

			free(temp);
		}
	}
}

void set_reg_variable(char* ident, char* member, void* pData)
{


	int error_flg = 1;

	VarDictList* pos;

	for (pos = g_VarDictList; pos != NULL; pos = pos->next) {

		if (strcmp(pos->VarDict.name, ident) == 0) {
			error_flg = 0;

			printf("reg32 add_variable_list\n");

			if (strcmp(member, "data") == 0) {
				//g_variable_list[index].val.u.reg32.data = *((int*)pData);
				pos->VarDict.val.u.regx.data_expr = (Expression*) pData;
			} else if (strcmp(member, "addr") == 0) {
				pos->VarDict.val.u.regx.addr = *((int*) pData);
			} else if (strcmp(member, "misk") == 0) {
				pos->VarDict.val.u.regx.misk = *((int*) pData);
			} else {
				fprintf(stderr,
						"reg32 member is wrong. allow addr,data, misk\n");
				exit(1);
			}

			break;
		}

	}

	//error check
	if (error_flg) {
		fprintf(stderr, "set_reg_varialbe()\n");
		exit(1);
	}


}

VALUE get_variable_from_list(char* name, int line_num)
{


    VALUE val;

    VarDictList* pos;


    for (pos = g_VarDictList; pos != NULL; pos = pos->next) {

    	if (strcmp(pos->VarDict.name, name) == 0) {

    	    return pos->VarDict.val;

    	}
    }


    printf("error near line: %d, undefined %s\n", line_num, name);
    exit(1);
    

    //not reached
    return val;

}


VALUE* search_relative_reg_from_list(char* name)
{


	VALUE* ret_val = NULL;

	strcpy(g_search_str, name);


	VarDictList* pos;

	for (pos = g_VarDictList; pos != NULL; pos = pos->next) {
		if (pos->VarDict.val.type == VARIABLE_REGX) {

			g_search_bingo = 0;

			//関数内で g_search_bingo を操作している。あまりよくない
			eval_expression(pos->VarDict.val.u.regx.data_expr);

			if (g_search_bingo) {
				//printf("****** bingo ********\n");
				//printf("%s, %s, 0x%08X\n",g_search_str, g_variable_list[i].name, temp);
				ret_val = &(pos->VarDict.val);
				break;
			}

		}
	}


	return ret_val;

}


void execute_statement(Statement* statement)
{

    printf("\nstart execute_statement\n");

    switch (statement->type) {
    case EXPRESSION_STATEMENT:
        printf("EXPRESSION_STATEMENT\n");
        eval_expression(statement->u.expr_s);
        break;
    
    default: 
        printf("execute_statement error\n");
    }

}


void execute_statement_list(StatementList* list)
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

char* alloc_string(char *str)
{
    char* new_str;
    
    new_str = malloc(strlen(str) + 1);
    
    strcpy(new_str, str);

    return new_str;
}
