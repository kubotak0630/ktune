/*
 * gtk_create.h
 *
 *  Created on: Jun 11, 2017
 *      Author: kubotak
 */

#ifndef GTK_CREATE_H_
#define GTK_CREATE_H_

struct Varialbe_Dict;  //前方参照

void window_init(int argc, char** argv, int size_x, int size_y);
void widget_show_all();
void set_default_all();

void start_create_gtk_widget();


#endif /* GTK_CREATE_H_ */
