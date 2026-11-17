#ifndef WORD_H
#define WORD_H

#include <stdio.h>

enum character_type_e
{
    CHAR_NULL = 0,
    CHAR_NORMAL = 1,
    CHAR_NUMBER = 2,
    CHAR_DQ = 3,
    CHAR_SQ = 4,
    CHAR_CONNECTABLE = 9,
    CHAR_UNCONNECTABLE = 10,
    CHAR_SPACE = 11,
    CHAR_BS = 12,
    CHAR_NEWLINE = 13,
    CHAR_SEMICOLON = 14
};

enum word_type_e
{
    WORD_NORMAL,
    WORD_STRING,
    WORD_NUMBER,
    WORD_SYMBOL,
    WORD_DIMENSION,
    WORD_BREAKER,
};

typedef struct word_s
{
    union
    {
        char * string;
        double number;
        struct {
            double value;
            char *unit;
        } dimension;
    } data;
    enum word_type_e word_type;
    char *name;
} word_t;

typedef struct word_carrier_s
{
    word_t * words;
    unsigned words_length;
} word_carrier_t;

typedef struct keyword_s
{
    char * name;
    int token;
    void * instance; //ptr of head_t, prompt_t, symbol_t or weld_t
} keyword_t;

typedef struct word_hash_item_s
{
    word_t word;

    unsigned long hash;
} word_hash_item_t;

typedef struct word_hash_table_s
{
    word_hash_item_t * items;
    unsigned items_length; //拜託用2倍數
    unsigned items_used;
} word_hash_table_t;

typedef struct hash_carrier_s
{
    unsigned long * hashes;
    unsigned hashes_length;
} hash_carrier_t;

extern unsigned long hash(char const * string);

word_hash_table_t set_word_hash_table(
    word_hash_table_t const original_table,
    unsigned const new_table_len
    );
int put_into_word_hash_table(
    word_hash_table_t * const table_ptr,
    word_hash_item_t const item,
    char const rehash
    );
word_hash_item_t get_word_hash_item(
    word_hash_table_t const hash_table,
    unsigned long const hash
    );


typedef hash_carrier_t word_hash_carrier_t;


extern word_hash_carrier_t stream_to_word(FILE * F,char END_FLAG,word_hash_table_t * word_hash_table_ptr);


#endif
