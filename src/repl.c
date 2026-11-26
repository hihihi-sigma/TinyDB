#define _POSIX_C_SOURCE 200809L
#include<stdio.h>
#include "row.h"
#include<ctype.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>

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

bool str_ieq(const char *a, const char *b, size_t len, bool exact_match){
    if(a == NULL || b == NULL) return false;
    for(size_t i = 0;i<len;i++){
        if(a[i] == '\0' || b[i] == '\0') return false;
        if(tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
    }
    if(exact_match) return a[len] == '\0' && b[len] == '\0';
    return true;
}

static MetaCommandResult do_meta_command(InputBuffer *input){
    if(input == NULL || input->Buffer == NULL) return MetaCommandResult_UNRECOGNIZED_COMMAND;
    if(str_ieq(input->Buffer, ".exit", 5, 1)){
        close_input_buffer(input);
        exit(EXIT_SUCCESS);
    }
    return MetaCommandResult_UNRECOGNIZED_COMMAND;
}

static PrepareResult prepare_statement(InputBuffer *input, Statement *statement){
    if(input == NULL || input->Buffer == NULL) return PrepareResult_UNRECOGNIZED_STATEMENT;
    if(str_ieq(input->Buffer, "SELECT", 6, 1)){
        statement->type = STATEMENT_SELECT;
        return PrepareResult_SUCCESS;
    }
    else if(str_ieq(input->Buffer, "INSERT ", 7, 0)){
        statement->type = STATEMENT_INSERT;
        int n = sscanf(input->Buffer + 7, "%u %63s %127s", &statement->row_to_insert.id,
                                                     statement->row_to_insert.username,
                                                     statement->row_to_insert.email);
        if(n != 3) return PrepareResult_SYNTAX_ERROR;   
        return PrepareResult_SUCCESS;
    }
    else if(str_ieq(input->Buffer, "DEL ", 4, 0)){
        statement->type = STATEMENT_DELETE;
        if(input->Buffer[4] == '-') return PrepareResult_SYNTAX_ERROR;
        int n = sscanf(input->Buffer + 4, "%u", &statement->row_to_insert.id);
        if(n != 1) return PrepareResult_SYNTAX_ERROR;
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
    else if(input.type == STATEMENT_DELETE){
        bool found = false;
        for(size_t i = 0;i<tiny_db->num_rows;i++){
            if(tiny_db->rows[i].id == input.row_to_insert.id) found = true;
            if(found && i != tiny_db->num_rows - 1) tiny_db->rows[i] = tiny_db->rows[i+1];
        }
        if(found == false) return EXECUTE_NOT_FOUND;
        (tiny_db->num_rows)--;
        memset(&tiny_db->rows[tiny_db->num_rows], 0, sizeof(Row));
    }
    return EXECUTE_SUCCESS;
}

int main(void){
    InputBuffer *input = new_input_buffer();
    if(input == NULL) return 1;
    Table tiny_db;
    memset(&tiny_db, 0, sizeof(tiny_db));
    // how it works: getline() -> trim_trailing_newline -> check if blank or spaces
    // -> check do_meta_command
    // -> analysis the statement -> Execute
    while(1){
        print();
        ssize_t len = my_getline(&input->Buffer, &input->Buffer_length, stdin);
        if(len < 0) break;
        input->input_length = len;
        trim_trailing_newline(input->Buffer, &input->input_length);
        if(is_blank_or_spaces(input->Buffer)) continue;
        if(str_ieq(input->Buffer, ".", 1, 0)){
            if(do_meta_command(input) == MetaCommandResult_SUCCESS) break;
            else{
                printf("Unrecognized command %s \n", input->Buffer);
                continue;
            }
        }
        Statement now = {0};
        PrepareResult prep = prepare_statement(input, &now);
        if(prep != PrepareResult_SUCCESS){
            if(prep == PrepareResult_SYNTAX_ERROR) printf("Syntax Error\n");
            else if(prep == PrepareResult_UNRECOGNIZED_STATEMENT) printf("Unrecognized Statement\n");
            continue;
        }
        ExecuteResult exe = execute_statement(now, &tiny_db);
        if(exe == EXECUTE_TABLE_FULL){
            printf("Table Full\n");
            continue;
        }
        else if(exe == EXECUTE_NOT_FOUND){
            printf("Index Not Found\n");
            continue;
        }
        printf("Executed Successfully\n");
    }
    close_input_buffer(input);
    return 0;
}