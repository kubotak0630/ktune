#ifndef __KTU_CREATE_H
#define __KTU_CREATE_H

#include <stdint.h>



struct Expression_tag;

/**** Variable *****************/
//変数の型を定義
typedef enum {
    VARIABLE_INT = 1,  //int64_t
	VARIABLE_STRING,
	VARIABLE_REGX,
	VARIABLE_WIDGET,
	PAGE,
} ValType;


typedef enum {
    REG16  = 1,
    REG32  = (1 << 1),
	REG64  = (1 << 2),
	REG16B = (1 << 3),
	REG32B = (1 << 4),
	REG64B = (1 << 5),
} RegType;


typedef struct {
	RegType reg_type;
    uint32_t addr;
    struct Expression_tag* data_expr;  //構文木のアドレスを保持する
    int32_t misk;
    void* p_gtk_label;
    void* p_gtk_button;  //REGXXR の時のみ使用。ボタンのアドレス
    int64_t data_buf;
} RegX;


//widgetの種類
typedef enum {
	SCALE_WIDGET = 1,
	SPIN_WIDGET,
	BUTTON_WIDGET,
	COMBO_WIDGET,
	RADIO_WIDGET,

}widgetType;



typedef struct {
	widgetType type;
	int min;
	int max;
	int def;
	char** str_list;  //use enum, radio widet
	int list_size;   //use enum, radio widet
	int shift;
	int val;
	void* p_gtk_self;   //自分自信のgtk_widget のアドレスを格納
	void* p_gtk_init_button;  //初期化ボタンのアドレス
}Widget;



typedef struct VALUE_tag{
    ValType type;
    union {
        int64_t long_val;
        char* str;
        RegX regx;
        Widget widget;
    } u;

}VALUE;

typedef struct {

	char *name;
	VALUE val;

}Varialbe_Dict;



typedef struct VarDictList_tag {

	Varialbe_Dict VarDict;
	struct VarDictList_tag* next;
	struct VarDictList_tag* prev;

}VarDictList;



typedef enum {
    INT_EXPRESSION = 1,
	STRING_EXPRESSION,  //"hoge"
    IDENT_EXPRESSION, //  変数を参照した時
	WIDGET_EXPRESSION, //[0,10,0,0]
    ASSIGN_EXPRESSION,  // "="
	ASSIGN_STRUCT_INIT_EXPRESSION,   //構造体の初期化
    ADD_EXPRESSION,     // "+"
    SUB_EXPRESSION,     // "-"
    MUL_EXPRESSION,     // "*"
    DIV_EXPRESSION,      // "/"
	BIT_AND_EXPRESSION,  // "&"
	BIT_OR_EXPRESSION,  // "|"
	EQ_EXPRESSION,      // "=="
	BIT_L_SHIFT_EXPRESSION,  // "<<"
	BIT_R_SHIFT_EXPRESSION,  // ">>"
    MINUS_EXPRESSION,     //-5, -(6+7)
	REGX_DECLARE,
	SCALE_WIDGET_ASSIGN_EXPRESSION,
	SPIN_WIDGET_ASSIGN_EXPRESSION,
	ENUM_WIDGET_ASSIGN_EXPRESSION,   //combo or radio widget
	PAGE_EXPRESSION         //gtk page
} ExprType;





typedef struct {
    struct Expression_tag *left;
    struct Expression_tag *right;
} BinaryExpr;


typedef struct {
	int is_register_flg;  //REGXへのメンバへの代入はこのflgが1
    char *variable;
    struct Expression_tag *operand;
} AssignExpr;


typedef struct {
	char *str_name;
	struct Expression_tag *expr_min;
	struct Expression_tag *expr_max;
	struct Expression_tag *expr_def;
	struct Expression_tag *expr_shift;
} ScaleWidgetAssignExpr;




typedef struct ExprList_tag {

	struct Expression_tag *expression;
    struct ExprList_tag *next;

} ExprList;

typedef struct {
	widgetType widget_type;  //COMBO_WIDGET or RADIO_WIDGET
	char *str_name;
	int list_size;
	ExprList* expr_list;
	int def;

} EnumWidgetAssignExpr;


typedef struct {
	char* str_name;
	RegType reg_type;
	ExprList* expr_list;
} AssignStructInitExpr;


typedef struct {
	char *ident;
	RegType regType;
} RegDeclareExpr;

typedef struct Expression_tag {
    ExprType type;
    int line_number;

    union {
        int int_value;
        char* ident;
        AssignExpr assign_expr;
        BinaryExpr binary_expr;
        struct Expression_tag *operand_expr;
        ScaleWidgetAssignExpr scale_widget_assign_expr;
        EnumWidgetAssignExpr enum_widget_assign_expr;
        RegDeclareExpr reg_declare_expr;
        AssignStructInitExpr assign_struct_init_expr;

    } u;

}Expression;

/***** Statement *************************************/
struct Statement_tag;  //前方参照

typedef enum {
    EXPRESSION_STATEMENT = 1,
	IF_STATEMENT
} StatementType;

typedef struct StatementList_tag {

	struct Statement_tag *statement;
    struct StatementList_tag *next;

} StatementList;

typedef struct Elsif_tag {
	Expression* condition;
	StatementList *block;
	struct Elsif_tag *next;
} Elsif;


typedef struct {
	Expression *condition;
	StatementList *true_stmt_list;
	Elsif *elsif;
	StatementList *else_stmt_list;
} IfStatement;


typedef struct Statement_tag{

    StatementType type;
    int line_number;

    union {
        Expression *expr_s;
        IfStatement if_s;
    } u;

} Statement;







/*** ProtoType declare **************************/
//void add_variable_list(char* name, void* pData, ValType type, RegType reg_type);
void add_variable_list(char* name, VALUE* pVal);

Expression* ktu_create_int_expression(ExprType type, int val);
Expression* ktu_create_string_expression(char* str);
Expression* ktu_create_binary_expression(ExprType operator,
                                         Expression* left, Expression* right);
Expression* ktu_create_assign_expression(char* variable, Expression* operand, int is_register_flg);
Expression* ktu_create_identifier_expression(char* str);
Expression* ktu_create_minus_expression(Expression* operand);
Expression* ktu_create_declare_expression(char* str, RegType type);
Expression* ktu_create_assign_scale_widget(char* ident, Expression* a1, Expression* a2, Expression* a3, Expression* a4);
Expression* ktu_create_assign_spin_widget(char* ident, Expression* a1, Expression* a2, Expression* a3, Expression* a4);
Expression* ktu_create_assign_enum_widget(ExprList* expr_list ,char* ident, widgetType type);
Expression* ktu_create_sturct_init_assign_expression(ExprList* expr_list, char* ident, RegType reg_type);

Statement* ktu_create_if_statement(Expression* condition, StatementList* true_stmt_list, Elsif* elsif, StatementList* else_stmt_list);
Elsif* ktu_create_elsif(Expression *expr, StatementList* stmt_list);
Elsif* ktu_chain_elsif_list(Elsif* list, Elsif* add);


Expression* ktu_create_page(Expression* expr_ope);


Statement* ktu_create_expression_statement(Expression* expr);
StatementList* ktu_create_statement_list(Statement* statement);

StatementList* ktu_chain_statement_list(StatementList* list, Statement* statement);

ExprList* ktu_create_expression_list(Expression* expr);

ExprList* ktu_chain_expression_list(ExprList* expr_list,Expression* expr);

VALUE eval_expression(Expression* expr);

/*** temp 後で関数ポインタをメンバにした方がいい***/
int eval_assign_expression(char* ident, int is_register_flg, Expression* expr);

void ktu_execute_statement_list(StatementList* list);

void increment_line_number();

char* alloc_string(char *str);




#endif
