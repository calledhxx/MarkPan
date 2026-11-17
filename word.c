#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "word.h"

unsigned long hash(char const * string)
{
    unsigned long hash = 5381;

    while (*string)
    {
        hash = hash * 33 + (char)*string;
        string++;
    }


    return hash;
}



word_hash_table_t set_word_hash_table(
    word_hash_table_t const original_table,
    unsigned const new_table_len
    )
{
    word_hash_table_t result = {0};
    result.items_length = new_table_len;
    result.items = (word_hash_item_t*)realloc(
        result.items,
        sizeof(word_hash_item_t) * result.items_length
        );
    result.items_used = original_table.items_used;

    //rehash
    for (int i = 0; i < original_table.items_length; i++)
    {
        word_hash_item_t const item = original_table.items[i];

        unsigned new_index = item.hash % result.items_length;

        for (int si = 1; si < result.items_length; si++)
            if (result.items[new_index].hash)
            {
                // if (result.items[new_index].hash == item.hash) 不可能發生
                new_index = (item.hash + si*si) % result.items_length;
            }
            else goto found;

        return result; //no space

        found:

        result.items[new_index] = item;
    }

    free(original_table.items);

    return result;
}

int put_into_word_hash_table(
    word_hash_table_t * const table_ptr,
    word_hash_item_t const item,
    char const rehash
    )
{
    unsigned new_index = item.hash % table_ptr->items_length;

    for (int si = 1; si < table_ptr->items_length; si++)
        if (table_ptr->items[new_index].hash)
        {
            if (table_ptr->items[new_index].hash == item.hash)
                return -2; //already exist

            new_index = (item.hash + si*si) % table_ptr->items_length;
        }
        else goto found;

    if (rehash)
    {
        word_hash_table_t const new = set_word_hash_table(*table_ptr,table_ptr->items_length*2);
        table_ptr->items = new.items;
        table_ptr->items_length = new.items_length;
        table_ptr->items_used = new.items_used;

        return put_into_word_hash_table(
        table_ptr,
        item,
        1
            );
    }

    return -1; //no space

    found:

    table_ptr->items[new_index] = item;
    ++table_ptr->items_used;

    return (int)new_index;
}

word_hash_item_t get_word_hash_item(
    word_hash_table_t const hash_table,
    unsigned long const hash
    )
{
    word_hash_item_t word_hash_item = {0};

    unsigned index = hash % hash_table.items_length;

    for (int si = 1; si < hash_table.items_length; si++)
        if (hash_table.items[index].hash)
        {
            if (hash_table.items[index].hash == hash)
            {
                word_hash_item = hash_table.items[index];
                goto found;
            }

            index = (hash + si*si) % hash_table.items_length;
        }

    return (word_hash_item_t){0};

    found:

    return word_hash_item;
}

static const char Connectable[] = {
    '*',
    '/',
    '+',
    '=',
    '^',
    '<',
    '>',
    '.'
};

static const char Unconnectable[] = {
    '(',
    ')',
    '[',
    ']',
    '{',
    '}',
    '-',
    ',',
    ':',
    '@',
};

enum character_type_e CheckCharType(const char Char)
{
    if (Char == EOF || Char == 0) return CHAR_NULL;

    if ((Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z')) return CHAR_NORMAL;

    if (Char >= '0' && Char <= '9') return CHAR_NUMBER;

    if (Char == '"') return CHAR_DQ;

    if (Char == '\'') return CHAR_SQ;

    for (int i = 0; i < sizeof(Connectable)/sizeof(Connectable[0]); i++)
        if (Connectable[i] == Char)  return CHAR_CONNECTABLE;

    for (int i = 0; i < sizeof(Unconnectable)/sizeof(Unconnectable[0]); i++)
        if (Unconnectable[i] == Char) return CHAR_UNCONNECTABLE;

    if (Char == ' ' || Char == '\t') return CHAR_SPACE;

    if (Char == '\\') return CHAR_BS;

    if (Char == '\n') return CHAR_NEWLINE;

    if (Char == ';') return CHAR_SEMICOLON;

    return 1;
}

word_hash_carrier_t stream_to_word(FILE * F,const char END_FLAG,word_hash_table_t * word_hash_table_ptr)
{
    // word_carrier_t CaseCarriers = {0};

    word_hash_carrier_t word_hash_carrier = {0};

    char * CaseName = NULL;
    unsigned int CaseNameLen = 0;

    unsigned int CharIndex = 0; //字元的偏移位

    char ThisChar = 0; //目前字元
    char LastChar = 0; //上個字元

    enum character_type_e ThisCharType = 0; //目前字元類型 包括被覆蓋的類型
    enum character_type_e LastCharType = 0; //上個字元類型 包括被覆蓋的類型

    char HandleType = 0; //處理類型

    char StringHandleChar = 0; //字串類型 `'` 與 `"` 的編碼

    unsigned int ProcessingLine = 1;
    unsigned int ProcessingColumn = 1;

    char * SuperCharParentName = NULL;
    unsigned int SuperCharParentNameLen = 0;

    char SuperCharParentType = 0; //Super Character Parent 的類型

    char * SuperCharChildName = NULL;
    unsigned int SuperCharChildNameLen = 0;

    char SuperCharCollect = 0; //Super Character 的收集狀態。1 : 已收集完Parent ; 2 : 已收集完Child

    char * toCMPChar = 0; //Super Character Parent名稱判斷
    // !

    // Number 特殊處理
    char RecordingNumber = 0; //正在紀錄數字 在有表達數字時 0表一般數字; 1表二進制; 2表十六進制;
    char RecordingUnitLen = 0; //正在紀錄數字後方的單位的長度
    char DotExist = 0; //紀錄小數點是否存在
    // !

    // 註解
    char CommentChar = 0; //註解類型，以註解開頭的第二符號為主要記錄對象。
    // !

    do
    {
        ThisChar = (char)fgetc(F);
        ThisCharType = CheckCharType(ThisChar);


        switch (HandleType) //串一定要有休止符 才能使用Handle處裡 如：`"Hello World"`的尾端 `"`
        {
        case 0: //一般
            {
                //自元分節
                if (CharIndex)
                {
                    if (!(
                        (LastCharType == CHAR_CONNECTABLE && LastChar == '.' && CaseNameLen == 1 && ThisCharType == CHAR_NUMBER && !DotExist)
                        ||
                        (LastCharType == CHAR_NUMBER && ThisChar == '.' && !DotExist)
                        ||
                        (LastCharType == CHAR_NORMAL && ThisCharType == CHAR_NUMBER)
                        ||
                        (LastCharType == CHAR_NUMBER && LastChar == '0' && CaseNameLen == 1 && ThisCharType == CHAR_NORMAL && (ThisChar == 'x' || ThisChar == 'b'))
                        ||
                        (RecordingNumber == 2 && ThisCharType == CHAR_NORMAL)
                        ||
                        (LastCharType == CHAR_NUMBER && ThisCharType == CHAR_NORMAL)
                    ))
                        if (CaseNameLen && LastCharType != ThisCharType){
                            word_t word = {
                                .word_type = WORD_NORMAL,
                            };

                            switch (LastCharType)
                            {
                            case CHAR_NUMBER:
                                {
                                    if (RecordingNumber == 0)
                                    {
                                        word.data.number = strtol(CaseName,0,10);
                                        word.word_type = WORD_NUMBER;
                                    }
                                    else if (RecordingNumber == 1)
                                    {
                                        word.data.number = strtol(CaseName,0,2);
                                        word.word_type = WORD_NUMBER;
                                    }
                                    else if (RecordingNumber == 2)
                                    {
                                        word.data.number = strtol(CaseName,0,16);
                                        word.word_type = WORD_NUMBER;
                                    }

                                    break;
                                }
                            case CHAR_CONNECTABLE:
                                {
                                    word.word_type = WORD_SYMBOL;

                                    break;
                                }
                            case CHAR_SEMICOLON:
                                {
                                    word.word_type = WORD_BREAKER;

                                    break;
                                }

                                default:break;
                            }

                            CaseNameLen++;
                            CaseName = realloc(
                                CaseName,
                                CaseNameLen*sizeof(char)
                                );
                            CaseName[CaseNameLen-1] = 0;

                            if (RecordingUnitLen)
                            {
                                char buf[128] = {0};
                                memcpy(buf, CaseName, CaseNameLen - RecordingUnitLen);
                                buf[CaseNameLen - RecordingUnitLen - 1] = 0;

                                word.word_type = WORD_DIMENSION;
                                word.data.dimension.unit = CaseName + RecordingUnitLen;
                                word.data.dimension.value = strtol(buf,0,10);
                            }

                            word.name = CaseName;

                            unsigned long const word_hash = hash(CaseName);

                            put_into_word_hash_table(
                                word_hash_table_ptr,
                                (word_hash_item_t){
                                    .hash = word_hash,
                                    .word = word,
                                },
                                1
                                );

                            word_hash_carrier.hashes_length++;
                            word_hash_carrier.hashes = realloc(
                                word_hash_carrier.hashes,
                                word_hash_carrier.hashes_length * sizeof(unsigned long)
                                );
                            word_hash_carrier.hashes[
                            word_hash_carrier.hashes_length - 1
                                ] = word_hash;


                            // CaseCarriers.words_length++;
                            // CaseCarriers.words = realloc(
                            //     CaseCarriers.words,
                            //     CaseCarriers.words_length*sizeof(word_t)
                            // );
                            // CaseCarriers.words[CaseCarriers.words_length-1] = (word_t){
                            //     .word = CaseName,
                            //     .word_type = CaseType,
                            // };


                            RecordingNumber = 0; //重置數字紀錄
                            RecordingUnitLen = 0;
                            DotExist = 0;

                            CaseNameLen = 0;
                            // free(CaseName);
                            CaseName = NULL;
                        }


                }



                //單字元處裡
                switch (ThisCharType)
                {
                case CHAR_NULL: break;
                case CHAR_NORMAL:
                    {
                        if (RecordingUnitLen)
                        {
                            RecordingUnitLen++;
                            CaseNameLen++;
                            CaseName = realloc(
                                CaseName,
                                CaseNameLen*sizeof(char)
                                );
                            CaseName[CaseNameLen-1] = ThisChar;

                            ThisCharType = CHAR_NUMBER;
                        }else if (
                            RecordingNumber == 0
                            &&
                            LastCharType == CHAR_NUMBER && LastChar == '0' && CaseNameLen == 1
                            &&
                            (ThisChar == 'x' || ThisChar == 'b')
                            )
                        {
                            RecordingNumber = ThisChar == 'x' ? 2 : 1;
                            ThisCharType = CHAR_NUMBER;

                        }else
                        {
                            if (RecordingNumber == 2)
                                if (
                                    (ThisChar >= 'A' && ThisChar <= 'F')
                                    ||
                                    (ThisChar >= 'a' && ThisChar <= 'f')
                                    )
                                    ThisCharType = CHAR_NUMBER;

                            if (
                                RecordingNumber == 0
                                &&
                                LastCharType == CHAR_NUMBER
                                )
                            {
                                RecordingUnitLen = 1;
                                ThisCharType = CHAR_NUMBER;
                            }

                            CaseNameLen++;
                            CaseName = realloc(
                                CaseName,
                                CaseNameLen*sizeof(char)
                                );
                            CaseName[CaseNameLen-1] = ThisChar;
                        }

                        break;
                    }
                case CHAR_NUMBER:
                    {
                        if (LastCharType == CHAR_CONNECTABLE && CaseNameLen == 1 && LastChar == '.' && !DotExist)
                            DotExist = 1;

                        if (LastCharType == CHAR_NORMAL)
                            ThisCharType = CHAR_NORMAL;


                        if (RecordingNumber == 1)
                            if (!(ThisChar == '0' || ThisChar == '1'))
                            {
                                CaseNameLen++;
                                CaseName = realloc(
                                    CaseName,
                                    CaseNameLen*sizeof(char)
                                    );
                                CaseName[CaseNameLen-1] = 0;

                                unsigned long const word_hash = hash(CaseName);

                                put_into_word_hash_table(
                                word_hash_table_ptr,
                                (word_hash_item_t){
                                  .hash = word_hash,
                                  .word = {
                                      .data.number = strtol(CaseName,0,10),
                                      .word_type = WORD_NUMBER,
                                      .name = CaseName
                                  }
                              },
                              1
                              );


                                word_hash_carrier.hashes_length++;
                                word_hash_carrier.hashes = realloc(
                                    word_hash_carrier.hashes,
                                    word_hash_carrier.hashes_length * sizeof(unsigned long)
                                    );
                                word_hash_carrier.hashes[
                                word_hash_carrier.hashes_length - 1
                                    ] = word_hash;

                                // CaseCarriers.words_length++;
                                // CaseCarriers.words = realloc(
                                //     CaseCarriers.words,
                                //     CaseCarriers.words_length*sizeof(word_t)
                                // );
                                //
                                // CaseCarriers.words[CaseCarriers.words_length-1] = (word_t){
                                //     .word = CaseName,
                                //     .word_type = WORD_DECNUMBER,
                                // };



                                RecordingNumber = 0; //重置數字紀錄

                                CaseNameLen = 0;
                                //free(CaseName);
                                CaseName = NULL;
                            }


                        CaseNameLen++;
                        CaseName = realloc(
                            CaseName,
                            CaseNameLen*sizeof(char)
                            );
                        CaseName[CaseNameLen-1] = ThisChar;

                        break;
                    }

                case CHAR_CONNECTABLE:
                    {
                        if (LastChar == '/')
                            if (ThisChar == '/' || ThisChar == '*')
                            {
                                HandleType = 3;
                                CommentChar = ThisChar;

                                CaseNameLen = 0;
                                //free(CaseName);
                                CaseName = NULL;

                                continue;
                            }

                        CaseNameLen++;
                        CaseName = realloc(
                            CaseName,
                            CaseNameLen*sizeof(char)
                            );
                        CaseName[CaseNameLen-1] = ThisChar;


                        if (ThisChar == '.' && !DotExist && LastCharType == CHAR_NUMBER && CaseNameLen != 1)
                        {
                            DotExist = 1;
                            ThisCharType = CHAR_NUMBER;
                        }


                        break;
                    }

                case CHAR_UNCONNECTABLE:
                    {

                            CaseNameLen+=2;
                            CaseName = realloc(
                                CaseName,
                                CaseNameLen*sizeof(char)
                                );

                            CaseName[0] = ThisChar;
                            CaseName[1] = 0;

                        unsigned long const word_hash = hash(CaseName);

                        put_into_word_hash_table(
                               word_hash_table_ptr,
                               (word_hash_item_t){
                                 .hash = word_hash,
                                 .word = {
                                     .word_type = WORD_SYMBOL,
                                     .name = CaseName
                                 }
                             },
                             1
                             );

                        word_hash_carrier.hashes_length++;
                        word_hash_carrier.hashes = realloc(
                            word_hash_carrier.hashes,
                            word_hash_carrier.hashes_length * sizeof(unsigned long)
                            );
                        word_hash_carrier.hashes[
                        word_hash_carrier.hashes_length - 1
                            ] = word_hash;

                            // CaseCarriers.words_length++;
                            // CaseCarriers.words = realloc(
                            //     CaseCarriers.words,
                            //     CaseCarriers.words_length*sizeof(word_t)
                            // );
                            //
                            // CaseCarriers.words[CaseCarriers.words_length-1] = (word_t){
                            //     .word = CaseName,
                            //     .word_type = WORD_UNCONNECTABLE,
                            // };


                            CaseNameLen = 0;
                            //free(CaseName);
                            CaseName = NULL;

                        break;
                    }

                case CHAR_SEMICOLON:
                    {
                        CaseNameLen++;
                        CaseName = realloc(
                            CaseName,
                            CaseNameLen*sizeof(char)
                            );
                        CaseName[CaseNameLen-1] = ThisChar;

                        break;
                    }


                //字串Handler
                case CHAR_DQ:
                case CHAR_SQ:
                    {
                        HandleType = 1; //開始處理字串
                        StringHandleChar = ThisChar;

                        break;
                    }
                default:break;
                }

                break;
            }
        case 1: //字串
            {
                //單字元處裡
                switch (ThisCharType)
                {
                case CHAR_DQ:
                case CHAR_SQ:
                    {

                        if (StringHandleChar == ThisChar)
                        {
                            HandleType = 0;

                            CaseNameLen++;
                            CaseName = realloc(
                                CaseName,
                                CaseNameLen*sizeof(char)
                                );
                            CaseName[CaseNameLen-1] = 0;

                            unsigned long const word_hash = hash(CaseName);

                            put_into_word_hash_table(
                               word_hash_table_ptr,
                               (word_hash_item_t){
                                 .hash = word_hash,
                                 .word = {
                                     .data.string = CaseName,
                                     .word_type = WORD_STRING,
                                     .name = CaseName
                                 }
                             },
                             1
                             );

                            word_hash_carrier.hashes_length++;
                            word_hash_carrier.hashes = realloc(
                                word_hash_carrier.hashes,
                                word_hash_carrier.hashes_length * sizeof(unsigned long)
                                );
                            word_hash_carrier.hashes[
                            word_hash_carrier.hashes_length - 1
                                ] = word_hash;

                            // CaseCarriers.words_length++;
                            // CaseCarriers.words = realloc(
                            //     CaseCarriers.words,
                            //     CaseCarriers.words_length*sizeof(word_t)
                            // );
                            //
                            // CaseCarriers.words[CaseCarriers.words_length-1] = (word_t){
                            //     .word = CaseName,
                            //     .word_type = StringHandleChar == '\'' ? WORD_SINGLE_STRING : WORD_DOUBLE_STRING,
                            // };

                            StringHandleChar = 0;

                            CaseNameLen = 0;
                            //free(CaseName);
                            CaseName = NULL;

                            break;
                        }
                    }
                case CHAR_NORMAL:
                case CHAR_NUMBER:
                case CHAR_CONNECTABLE:
                case CHAR_UNCONNECTABLE:
                case CHAR_SPACE:
                    {

                        CaseNameLen++;
                        CaseName = realloc(
                            CaseName,
                            CaseNameLen*sizeof(char)
                            );
                        CaseName[CaseNameLen-1] = ThisChar;

                        break;
                    }


                case CHAR_BS:
                    {
                        HandleType = 2;

                        break;
                    }

                default:break;
                }



                break;
            }
        case 2: //Super Character
            {
                if (SuperCharCollect == 0)
                {
                    switch (ThisCharType)
                    {
                    case CHAR_NORMAL:
                        {
                            SuperCharParentNameLen++;
                            SuperCharParentName = realloc(
                                SuperCharParentName,
                                SuperCharParentNameLen*sizeof(char)
                                );
                            SuperCharParentName[SuperCharParentNameLen-1] = ThisChar;

                            break;
                        }
                    case CHAR_UNCONNECTABLE:
                        {
                            if (ThisChar == '(')
                            {
                                SuperCharCollect = 1;
                                break;
                            }
                        }

                    default:
                        {
                            SuperCharCollect = 2;
                            break;
                        }
                    }

                    toCMPChar = realloc(
                        toCMPChar,
                        (SuperCharParentNameLen+1)*sizeof(char)
                        );
                    toCMPChar = memcpy(
                        toCMPChar,
                        SuperCharParentName,
                        SuperCharParentNameLen*sizeof(char)
                        );
                    toCMPChar[SuperCharParentNameLen] = 0;

                    if (strcmp(toCMPChar,"n") == 0)
                    {
                        SuperCharParentType = 1;
                        SuperCharCollect = 2;
                    }else if (strcmp(toCMPChar,"u") == 0)
                    {
                        SuperCharParentType = 2;
                        SuperCharCollect = 1;

                        break;
                    }else exit(301) ;

                    free(toCMPChar);
                }

                if (SuperCharCollect == 1){

                    switch (ThisCharType)
                    {
                    case CHAR_NORMAL:
                    case CHAR_NUMBER:
                        {
                            SuperCharChildNameLen++;
                            SuperCharChildName = realloc(
                                SuperCharChildName,
                                SuperCharChildNameLen*sizeof(char)
                                );
                            SuperCharChildName[SuperCharChildNameLen-1] = ThisChar;

                            break;
                        }

                    case CHAR_UNCONNECTABLE:
                        {
                            if (ThisChar == ')')
                                SuperCharCollect = 2;

                            break;
                        }

                    default:
                        {
                            exit(400);
                        };
                    }
                }

                if (SuperCharCollect == 2)
                {
                    if (SuperCharParentNameLen)
                    {
                        switch (SuperCharParentType)
                        {
                        case 1:
                            {

                                CaseNameLen++;
                                CaseName = realloc(
                                    CaseName,
                                    CaseNameLen*sizeof(char)
                                    );
                                CaseName[CaseNameLen-1] = '\n';



                                break;
                            }
                        case 2:
                            {

                                if (!(SuperCharChildNameLen >= 4 && SuperCharChildNameLen <= 6)) exit(302);

                                SuperCharChildNameLen++;
                                SuperCharChildName = realloc(
                                    SuperCharChildName,
                                    SuperCharChildNameLen*sizeof(char)
                                    );
                                SuperCharChildName[SuperCharChildNameLen-1] = 0;

                                unsigned long const u = strtoul(SuperCharChildName,NULL,16);

                                if (u <= (1<<7) - 1)
                                {
                                    CaseNameLen += 1;
                                    CaseName = realloc(
                                        CaseName,
                                        CaseNameLen*sizeof(char)
                                        );
                                    CaseName[CaseNameLen-1] = (char)u;
                                }else if (u <= (1<<11) - 1)
                                {
                                    CaseNameLen += 2;
                                    CaseName = realloc(
                                        CaseName,
                                        CaseNameLen*sizeof(char)
                                        );
                                    CaseName[CaseNameLen-2] = (char)(0xC0 | u >> 6);
                                    CaseName[CaseNameLen-1] = (char)(0x80 | u & (1<<6) - 1);
                                }else if (u <= (1<<16) - 1)
                                {
                                    CaseNameLen += 3;
                                    CaseName = realloc(
                                        CaseName,
                                        CaseNameLen*sizeof(char)
                                        );
                                    CaseName[CaseNameLen-3] = (char)(0xE0 | u >> 12) ;
                                    CaseName[CaseNameLen-2] = (char)(0x80 | (u >> 6) & (1<<6) - 1);
                                    CaseName[CaseNameLen-1] = (char)(0x80 | u & (1<<6) - 1);

                                }else if (u <= (1<<21) - 1)
                                {
                                    CaseNameLen += 4;
                                    CaseName = realloc(
                                        CaseName,
                                        CaseNameLen*sizeof(char)
                                        );

                                    CaseName[CaseNameLen-4] = (char)(0xF0 | u >> 18) ;
                                    CaseName[CaseNameLen-3] = (char)(0x80 | (u >> 12) & (1<<6) - 1);
                                    CaseName[CaseNameLen-2] = (char)(0x80 | (u >> 6) & (1<<6) - 1);
                                    CaseName[CaseNameLen-1] = (char)(0x80 | u & (1<<6) - 1);
                                }

                                free(SuperCharChildName);

                                break;
                            }
                        default:
                            {
                                exit(300);

                            };
                        }

                        free(SuperCharParentName);
                    }

                    if (SuperCharChildNameLen)
                    {

                    }

                    HandleType = 1;


                    SuperCharParentName = 0;
                    SuperCharParentNameLen = 0;
                    SuperCharParentType = 0;
                    SuperCharChildName = 0;
                    SuperCharChildNameLen = 0;
                    SuperCharCollect = 0;

                }
                break;
            }
        case 3: //註解
            {
                if (
                    (ThisChar == '\n' && CommentChar == '/')
                    ||
                    (LastChar == '*' && ThisChar == '/' && CommentChar == '*')
                    )
                {
                    CommentChar = 0;
                    HandleType = 0;
                }

                break;
            }
        default:
            {
               exit(200);
            };
        }

        if (ThisCharType == CHAR_NEWLINE)
        {
            ProcessingLine++;
            ProcessingColumn=1;
            continue;
        }


        if (ThisChar == END_FLAG)
            break;

        printf(":`%c` %d\n", ThisChar, ThisCharType);

        ProcessingColumn++;
        CharIndex++;
        LastChar = ThisChar;
        LastCharType = ThisCharType;
    }while (1);




    if (CommentChar == '*')
    {
        exit(100);
    }

    end:

    return word_hash_carrier;
}