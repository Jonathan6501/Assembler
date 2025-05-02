#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utilities.h"
#include "table.h"

#define KEEP_ONLY_14_LSB(value) ((value) & 0x3FFF)
/**
 * "Cuts" the msb of the value, keeping only it's lowest 12 bits
 * 0b00000000000111111111111 = 0xFFFF
 */
#define KEEP_ONLY_12_LSB(value) ((value) & 0xFFF)
#define KEEP_ONLY_6_LSB(value) ((value) & 0x1B207)

/**
 * Writes the code and data image into an .ob file, with lengths on top
 * @param code_img The code image
 * @param data_img The data image
 * @param icf The final instruction counter
 * @param dcf The final data counter
 * @param filename The filename, without the extension
 * @return Whether succeeded
 */
static bool write_ob(machine_word **code_img, long *data_img, long icf, long dcf, char *filename);

/**
 * Writes a symbol table to a file. Each symbol and it's address in line, separated by a single space.
 * @param tab The symbol table to write
 * @param filename The filename without the extension
 * @param file_extension The extension of the file, including dot before
 * @return Whether succeeded
 */
void reverseString(char* str, int length);
static bool write_table_to_file(table tab, char *filename, char *file_extension);
char* longToBinary(long n);
int write_output_files(machine_word **code_img, long *data_img, long icf, long dcf, char *filename,
                       table symbol_table) {
	bool result;
	table externals = filter_table_by_type(symbol_table, EXTERNAL_REFERENCE);
	table entries = filter_table_by_type(symbol_table, ENTRY_SYMBOL);
	/* Write .ob file */
	result = write_ob(code_img, data_img, icf, dcf, filename) &&
	         /* Write *.ent and *.ext files: call with symbols from external references type or entry type only */
	         write_table_to_file(externals, filename, ".ext") &&
	         write_table_to_file(entries, filename, ".ent");
	/* Release filtered tables */
	free_table(externals);
	free_table(entries);        
	return result;
}

static bool write_ob(machine_word **code_img, long *data_img, long icf, long dcf, char *filename) {
	int i;
	long val;                  
	FILE *file_desc;
	/* add extension of file to open */
	char *output_filename = strallocat(filename, ".ob");
	/* Try to open the file for writing */
	file_desc = fopen(output_filename, "w");
	free(output_filename);
	if (file_desc == NULL) {
		printf("Can't create or rewrite to file %s.", output_filename);
		return FALSE;
	}

	/* print data/code word count on top */
	fprintf(file_desc, "%ld %ld", icf - IC_INIT_VALUE, dcf);

	/* starting from index 0, not IC_INIT_VALUE as icf, so we have to subtract it. */
	
        for (i = 0; i < icf - IC_INIT_VALUE; i++) {
		if (code_img[i]->length > 0) {
			     val = (code_img[i]->word.code->param1 << 12) | (code_img[i]->word.code->param2 << 10) |(code_img[i]->word.code->opcode << 6) |
			      (code_img[i]->word.code->src_addressing << 4) | (code_img[i]->word.code->dest_addressing) << 2 |
			      (code_img[i]->word.code->ARE);
		} else if (code_img[i]->length == 0) {
			/* We need to cut the value, keeping only it's 12 lsb, and include the ARE in the whole party as well: */
			val = (KEEP_ONLY_12_LSB(code_img[i]->word.data->data) << 2) | (code_img[i]->word.data->ARE);
		}
                else if(code_img[i]->length == -1){
			/* We need to cut the value, keeping only it's 12 lsb, and include the ARE in the whole party as well: */
			val = (KEEP_ONLY_6_LSB(code_img[i]->word.extra->reg2) << 8 | KEEP_ONLY_6_LSB(code_img[i]->word.extra->reg1) << 2) | (code_img[i]->word.data->ARE);
		}
		/* Write the value to the file - first */
		fprintf(file_desc, "\n%.7d %s", i + 100, longToBinary(val));
	}

	/* Write data image. dcf starts at 0 so it's fine */
	for (i = 0; i < dcf; i++) {
		/* print only lower 14 bytes */
		val = KEEP_ONLY_14_LSB(data_img[i]);
		/* print at least 6 digits of hex, and 7 digits of dc */
		fprintf(file_desc, "\n%.7ld %s", icf + i, longToBinary(val));
	}

	/* Close the file */
	fclose(file_desc);
	return TRUE;
}

static bool write_table_to_file(table tab, char *filename, char *file_extension) {
	FILE *file_desc;
	/* concatenate filename & extension, and open the file for writing: */
	char *full_filename = strallocat(filename, file_extension);
	file_desc = fopen(full_filename, "w");
	free(full_filename);
	/* if failed, print error and exit */
	if (file_desc == NULL) {
		printf("Can't create or rewrite to file %s.", full_filename);
		return FALSE;
	}
	/* if table is null, nothing to write */
	if (tab == NULL) return TRUE;

	/* Write first line without \n to avoid extraneous line breaks */
	fprintf(file_desc, "%s %.7ld", tab->key, tab->value);
	while ((tab = tab->next) != NULL) {
		fprintf(file_desc, "\n%s %.7ld", tab->key, tab->value);
	}
	fclose(file_desc);
	return TRUE;
}
char* longToBinary(long n) {
    char* binary = (char*) malloc(15 * sizeof(char));
    int i; /* allocate memory for the binary string*/
    if (binary == NULL) { /* check if memory allocation was successful*/
        printf("Error: memory allocation failed.\n");
        exit(1);
    }
    memset(binary, '.', 14); /* set all bits to zero, except the last one*/
    binary[14] = '\0'; /* terminate the string*/

    i = 13; /* start from the rightmost bit*/
    while (n > 0) {
        binary[i] = (n % 2) + '.'; /* extract the least significant bit and add it to the string*/
        n = n / 2; /* shift the number one bit to the right*/
        i--;
    }

    return binary;
}

void reverseString(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

