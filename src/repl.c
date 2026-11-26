#define _POSIX_C_SOURCE 200809L
#include<stdio.h>
#include "row.h"
#include<ctype.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>

typedef struct{
    char *Buffer; // tro toi vung nho chuoi tren heap
    size_t Buffer_length; // maximum length
    ssize_t input_length; // real input_length
}InputBuffer;

static InputBuffer *new_input_buffer(void){
    InputBuffer *ptr = malloc(sizeof(InputBuffer));
    if(ptr == NULL) return NULL;
    ptr->Buffer = NULL;
    ptr->Buffer_length = 0;
    ptr->input_length = 0;
    return ptr;
}

static void close_input_buffer(InputBuffer *input){
    if(input == NULL) return;
    free(input->Buffer);
    free(input);
}

static void print(void){
    printf("Tinydb> ");
    fflush(stdout);
}

static void trim_trailing_newline(char *str, ssize_t *len){
    if(str == NULL || len == NULL || (*len) <= 0) return;
    if(*len > 0 && str[*len - 1] == '\n'){
        str[*len - 1] = '\0';
        (*len)--;
    }
}

static int is_blank_or_spaces(const char *s){
    int idx = 0;
    while(s[idx] != '\0'){
        if(s[idx] != ' ' && s[idx] != '\t') return 0;
        idx++;
    }
    return 1;
}

ssize_t my_getline(char **str, size_t *len, FILE *stream){
    if(str == NULL || len == NULL || stream == NULL) return -1;
    if(*str == NULL || *len == 0){
        size_t init_cap = 128;
        char *buf = malloc(init_cap);
        if(buf == NULL) return -1;
        *str = buf;
        *len = init_cap;
    }
    size_t pos = 0; // dung luong da doc duoc
    while(1){
        if(pos + 1 >= *len){
            size_t new_cap = (*len) * 2;
            char *new_buf = realloc(*str, new_cap);
            if(new_buf == NULL) return -1;
            *str = new_buf;
            *len = new_cap;
        }
        if(fgets(*str + pos, (int)(*len - pos), stream) == NULL){
            if(pos == 0) return -1;
            break;
        }
        pos += strlen(*str + pos);
        if(pos > 0 && (*str)[pos - 1] == '\n'){
            break;
        }
    }
    return (ssize_t)pos;
}

static MetaCommandResult do_meta_command(InputBuffer *input){
    if(input == NULL || input->Buffer == NULL) return MetaCommandResult_UNRECOGNIZED_COMMAND;
    if(strcmp(input->Buffer, ".exit") == 0){
        close_input_buffer(input);
        exit(EXIT_SUCCESS);
    }
    return MetaCommandResult_UNRECOGNIZED_COMMAND;
}

static PrepareResult prepare_statement(InputBuffer *input, Statement *statement){
    if(input == NULL || input->Buffer == NULL) return PrepareResult_UNRECOGNIZED_STATEMENT;
    if(strcmp(input->Buffer, "SELECT") == 0){
        statement->type = STATEMENT_SELECT;
        return PrepareResult_SUCCESS;
    }
    else if(strncmp(input->Buffer, "INSERT ", 7) == 0){
        statement->type = STATEMENT_INSERT;
        int n = sscanf(input->Buffer + 7, "%u %63s %127s", &statement->row_to_insert.id,
                                                     statement->row_to_insert.username,
                                                     statement->row_to_insert.email);
        if(n != 3) return PrepareResult_SYNTAX_ERROR;   
        return PrepareResult_SUCCESS;
    }
    return PrepareResult_UNRECOGNIZED_STATEMENT;
}

static ExecuteResult execute_statement(Statement input, Table *tiny_db){
    if(tiny_db == NULL) return EXECUTE_TABLE_FULL;
    if(input.type == STATEMENT_INSERT){
        if(tiny_db->num_rows >= TABLE_MAX_ROWS){
            return EXECUTE_TABLE_FULL;
        }
        tiny_db->rows[tiny_db->num_rows].id = input.row_to_insert.id;
        strcpy(tiny_db->rows[tiny_db->num_rows].username, input.row_to_insert.username);
        strcpy(tiny_db->rows[tiny_db->num_rows].email, input.row_to_insert.email);
        (tiny_db->num_rows)++;
    }
    else if(input.type == STATEMENT_SELECT){
        for(size_t i = 0;i<tiny_db->num_rows;i++){
            printf("%u %s %s\n", tiny_db->rows[i].id, tiny_db->rows[i].username, tiny_db->rows[i].email);
        }
    }
    return EXECUTE_SUCCESS;
}

int main(void){
    InputBuffer *input = new_input_buffer();
    if(input == NULL) return 1;
    Table tiny_db;
    while(1){
        Statement now;
        print();
        ssize_t len = my_getline(&input->Buffer, &input->Buffer_length, stdin);
        if(len < 0) break;
        input->input_length = len;
        trim_trailing_newline(input->Buffer, &input->input_length);
        if(is_blank_or_spaces(input->Buffer)) continue;
    }
    close_input_buffer(input);
    return 0;
}