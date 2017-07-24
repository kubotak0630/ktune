#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ktu_create.h"

extern int g_line_number;

int g_search_bingo = 0;
char g_search_str[64];

/*** prototype declare ***********************/
VALUE get_variable_from_list(char* name, int line_num);
//VALUE eval_expression(Expression* expr);
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

/* 戻り値は0を返す if (a = 5) は0を返すことにする */
int eval_assign_expression(char* ident, int is_register_flg, Expression* expr)
{

    char str0[128];
    char str_mem[16];


    int ret_val = 0;

    VALUE v;

    //INT型 or STRING 変数への代入
    if (is_register_flg == 0) {

    	//ret_val = eval_expression(expr).u.long_val;
    	v = eval_expression(expr);

    	add_variable_list(ident, &v);

    }
    //REGX型のメンバ変数への代入
    else {

    	//hoge.dataを str0="hoge2, str_mem="data"と分離する
		str_split(ident, str0, str_mem);

		//printf("bunri_ident = %s\n", str0);
		//printf("mem = %s\n", str_mem);

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


    //printf("eval_assign_expr: %s\n", ident);
    

    return ret_val;

}


/***********************************************************
 * $reg32 hoge
 * hoge.addr = aaa
 * hoge.data = bbb
 *
 * と記述した時と同じになるように
 * 宣言 ktu_create_declare_expression()
 * 代入 ktu_create_assign_expression() をコールしている
 ************************************************************/
VALUE eval_assign_struct_init_expression(AssignStructInitExpr* sturct_init_expr)
{
	VALUE v;

	ExprList* pos;
	Expression* expr;
	char* str_member;




	//エラーのライン数を無理矢理あわせる。暫定対策
	//g_line_number = sturct_init_expr->expr_list->expression->line_number;

	//変数を宣言
	expr = ktu_create_declare_expression(sturct_init_expr->str_name, sturct_init_expr->reg_type);
	eval_expression(expr);

	//メンバを代入
	for (pos = sturct_init_expr->expr_list; pos != NULL; pos = pos->next) {

		//v = eval_expression(pos->expression);
		printf("mem = %s\n", pos->expression->u.assign_expr.variable);

		//メンバーのエラーチェック
		str_member = pos->expression->u.assign_expr.variable;
		if (strcmp(str_member, ".data") != 0 && strcmp(str_member, ".addr") != 0 && strcmp(str_member, ".misk") != 0) {

			fprintf(stderr, "member should data or addr or misk\n");
			fprintf(stderr, "error line near %d\n", pos->expression->line_number);
			exit(1);
		}

		//.dataを hoge.dataに変換
		char* str_temp;


		str_temp = malloc(strlen(sturct_init_expr->str_name)+ strlen(str_member) + 1);
		strcpy(str_temp, sturct_init_expr->str_name);
		strcat(str_temp, str_member);


		expr = ktu_create_assign_expression(str_temp, pos->expression->u.assign_expr.operand, 1);
		eval_expression(expr);

	}


	return v;


}

/**  変数の参照時に呼ばれる。 listから変数を探して値を返す ***/
VALUE eval_identifier_expression(Expression* expr)
{

	VALUE val;
	val = get_variable_from_list(expr->u.ident, expr->line_number);

	VALUE ret_val;  ////戻り値はINTかSTRING とする。(widget型はshiftを計算した値のINTで返す)

	if (val.type == VARIABLE_INT) {

		ret_val.type = VARIABLE_INT;
		ret_val.u.long_val = val.u.long_val;
	}

	else if (val.type == VARIABLE_STRING) {
		ret_val.type = VARIABLE_STRING;
		ret_val.u.str = val.u.str;
	}

	else if (val.type == VARIABLE_WIDGET) {
		ret_val.type = VARIABLE_INT;
		ret_val.u.long_val = ((uint64_t)(val.u.widget.val) << val.u.widget.shift);


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


	if (widgetData.min == 0 && widgetData.max == 1 && expr->u.scale_widget_assign_expr.auto_flg == 0) {
		widgetData.type = BUTTON_WIDGET;
	}
	else {
		widgetData.type = SCALE_WIDGET;
	}

	VALUE v;
	v.type = VARIABLE_WIDGET;
	v.u.widget = widgetData;


	//scale型変数の作成
	add_variable_list(expr->u.scale_widget_assign_expr.str_name, &v);

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

	VALUE v;
	v.type = VARIABLE_WIDGET;
	v.u.widget = widgetData;

	//scale型変数の作成
	add_variable_list(expr->u.scale_widget_assign_expr.str_name, &v);

	return 0;

}


int eval_enum_widet_assign_expression(Expression* expr)
{


	Widget widgetData;
	widgetData.type = expr->u.enum_widget_assign_expr.widget_type;

	widgetData.list_size = expr->u.enum_widget_assign_expr.list_size;

	widgetData.str_list = malloc(sizeof(char*) * widgetData.list_size);


	//exprを評価して文字列をlistを作成
	ExprList* pos;

	int i = 0;
	int def_flg = 0;
	VALUE eval_value;

	for (pos = expr->u.enum_widget_assign_expr.expr_list; pos != NULL; pos = pos->next) {


		eval_value = eval_expression(pos->expression);

		//整数値はdefault指定と判断
		if (eval_value.type == VARIABLE_INT) {

			widgetData.def = eval_value.u.long_val;
			def_flg = 1;
			break;
		}
		else if (eval_value.type == VARIABLE_STRING) {
			widgetData.str_list[i++] = eval_value.u.str;
		}
		else {
			fprintf(stderr, "error eval_enum_widet_assign_expression, line near %d\n", pos->expression->line_number);
			exit(1);
		}
	}

	widgetData.list_size = i;

	//デフォルトの指定がない時
	if (def_flg == 0) {

		widgetData.def = 0;
	}

	widgetData.val = widgetData.def; //ここではvalの値はdefをいれておく



	VALUE v;
	v.type = VARIABLE_WIDGET;
	v.u.widget = widgetData;

	//scale型変数の作成
	add_variable_list(expr->u.enum_widget_assign_expr.str_name, &v);

	return 0;


}

//linuxではitoaが使えないので自作(Webからコピペ)
char *itoa(int val, char *a, int radix) {
	char *p = a;
	unsigned int v = val;/* 作業用(変換対象の値) */
	int n = 1;/* 変換文字列の桁数記憶用 */
	while (v >= radix) {/* 桁数を求める */
		v /= radix;
		n++;
	}
	p = a + n; /* 最下位の位置から設定する */
	v = val;
	*p = '\0';/* 文字列終端の設定 */
	do {
		--p;
		*p = v % radix + '0';/* 1桁の数値を文字に変換 */
		if (*p > '9') {/* 変換した文字が10進で表現できない場合 */
			*p = v % radix - 10 + 'A'; /* アルファベットを使う */
		}
		v /= radix;
	} while (p != a);
	return a;
}


/** INT とSTRINGに対応 *****/
VALUE eval_add_expression(Expression *expr_left, Expression* expr_right) {

	VALUE ret_val, left_val, right_val;

	left_val = eval_expression(expr_left);
	right_val = eval_expression(expr_right);

	if (left_val.type == VARIABLE_INT && right_val.type == VARIABLE_INT) {
		ret_val.type = VARIABLE_INT;
		ret_val.u.long_val = left_val.u.long_val + right_val.u.long_val;
	}
	else if (left_val.type == VARIABLE_STRING && right_val.type ==VARIABLE_STRING) {
		ret_val.type = VARIABLE_STRING;

		char* new_str = malloc(strlen(left_val.u.str) + strlen(right_val.u.str) + 1);

		strcpy(new_str, left_val.u.str);
		strcat(new_str, right_val.u.str);

		ret_val.u.str = new_str;

	}
	//文字列と整数型の足し算は整数型を文字列に自動的にCASTする仕様
	else {

		ret_val.type = VARIABLE_STRING;

		char buffer[33];
		char* new_str;

		if (left_val.type == VARIABLE_INT) {
			itoa(left_val.u.long_val, buffer, 10);
			new_str = malloc(strlen(buffer) + strlen(right_val.u.str) + 1);

			strcpy(new_str, buffer);
			strcat(new_str, right_val.u.str);

		} else {
			itoa(right_val.u.long_val, buffer, 10);
			new_str = malloc(strlen(buffer) + strlen(left_val.u.str) + 1);

			strcpy(new_str, left_val.u.str);
			strcat(new_str, buffer);
		}

		ret_val.u.str = new_str;
	}

	return ret_val;
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

    VALUE left_val;
    VALUE right_val;

    VALUE v;

    char* str_name;

    //char str[32];
    //debug_convert_expr_type(expr->type, str);
    //printf("eval_expr: %s\n", str);

    switch(expr->type) {
    case INT_EXPRESSION:
    	v.type = VARIABLE_INT;
    	v.u.long_val = expr->u.int_value;
        return v;

    case STRING_EXPRESSION:
    	//printf("STRING_EXPRESSION = %s\n", expr->u.ident);
    	v.type = VARIABLE_STRING;
    	v.u.str = expr->u.ident;
    	return v;

    //変数を参照した時の動作 変数リストをサーチして値を返す
    case IDENT_EXPRESSION:
    	//v.type = VARIABLE_INT;
    	//v.u.long_val = eval_identifier_expression(expr);
    	v = eval_identifier_expression(expr);
        return v;

    case WIDGET_EXPRESSION:

    	return v;

    //INT型とSTRING型の両方がここ 戻り値のvはほとんど意味がないが代入した値を返す仕様とする
    case ASSIGN_EXPRESSION:
    	//v.type = VARIABLE_INT;
    	v.u.long_val = eval_assign_expression(expr->u.assign_expr.variable, expr->u.assign_expr.is_register_flg, expr->u.assign_expr.operand);
    	return v;

    case ASSIGN_STRUCT_INIT_EXPRESSION:

    	eval_assign_struct_init_expression(&(expr->u.assign_struct_init_expr));
    	return v;


    case ADD_EXPRESSION:

        v = eval_add_expression(expr->u.binary_expr.left, expr->u.binary_expr.right);

        return v;

    case SUB_EXPRESSION:

        left_val = eval_expression(expr->u.binary_expr.left);
        right_val = eval_expression(expr->u.binary_expr.right);

        v.type = VARIABLE_INT;
        v.u.long_val = left_val.u.long_val - right_val.u.long_val;
        return v;


    case MUL_EXPRESSION:

        left_val = eval_expression(expr->u.binary_expr.left);
        right_val = eval_expression(expr->u.binary_expr.right);

        v.type = VARIABLE_INT;
        v.u.long_val = left_val.u.long_val * right_val.u.long_val;
        return v;


    case DIV_EXPRESSION:

        left_val = eval_expression(expr->u.binary_expr.left);
        right_val = eval_expression(expr->u.binary_expr.right);

        v.type = VARIABLE_INT;
        v.u.long_val = left_val.u.long_val / right_val.u.long_val;
        return v;



    case BIT_AND_EXPRESSION:
    	left_val = eval_expression(expr->u.binary_expr.left);
	    right_val = eval_expression(expr->u.binary_expr.right);

	    v.type = VARIABLE_INT;
	    v.u.long_val = left_val.u.long_val & right_val.u.long_val;
	    return v;

    case BIT_OR_EXPRESSION:
    	left_val = eval_expression(expr->u.binary_expr.left);
 		right_val = eval_expression(expr->u.binary_expr.right);

 		v.type = VARIABLE_INT;
 		v.u.long_val = left_val.u.long_val | right_val.u.long_val;
 		return v;


    case BIT_L_SHIFT_EXPRESSION:
        left_val = eval_expression(expr->u.binary_expr.left);
     	right_val = eval_expression(expr->u.binary_expr.right);

     	v.type = VARIABLE_INT;
     	v.u.long_val = left_val.u.long_val << right_val.u.long_val;
     	return v;


    case BIT_R_SHIFT_EXPRESSION:
        left_val = eval_expression(expr->u.binary_expr.left);
        right_val = eval_expression(expr->u.binary_expr.right);

        v.type = VARIABLE_INT;
        v.u.long_val = left_val.u.long_val >> right_val.u.long_val;
        return v;


    case EQ_EXPRESSION:

    	left_val = eval_expression(expr->u.binary_expr.left);
    	right_val = eval_expression(expr->u.binary_expr.right);

    	v.type = VARIABLE_INT;

    	if (left_val.type != right_val.type) {
    		fprintf(stderr, "error EQ_EXPRESSION, TYPE Different left and rigth\n");
    		exit(1);
    	}
    	else if (left_val.type == VARIABLE_INT) {
    		v.u.long_val = (left_val.u.long_val == right_val.u.long_val) ;
    	}
    	else if (left_val.type == VARIABLE_STRING) {

    		v.u.long_val = (strcmp(left_val.u.str, right_val.u.str) == 0) ? 1 : 0;
    	}

		return v;



    case MINUS_EXPRESSION:

    	v.type = VARIABLE_INT;
    	v.u.long_val = eval_minus_expression(expr->u.operand_expr);
    	return v;



    case REGX_DECLARE:
    	/*** $data32 data_hoge という特殊変数の宣言 *********
    	* 変数リストに変数を追加。データの値はセットしない。 */
    	//add_variable_list(expr->u.reg_declare_expr.ident, NULL, VARIABLE_REGX, expr->u.reg_declare_expr.regType);

    	v.type = VARIABLE_REGX;
    	v.u.regx.reg_type = expr->u.reg_declare_expr.regType;

    	add_variable_list(expr->u.reg_declare_expr.ident, &v);

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

    	v.type = PAGE;

    	str_name = eval_expression(expr->u.operand_expr).u.str;

    	printf("---%s \n", str_name);


    	add_variable_list(str_name, &v);

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
void add_variable_list(char* name, VALUE* pVal)
{


	VarDictList* node;


	node = malloc(sizeof(VarDictList));


	node->VarDict.val.type = pVal->type;


	//変数名の領域の確保してセット
	int str_size = strlen(name) + 1;
	node->VarDict.name = (char*) malloc(str_size);
	strcpy(node->VarDict.name, name);

	/*** 変数の値をセット ***************************/
	if (pVal->type == VARIABLE_INT) {

		node->VarDict.val.u.long_val = pVal->u.long_val;
	}
	else if (pVal->type == VARIABLE_STRING) {

		node->VarDict.val.u.str = pVal->u.str;
	}
	else if (pVal->type == VARIABLE_WIDGET) {

		//Widget* p = (Widget*) pData;

		//共通の要素
		node->VarDict.val.u.widget.type = pVal->u.widget.type;
		node->VarDict.val.u.widget.val = pVal->u.widget.val;
		node->VarDict.val.u.widget.def = pVal->u.widget.def;

		//SCALE Widget
		if (pVal->u.widget.type == SCALE_WIDGET || pVal->u.widget.type == BUTTON_WIDGET || pVal->u.widget.type == SPIN_WIDGET) {

			//構造体のコピー
			node->VarDict.val.u.widget.min = pVal->u.widget.min;
			node->VarDict.val.u.widget.max = pVal->u.widget.max;
			node->VarDict.val.u.widget.shift = pVal->u.widget.shift;

		}
		else if (pVal->u.widget.type == COMBO_WIDGET || pVal->u.widget.type == RADIO_WIDGET) {
			node->VarDict.val.u.widget.str_list = pVal->u.widget.str_list;
			node->VarDict.val.u.widget.list_size = pVal->u.widget.list_size;
		}
		else {

			printf("error add_vbariable_list(), widget typte unknow\n");
			exit(1);

		}

	}
	//REG32,16,64	は変数の宣言と代入が別のため、データのセットはeval_assign_expressionで実行
	else if (pVal->type == VARIABLE_REGX) {
		//ポインタをNULLで初期化しておく
		node->VarDict.val.u.regx.data_expr = NULL;
		node->VarDict.val.u.regx.p_gtk_label = NULL;

		node->VarDict.val.u.regx.reg_type = pVal->u.regx.reg_type;

		//not data setting

	}
	else if (pVal->type == PAGE) {
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
		node->next = NULL;
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
		node->next = NULL;
		pos->next = node;
	}
	//上書き(古い変数を削除して入れ替える)
	else {

		//先頭の要素
		if (pos->prev == NULL) {

			g_VarDictList = node;
			node->prev = NULL;
			node->next = NULL;

			free(pos); //古いNodeを削除
		}
		else {
			pos->prev->next = node;  //New Nodeをつなぐ
			node->prev = pos->prev;
			node->next = pos->next;

			free(pos);
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


char* alloc_string(char *str)
{
    char* new_str;
    
    new_str = malloc(strlen(str) + 1);
    
    strcpy(new_str, str);

    return new_str;
}
