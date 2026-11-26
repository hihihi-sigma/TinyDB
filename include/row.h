#ifndef TINYDB_ROW_H
#define TINYDB_ROW_H
#include<stdint.h>
#include<stddef.h>
#define COLUMN_USERNAME_SIZE 63
#define COLUMN_EMAIL_SIZE 127
#define TABLE_MAX_ROWS 1024

typedef enum{
    MetaCommandResult_SUCCESS,
    MetaCommandResult_UNRECOGNIZED_COMMAND
}MetaCommandResult; // work with some statement start with . (Ex: .exit)

typedef enum{
    PrepareResult_SUCCESS,
    PrepareResult_SYNTAX_ERROR,
    PrepareResult_UNRECOGNIZED_STATEMENT
}PrepareResult; // check that if the statement is correct or not 

typedef enum{
    STATEMENT_INSERT,
    STATEMENT_SELECT,
    STATEMENT_DELETE
}StatementType; // check which type of statement is

typedef enum{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL,
    EXECUTE_NOT_FOUND // if cannot find the index to delete
}ExecuteResult; // check that if the statement is executed success or not

typedef struct{
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
}Row; // 

typedef struct{
    StatementType type;
    Row row_to_insert; // if type = DEL => id = delete this id
}Statement; // which statemnt and insert

typedef struct{
    size_t num_rows;
    Row rows[TABLE_MAX_ROWS];
}Table; // where to store the data of each row


#endif