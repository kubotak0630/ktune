#ifndef __KTU_CREATE_H
#define __KTU_CREATE_H

#include <stdint.h>


struct Expression_tag;

/**** Variable *****************/
//変数の型を定義
typedef enum {
    VARIABLE_INT = 1,
	VARIABLE_LONG,
	VARIABLE_STRING,
	VARIABLE_REGX,
	VARIABLE_WIDGET,
	PAGE,
} ValType;


typedef enum {
    REG16  = 1,
    REG32  = (1 << 1),
	REG64  = (1 << 2),
	REG16R = (1 << 3),
	REG32R = (1 << 4),
	REG64R = (1 << 5),
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
        int  int_val;
        uint64_t long_val;
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
    IDENT_EXPRESSION, // variable "a" 
	WIDGET_EXPRESSION, //[0,10,0,0]
    ASSIGN_EXPRESSION,  // "="
    ADD_EXPRESSION,     // "+"
    SUB_EXPRESSION,     // "-"
    MUL_EXPRESSION,     // "*"
    DIV_EXPRESSION,      // "/"
	BIT_AND_EXPRESSION,  // "&"
	BIT_OR_EXPRESSION,  // "|"
	BIT_L_SHIFT_EXPRESSION,  // "<<"
	BIT_R_SHIFT_EXPRESSION,  // ">>"
    MINUS_EXPRESSION,     //-5, -(6+7)
	REGX_DECLARE,
	SCALE_WIDGET_ASSIGN_EXPRESSION,
	SPIN_WIDGET_ASSIGN_EXPRESSION,
	ENUM_WIDGET_ASSIGN_EXPRESSION,   //combo or radio widget
	PAGE_EXPRESSION         //gtk page
} ExprType;


//struct Expression_tag;


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

/*
typedef enum {
	COMBO_ENUM = 1,
	RADIO_ENUM
}EnumWidgetType;
*/
typedef struct {
	widgetType widget_type;  //COMBO_WIDGET or RADIO_WIDGET
	char *str_name;
	int list_size;
	char* len_val_list[20];   //要素の最大は20とする
	int def;

} EnumWidgetAssignExpr;


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
        struct Expression_tag *minus_expr;
        ScaleWidgetAssignExpr scale_widget_assign_expr;
        EnumWidgetAssignExpr enum_widget_assign_expr;
        RegDeclareExpr reg_declare_expr;

    } u;

}Expression;


typedef enum {
    EXPRESSION_STATEMENT = 1,

} StatementType;

typedef struct Statement_tag {

    StatementType type;
    int line_number;

    union {
        Expression *expr_s;
    } u;

} Statement;

typedef struct StatementList_tag {

    Statement *statement;
    struct StatementList_tag *next;

} StatementList;



/*** ProtoType declare **************************/
void add_variable_list(char* name, void* pData, ValType type, RegType reg_type);

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
Expression* ktu_create_assign_enum_widget(char* ident, widgetType type);



Expression* ktu_create_page(char* ident);


Statement* ktu_create_expression_statement(Expression* expr);
StatementList* ktu_create_statement_list(Statement* statement);

void ktu_chain_statement_list(StatementList* list, Statement* statement);

void ktu_create_valiable_length_val(Expression* expr);

void ktu_add_valiable_length_val(Expression* expr);

/*** temp 後で関数ポインタをメンバにした方がいい***/
int eval_assign_expression(char* ident, int is_register_flg, Expression* expr);

void execute_statement_list(StatementList* list);

void increment_line_number();

char* alloc_string(char *str);




#endif
