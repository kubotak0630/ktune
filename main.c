#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ktu_create.h"
#include "gtk_create.h"

#include <gtk/gtk.h>

StatementList* g_st_list = NULL;


int g_debug = 0;

int main(int argc, char** argv) {


	printf("argc = %d\n", argc);

	int i;
	for (i = 0; i < argc; i++) {
		printf("%s\n", argv[i]);
	}

	if (argc == 3) {
		if (strcmp(argv[2], "-debug") == 0) {
			g_debug = 1;
		}
		else {
			fprintf(stderr, "arg error\n");
			fprintf(stderr, "support \"-debug\" only\n");
			exit(1);
		}
	}


	extern int yyparse(void);
	extern FILE *yyin;


	yyin = fopen(argv[1], "r");

	if (yyin == NULL) {
		fprintf(stderr, "file open error, %s not exist\n", argv[1]);
		exit(1);

	}


/*
	if ( (yyin = fopen(argv[1], "r")) == NULL )
		fprintf(stderr, "file open error, %s not exist\n", argv[1]);
		exit(1);

	}
*/
	//yyin = stdin;
	if (yyparse()) {
		//fprintf(stderr, "Error ! Error ! Error !\n");
		exit(1);
	}



	printf("\n**** start execute_statement_list ********\n\n");
	execute_statement_list(g_st_list);




	printf("\n*** show gtk val ********\n");

	window_init(argc, argv, 200, 400);

	start_create_gtk_widget();

	set_default_all();

	widget_show_all();

	gtk_main();


	return 0;

}
