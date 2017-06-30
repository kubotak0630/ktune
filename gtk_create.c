#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "ktu_create.h"
#include "gtk_create.h"


extern VarDictList* g_VarDictList;
extern int g_debug;
GtkWidget *window;


extern VALUE eval_expression(Expression* expr);
extern VALUE* search_relative_reg_from_list(char* name);


//ProtoType declare
static void create_gtk_page();
static void create_regx(Varialbe_Dict* val_dict);
static void create_widget(Varialbe_Dict* val_dict);
static void change_widget_value(Varialbe_Dict* dict);
static void set_default_page(int page_num);


void window_init(int argc, char** argv, int size_x, int size_y) {

	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_container_set_border_width(GTK_CONTAINER(window), 10);

	gtk_widget_set_size_request(window, size_x, size_y);

	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
}

void widget_show_all() {
	gtk_widget_show_all(window);

}

void set_reg32(uint32_t addr, uint32_t data, int32_t misk)
{

	//printf("******* Call set_reg32 ********\n");
	printf("addr = 0x%08X, data = 0x%08X, misk = %d\n", addr, data, misk);

}

void set_reg16(uint32_t addr, uint32_t data, int32_t misk)
{

	//printf("******* Call set_reg16 ********\n");
	printf("addr = 0x%08X, data = 0x%08X, misk = %d\n", addr, data, misk);

}

void set_reg64(uint32_t addr, uint64_t data, int32_t misk)
{

	//printf("******* Call set_reg64 ********\n");
	printf("addr = 0x%08X, data = 0x%016lX, misk = %d\n", addr, data, misk);

}

static void change_widget_value(Varialbe_Dict* dict) {

	VALUE* pVal = search_relative_reg_from_list(dict->name);

	if (pVal != NULL) {
		char str_temp[256];
		char str_temp_l[128];

		if (pVal->u.regx.reg_type == REG32) {

			int32_t eval_data = eval_expression(pVal->u.regx.data_expr).u.long_val;

			sprintf(str_temp, "0x%08X", eval_data);

			set_reg32(pVal->u.regx.addr, eval_data, pVal->u.regx.misk);

		} else if (pVal->u.regx.reg_type == REG32R) {


			int32_t eval_data = eval_expression(pVal->u.regx.data_expr).u.long_val;

			if (eval_data == pVal->u.regx.data_buf) {
				sprintf(str_temp, "  0x%08X", eval_data);
			} else {
				sprintf(str_temp, "*0x%08X", eval_data);
			}
		} else if (pVal->u.regx.reg_type == REG64) {
			uint64_t eval_data = eval_expression(pVal->u.regx.data_expr).u.long_val;
			sprintf(str_temp, "0x%08X-", (uint32_t) (eval_data >> 32));
			sprintf(str_temp_l, "%08X", (uint32_t) (eval_data & 0xFFFFFFFF));
			strcat(str_temp, str_temp_l);

			set_reg64(pVal->u.regx.addr, eval_data, pVal->u.regx.misk);

		} else if (pVal->u.regx.reg_type == REG64R) {

			uint64_t eval_data = eval_expression(pVal->u.regx.data_expr).u.long_val;

			sprintf(str_temp_l, "%08X", (uint32_t) (eval_data & 0xFFFFFFFF));

			if (eval_data == pVal->u.regx.data_buf) {
				sprintf(str_temp, "  0x%08X-", (uint32_t) (eval_data >> 32));
			} else {
				sprintf(str_temp, "*0x%08X-", (uint32_t) (eval_data >> 32));
			}

			strcat(str_temp, str_temp_l);
		}

		gtk_label_set_text(GTK_LABEL(pVal->u.regx.p_gtk_label), str_temp);
	}

}

static void cb_scale_changed(GtkScale* scale, gpointer user_data)
{

	Varialbe_Dict* dict = (Varialbe_Dict*)user_data;

    //変数リストの値を更新
	dict->val.u.widget.val = (int)gtk_range_get_value(GTK_RANGE(scale));

    //printf("call cb_scale_chagend\n");

    change_widget_value(dict);

}

/** widgetの左側のボタンが押された時にwidgetの値をdefault値に戻す **/
static void cb_def_button_clicked(GtkButton* button, gpointer user_data)
{

	Varialbe_Dict* dict = (Varialbe_Dict*)user_data;


	if (dict->val.u.widget.type == BUTTON_WIDGET) {
		GtkButton* button = (GtkButton*)(dict->val.u.widget.p_gtk_self);

		char str_new_val[2];
		sprintf(str_new_val, "%d", dict->val.u.widget.def);

		//default値セット
		gtk_button_set_label(button, str_new_val);

		//変数リストの値が変更がある場合は更新
		//ボタンだけが他と違う。状態を持っていないため。自分で変更があったかをチェック
		if (dict->val.u.widget.val != dict->val.u.widget.def) {
			dict->val.u.widget.val = dict->val.u.widget.def;
			change_widget_value(dict);
		}
	}
	else if (dict->val.u.widget.type == SCALE_WIDGET){
		GtkScale* scale = (GtkScale*)(dict->val.u.widget.p_gtk_self);
		gtk_range_set_value(GTK_RANGE(scale), dict->val.u.widget.def);

		//gtk_range_set_value() をよぶことで値が変更してcb_scale_changed が呼ばれるので
		//ボタンWidgetのように手動で値の変更を判定する必要はない
	}
	else if (dict->val.u.widget.type == COMBO_WIDGET){
		GtkComboBox* combo = (GtkComboBox*)(dict->val.u.widget.p_gtk_self);
		gtk_combo_box_set_active(combo, dict->val.u.widget.def);

	}
	else if (dict->val.u.widget.type == RADIO_WIDGET) {
		GtkToggleButton* rbutton = (GtkToggleButton*)(dict->val.u.widget.p_gtk_self);
		gtk_toggle_button_set_active(rbutton, TRUE);
	}
	else {

		fprintf(stderr, "error cb_def_button_clicked, unknown widget\n");
		exit(1);
	}

}




/** ボタンwidget(min=0, max=1)のボタンが押された時のhandler  ************************/
static void cb_val_button_clicked(GtkButton* button, gpointer user_data)
{

	Varialbe_Dict* dict = (Varialbe_Dict*)user_data;

	//変数リストの値を更新
	const char *str_val;
	str_val = gtk_button_get_label(button);

	//0,1を反転する
	int new_val = (atoi(str_val) == 0) ? 1 : 0;

	char str_new_val[2];
	sprintf(str_new_val, "%d", new_val);

	//新しい値(文字列)をボタンにセット
	gtk_button_set_label(button, str_new_val);

	//変数リストの値を更新
	dict->val.u.widget.val = new_val;

	change_widget_value(dict);


}


static void cb_changed_combo(GtkComboBox* widget, gpointer user_data) {

	Varialbe_Dict* dict = (Varialbe_Dict*)user_data;

	//変数リストの値を更新
	dict->val.u.widget.val = (int)gtk_combo_box_get_active(widget);

	gtk_combo_box_get_active(widget);

	change_widget_value(dict);

}


static void cb_radio_button_toggle(GtkWidget* widget, gpointer user_data) {

	Varialbe_Dict* dict = (Varialbe_Dict*)user_data;

	gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

	int val = 0;
	int i;

	//押されたボタンのみアクション
	if (active) {

		//ラベルを値に変換
		const char* str = gtk_button_get_label(GTK_BUTTON(widget));
		for (i = 0; i < dict->val.u.widget.list_size; i++) {
			if (strcmp(str, dict->val.u.widget.str_list[i]) == 0) {
				val = i;
				break;
			}
		}

		//変数リストの値を更新
		dict->val.u.widget.val = val;

		change_widget_value(dict);

		//printf("callcb_radio_button_toggle, %s, : %d\n", str, val);
	}
}


static void set_register(uint32_t addr, uint64_t data, int32_t misk, RegType reg_type) {

	if (reg_type & (REG32 | REG32R)) {
		set_reg32(addr, data, misk);
	} else if (reg_type & (REG64 | REG64R)) {
		set_reg32(addr, data, misk);
	} else if (reg_type & (REG16 | REG16R)) {
		set_reg16(addr, data, misk);
	}
}

//data32r などのREGXXR のボタンが押された時、 レジスタを送信
static void cb_regr_button_clicked(GtkButton* button, gpointer user_data)
{

	Varialbe_Dict* dict = (Varialbe_Dict*)user_data;

	VALUE v = eval_expression(dict->val.u.regx.data_expr);

	uint64_t eval_data = v.u.long_val;


	set_register(dict->val.u.regx.addr, eval_data, dict->val.u.regx.misk, dict->val.u.regx.reg_type);

	dict->val.u.regx.data_buf = eval_data;

	/*** データ表示の '*'を削除 *****/
	const char *str;
	str = gtk_label_get_text(GTK_LABEL(dict->val.u.regx.p_gtk_label));

	char str_new[128];
	int i = 1;

	if (str[0] == '*') {
		str_new[0] = ' ';
		while (str[i] != '\0') {
			str_new[i] = str[i];
			i++;
		}
		str_new[i] = '\0';

		gtk_label_set_text(GTK_LABEL(dict->val.u.regx.p_gtk_label), str_new);
	}

}

GtkWidget *vbox_top;
GtkWidget *g_vbox;
GtkCellRenderer *g_renderer = NULL;  //コンボボックスの表示で使う
GtkWidget *g_notebook;

void create_regx(Varialbe_Dict* val_dict) {

	GtkWidget *label_addr;
	GtkWidget *label_data;


	GtkWidget *hsep;

	GtkWidget *hbox;

	GtkWidget *frame;

	g_vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(g_vbox), 10);


	hbox = gtk_hbox_new(FALSE, 20);
	gtk_box_pack_start(GTK_BOX(g_vbox), hbox, FALSE, FALSE, 0);

	char str_temp[256];

	//テキストを作成
	//printf("%d\n", val_dict->val.u.regx.reg_type);
	if (val_dict->val.u.regx.reg_type & (REG16 | REG16R)) {
		sprintf(str_temp, "reg16@0x%08X = ", val_dict->val.u.regx.addr);
	}
	else if (val_dict->val.u.regx.reg_type & (REG32 | REG32R)) {
		sprintf(str_temp, "reg32@0x%08X = ", val_dict->val.u.regx.addr);
	}
	else if (val_dict->val.u.regx.reg_type & (REG64 | REG64R)) {
		sprintf(str_temp, "reg64@0x%08X = ", val_dict->val.u.regx.addr);
	}

	//ラベルを作成(即値レジスタ)
	if (val_dict->val.u.regx.reg_type & (REG16 | REG32 | REG64)) {

		label_addr = gtk_label_new(str_temp);
		val_dict->val.u.regx.p_gtk_button = NULL;
	}
	//ボタンを作成
	else if (val_dict->val.u.regx.reg_type & (REG16R | REG32R | REG64R)) {

		label_addr = gtk_button_new_with_label(str_temp);
		val_dict->val.u.regx.p_gtk_button = label_addr;

		g_signal_connect(G_OBJECT(label_addr), "clicked", G_CALLBACK(cb_regr_button_clicked), (void*)val_dict);
	}
	else {
		fprintf(stderr, "error create_regx\n");
		exit(1);
	}

	gtk_box_pack_start(GTK_BOX(hbox), label_addr, FALSE, FALSE, 0);


	//データ表示
	char str_temp_l[128];
	if (val_dict->val.u.regx.reg_type & (REG32 | REG32R)) {
		int32_t eval_data = eval_expression(val_dict->val.u.regx.data_expr).u.long_val;
		val_dict->val.u.regx.data_buf = eval_data;
		sprintf(str_temp, "0x%08X", eval_data);
	}
	else if (val_dict->val.u.regx.reg_type & (REG64 | REG64R)) {
		uint64_t eval_data = eval_expression(val_dict->val.u.regx.data_expr).u.long_val;
		sprintf(str_temp, "0x%08X-", (uint32_t)(eval_data >> 32));
		sprintf(str_temp_l, "%08X", (uint32_t)(eval_data & 0xFFFFFFFF));
		strcat(str_temp, str_temp_l);

		val_dict->val.u.regx.data_buf = eval_data;
	}
	else {
		fprintf(stderr, "create_regx error ,reg16\n");

	}

	label_data = gtk_label_new(str_temp);
	gtk_box_pack_start(GTK_BOX(hbox), label_data, FALSE, FALSE, 0);

	val_dict->val.u.regx.p_gtk_label = label_data;


	/** separator **/
	hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(g_vbox), hbox, FALSE, FALSE, 0);


	hsep = gtk_hseparator_new();
	gtk_widget_set_usize(hsep, 100, 8);
	gtk_box_pack_start(GTK_BOX(hbox), hsep, TRUE, TRUE, 0);

	/** frame ******/
	sprintf(str_temp, "%s", val_dict->name);

	frame = gtk_frame_new(str_temp);

	gtk_container_add(GTK_CONTAINER(frame), g_vbox);

	gtk_box_pack_start(GTK_BOX(vbox_top), frame, FALSE, FALSE, 0);



}


void create_widget(Varialbe_Dict* val_dict) {

	GtkWidget *hbox;
	GtkWidget *button0;
	GtkWidget *widget0;
	//GtkWidget *button_val;

	hbox = gtk_hbox_new(FALSE, 15);
	gtk_box_pack_start(GTK_BOX(g_vbox), hbox, FALSE, FALSE, 0);

	button0 = gtk_button_new_with_label(val_dict->name);

	gtk_box_pack_start(GTK_BOX(hbox), button0, TRUE, TRUE, 0);




	//Scale Widget
	if (val_dict->val.u.widget.type == SCALE_WIDGET) {
		int min = val_dict->val.u.widget.min;
		int max = val_dict->val.u.widget.max;
		int def = val_dict->val.u.widget.def;

		widget0 = gtk_hscale_new_with_range(min, max, 1);

		gtk_scale_set_value_pos(GTK_SCALE(widget0), GTK_POS_LEFT);
		gtk_range_set_value(GTK_RANGE(widget0), def);

		//自分自身のscale のアドレスを格納(ボタンが押された時にdefaultに戻すため)
		val_dict->val.u.widget.p_gtk_self = widget0;


		g_signal_connect(G_OBJECT(widget0), "value-changed",
				G_CALLBACK(cb_scale_changed), (void* )val_dict);


		gtk_box_pack_start(GTK_BOX(hbox), widget0, TRUE, TRUE, 0);

	}
	else if (val_dict->val.u.widget.type == BUTTON_WIDGET) {


		int def = val_dict->val.u.widget.def;

		char str_val[2];  //0 or 1
		sprintf(str_val, "%d", def);
		widget0 = gtk_button_new_with_label(str_val);

		//自分自身のscale のアドレスを格納(ボタンが押された時にdefaultに戻すため)
		val_dict->val.u.widget.p_gtk_self = widget0;

		g_signal_connect(G_OBJECT(widget0), "clicked",
				G_CALLBACK(cb_val_button_clicked), (void* )val_dict);


		gtk_box_pack_start(GTK_BOX(hbox), widget0, TRUE, TRUE, 0);
	}
	else if (val_dict->val.u.widget.type == COMBO_WIDGET) {
		GtkTreeIter iter;
		GtkListStore *list_store;

		list_store = gtk_list_store_new(1, G_TYPE_STRING);

		int i;

		for (i = 0; i < val_dict->val.u.widget.list_size; i++) {
			gtk_list_store_append(list_store, &iter);
			gtk_list_store_set(list_store, &iter, 0, val_dict->val.u.widget.str_list[i], -1);
		}

		widget0 = gtk_combo_box_new_with_model(GTK_TREE_MODEL(list_store));
		gtk_combo_box_set_active (GTK_COMBO_BOX(widget0), val_dict->val.u.widget.def);

		//自分自身のscale のアドレスを格納(ボタンが押された時にdefaultに戻すため)
		val_dict->val.u.widget.p_gtk_self = widget0;

	    g_signal_connect(G_OBJECT(widget0), "changed", G_CALLBACK(cb_changed_combo), (void* )val_dict);



		if (g_renderer == NULL) {
			g_renderer = gtk_cell_renderer_text_new();
		}
		gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(widget0), g_renderer, TRUE);
		gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(widget0), g_renderer, "text", 0, NULL);

		gtk_box_pack_start(GTK_BOX(hbox), widget0, TRUE, TRUE, 0);
	}
	else if (val_dict->val.u.widget.type == RADIO_WIDGET) {

		GtkWidget* vbox_radio;
		vbox_radio = gtk_vbox_new(FALSE, 0);

		GtkWidget* rbutton[20];

		int i;
		for (i = 0; i < val_dict->val.u.widget.list_size; i++) {

			if (i == 0) {
				rbutton[i] = gtk_radio_button_new_with_label(NULL, val_dict->val.u.widget.str_list[i]);
				//gtk_box_pack_start(GTK_BOX(vbox_radio), rbutton[0], FALSE, FALSE, 0);
			}
			else {
				rbutton[i] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rbutton[0]), val_dict->val.u.widget.str_list[i]);
				//gtk_box_pack_start(GTK_BOX(vbox_radio), rbutton[1], FALSE, FALSE, 0);
			}
		}

		for (i = 0; i < val_dict->val.u.widget.list_size; i++) {
			gtk_box_pack_start(GTK_BOX(vbox_radio), rbutton[i], FALSE, FALSE, 0);
		}

		//初期値を反映
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rbutton[val_dict->val.u.widget.def]), TRUE);

		//初期値を反映した後にコールバック関数を登録する。初期値反映でコールバック関数が呼ばれるのを防ぐため
		for (i = 0; i < val_dict->val.u.widget.list_size; i++) {
			g_signal_connect(G_OBJECT(rbutton[i]), "toggled", G_CALLBACK(cb_radio_button_toggle), (void* )val_dict);
		}

		//初期値のボタンWidgetのアドレスを格納(ボタンが押された時にdefaultに戻すため)
		val_dict->val.u.widget.p_gtk_self = rbutton[val_dict->val.u.widget.def];


/*
		for (i = 0; i < 3; i++) {
			gtk_box_pack_start(GTK_BOX(vbox_radio), rbutton[i], FALSE, FALSE, 0);
		}
*/
		gtk_box_pack_start(GTK_BOX(hbox), vbox_radio, TRUE, TRUE, 0);

	}
	else {

		fprintf(stderr, "error create_widget(), unknown widget\n");
		exit(1);
	}

	//widgetの左側のボタンのハンドラを登録(ボタンを押すとdefault値に戻る)
	g_signal_connect(G_OBJECT(button0), "clicked", G_CALLBACK(cb_def_button_clicked), (void* )val_dict);

}


void create_gtk_page(char* str)
{

	GtkWidget *page_label;

	vbox_top = gtk_vbox_new(FALSE, 5);


	page_label = gtk_label_new(str);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(g_notebook), GTK_POS_TOP);

	GtkWidget *scroll_window;
	scroll_window = gtk_scrolled_window_new(NULL, NULL);

	//スクロールバーは自動設定にする
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);



	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll_window), vbox_top);

	gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), scroll_window, page_label);

}


static void menu_sel_default_all(gpointer user_data)
{
	printf("%s\n", (char*)user_data);
	set_default_all();
}

static void menu_sel_default_page(gpointer user_data)
{
	printf("%s\n", (char*)user_data);

	int page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(g_notebook));
	printf("page = %d\n", page_num);

	set_default_page(page_num);

}



static GtkWidget* create_gtk_menu() {

	GtkWidget *menu_bar;
	GtkWidget *menu;
	GtkWidget *menu_items;
	GtkWidget *root_menu;

	menu_bar = gtk_menu_bar_new();

	root_menu = gtk_menu_item_new_with_label("set");

	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_menu);

	//create sub_menu

	menu = gtk_menu_new();

	menu_items = gtk_menu_item_new_with_label("set_default_all");
	gtk_menu_append(GTK_MENU(menu), menu_items);

	gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
			GTK_SIGNAL_FUNC(menu_sel_default_all), "set_default_all");

	menu_items = gtk_menu_item_new_with_label("set_default_page");
	gtk_menu_append(GTK_MENU(menu), menu_items);

	gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
				GTK_SIGNAL_FUNC(menu_sel_default_page), "set_default_page");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(root_menu), menu);

	return menu_bar;
}


void start_create_gtk_widget() {


	GtkWidget* menu_bar;
	GtkWidget* vbox_window;

	vbox_window = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox_window);

	menu_bar = create_gtk_menu();

	//メニューバーを追加
	gtk_box_pack_start(GTK_BOX(vbox_window), menu_bar, FALSE, FALSE, 0);

	//セパレータを追加
	GtkWidget *hsep;
	hsep = gtk_hseparator_new();
	gtk_widget_set_usize(hsep, 10, 10);

	gtk_box_pack_start(GTK_BOX(vbox_window), hsep, FALSE, FALSE, 0);

	g_notebook = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(vbox_window), g_notebook);



	VarDictList* pos;

	for (pos = g_VarDictList; pos != NULL; pos = pos->next) {

		if (pos->VarDict.val.type == PAGE) {
			create_gtk_page(pos->VarDict.name);
		} else if (pos->VarDict.val.type == VARIABLE_REGX) {
			printf("%s\n", pos->VarDict.name);
			create_regx(&(pos->VarDict));

		} else if (pos->VarDict.val.type == VARIABLE_WIDGET) {
			printf("%s\n", pos->VarDict.name);

			create_widget(&(pos->VarDict));
		}
	}


}

//全てのpageをdefault値に戻してレジスタを送信する
void set_default_all()
{

	VarDictList* pos;

	for (pos = g_VarDictList; pos != NULL; pos = pos->next) {
		//if (pos->VarDict.val.type == VARIABLE_REGX) {
		if (pos->VarDict.val.type == VARIABLE_WIDGET) {

			pos->VarDict.val.u.widget.val = pos->VarDict.val.u.widget.def;

			if (pos->VarDict.val.u.widget.type == SCALE_WIDGET){
				GtkScale* scale = (GtkScale*)(pos->VarDict.val.u.widget.p_gtk_self);
				gtk_range_set_value(GTK_RANGE(scale), pos->VarDict.val.u.widget.val);
			}

			//uint64_t eval_data = eval_expression(pos->VarDict.val.u.regx.data_expr).u.long_val;



			//set_register(pos->VarDict.val.u.regx.addr, eval_data, pos->VarDict.val.u.regx.misk, pos->VarDict.val.u.regx.reg_type);

		}
	}
}

void set_default_page(int page_num) {

	VarDictList* pos;

	int page = -1;
	for (pos = g_VarDictList; pos != NULL; pos = pos->next) {

		if (pos->VarDict.val.type == PAGE) {
			page++;
		}


		if (page == page_num) {

			if (pos->VarDict.val.type == VARIABLE_REGX) {

				uint64_t eval_data = eval_expression(pos->VarDict.val.u.regx.data_expr).u.long_val;

				set_register(pos->VarDict.val.u.regx.addr, eval_data, pos->VarDict.val.u.regx.misk, pos->VarDict.val.u.regx.reg_type);
			}
		}
	}

}
