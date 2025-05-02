#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "utilities.h"
#include "writefiles.h"
#include "second_pass.h"
#include "first_pass.h"
#define MAX_LENGTH 1000
#define MAX_STRINGS 100
/**
 * Processes a single assembly source file, and returns the result status.
 * @param filename The filename, without it's extension
 * @return Whether succeeded
 */
char *truncString(char *str, int pos);
static bool process_file(char *preAssembler,char *filename);
char* move_to_not_white(char* str);

/**
 * 14bit assembler.  proccess Macros by using a temporary middle file and procceeds to proccess assembly instruction to special binary code
 */
int main(int argc, char *argv[]) {
    char* start_word = " mcr ";
    char* end_word = " endmcr";
        FILE* new;
        FILE* final;
        FILE* file;
        int i;
        int MacrosIndex;
        char Macros[100][1000];
        char *preinput_filename;
        char Macro[MAX_LENGTH];
        char line[MAX_LENGTH];
        char str[MAX_LENGTH];
        bool in_block;
        char  *ptr;
        int Search;
        int found;
        char temp[MAX_LENGTH];
	/* To break line if needed */
	bool succeeded = TRUE;
	/* Process each file by arguments */
	for (i = 1; i < argc; ++i) {
		/* if last process failed and there's another file, break line: */
                if (argc == 1) {
        printf("Usage: %s <file_name>\n", argv[0]);
           return EXIT_FAILURE;
       }
       new = fopen("PreAssemblerTemp.txt", "w+");
       final = fopen("PreAssembler.am", "w+");
       if (!new) {
        perror("Error opening file 2");
        exit(EXIT_FAILURE);
       }
       if (!final) {
        perror("Error opening file 3");
        exit(EXIT_FAILURE);
       }
                
                preinput_filename = strallocat(argv[i], ".as");
                file = fopen(preinput_filename, "r");
                if (!file) {
                printf("Error opening file 1%s",preinput_filename);
                 exit(EXIT_FAILURE);
        }
    in_block = FALSE;
    MacrosIndex = 0;
    while (fgets(line, MAX_LENGTH, file)) { /* runs on the source file and extract the whole macro declaration to an array. and copy the other lines to middle temp*/
        if (strstr(line, start_word) != NULL) {
           strncpy(str, line,MAX_LENGTH);
           ptr = move_to_not_white(str); 
           Macro[0] = ptr[4];  Macro[1] = ptr[5];
           strcpy(Macros[MacrosIndex] , Macro);
           MacrosIndex++;
           in_block = TRUE;
        } 
         else if (strstr(line, end_word) != NULL) {
            in_block = FALSE;
            MacrosIndex++;
        } else if (in_block == TRUE) {
           strcat(Macros[MacrosIndex], truncString(line, 5));
        }
        else{
             fprintf(new,"%s", line);}
    }
    rewind(new);
    while (fgets(line, MAX_LENGTH, new)) /* Runs on the middle file and copy it to the final file but replaces the macros with the macros that were configiured before*/
    {  
       found = 0;
       Search = 0;
		while (Search < 50 && Macros[Search][0] != '\0')
       {
           strncpy(temp, line,MAX_LENGTH);
           ptr = move_to_not_white(temp); 
           
           if(ptr[0] == Macros[Search][0] && ptr[1] == Macros[Search][1]) 
           { 
             fprintf(final,"%s", Macros[Search+1]); 
             found = 1;
           } 
         Search += 2; 
          } 
         if(found == 0)
        {
          fprintf(final,"%s", line);
         }
    }


    fclose(file);
    fclose(new);
    fclose(final);
                 /* if last process failed and there's another file, break line: */
		if (!succeeded) puts("");
		/* foreach argument (file name), send it for full processing. */
		/* Line break if failed */
                /*sends the proccessed macros file to assembly proccedure */
		succeeded = process_file("PreAssembler" , argv[i]);

     }
   return 0;

}
char* move_to_not_white(char* str) { /* skips to the first non white char of the string */
    int len = strlen(str);
    int i = 0;
    while (i < len && isspace(str[i])) {
        i++;
    }
    return str + i;
}
char *truncString(char *str, int pos) /* skips to the X char of the string */
{
    size_t len = strlen(str);
    if (len > abs(pos)) {
        if (pos > 0)
            str = str + pos;
        else
            str[len + pos] = 0;
    }
    return str;
}
static bool process_file(char *preAssembler,char *filename) {
	int temp_c;
	long ic = IC_INIT_VALUE, dc = 0, icf, dcf;
	bool is_success = TRUE; /* is succeeded so far */
	char *input_filename;
	char temp_line[MAX_LINE_LENGTH + 2]; /* temporary string to store line that was read from file */
	FILE *file_des; /* The current file to proccess */
	long data_img[CODE_ARR_IMG_LENGTH]; /* Contains an image of the machine code */
	machine_word *code_img[CODE_ARR_IMG_LENGTH];
	/* Our symbol table */
	table symbol_table = NULL;
	line_info curr_line_info;
	/* Concat extensionless filename with .as extension */
	input_filename = strallocat(preAssembler, ".am");
	/* Open file, skip on failure */
	file_des = fopen(input_filename, "r");
	if (file_des == NULL) {
		/* if file couldn't be opened, write to stderr. */
		printf("Error: file \"%s.as\" is inaccessible for reading. skipping it.\n", filename);
		free(input_filename); /* The only allocated space is for the full file name */
		return FALSE;
	}
	/* start first pass: */
	curr_line_info.file_name = input_filename;
	curr_line_info.content = temp_line; /* We use temp_line to read from the file, but it stays at same location. */
	/* Read line - stop if read failed (when NULL returned) - usually when EOF. increase line counter for error printing. */
	for (curr_line_info.line_number = 1;
	     fgets(temp_line, MAX_LINE_LENGTH + 2, file_des) != NULL; curr_line_info.line_number++) {
		/* if line too long, the buffer doesn't include the '\n' char OR the file isn't on end. */
		if (strchr(temp_line, '\n') == NULL && !feof(file_des)) {        
			/* Print message and prevent further line processing, as well as second pass.  */
			printf_line_error(curr_line_info, "Line too long to process. Maximum line length should be %d.",
			                  MAX_LINE_LENGTH);
			is_success = FALSE;
			/* skip leftovers */
			do {
				temp_c = fgetc(file_des);
			} while (temp_c != '\n' && temp_c != EOF);
		} else {
			if (!process_line_fpass(curr_line_info, &ic, &dc, code_img, data_img, &symbol_table)) {
				if (is_success) {
					icf = -1;
					is_success = FALSE;  
				}
			}
		}
	}
	/* Saves DCF & ICF */
	icf = ic;
	dcf = dc;
	/* if first pass didn't fail, start the second pass */
	if (is_success) {
	ic = IC_INIT_VALUE;
	/* Now let's add IC to each DC for each of the data symbols in table (step 1.19) */
	add_value_to_type(symbol_table, icf, DATA_SYMBOL);
	/* First pass done right. start second pass: */
	rewind(file_des); /* Start from beginning of file again */
	for (curr_line_info.line_number = 1; !feof(file_des); curr_line_info.line_number++) {
		int i = 0;
		fgets(temp_line, MAX_LINE_LENGTH, file_des); /* Get line */
		MOVE_TO_NOT_WHITE(temp_line, i)
		if (code_img[ic - IC_INIT_VALUE] != NULL || temp_line[i] == '.')
			is_success &= process_line_spass(curr_line_info, &ic, code_img, &symbol_table);
	}
		/* Write files if second pass succeeded */
		if (is_success) {                  
			/* Everything was done. Write to *filename.ob/.ext/.ent */
			is_success = write_output_files(code_img, data_img, icf, dcf, filename, symbol_table);
		}
	}

	/* Now we free the pointer: */
	/* current file name */
	free(input_filename);
	/* Free symbol table */
	free_table(symbol_table);
	/* Free code & data & extra buffer contents */
	free_code_image(code_img, icf);
	/* return whether every assembling succeeded */
	return is_success;
}
