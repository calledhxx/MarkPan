#include <stdio.h>
#include "tokenizer.h"

#include <stdlib.h>


int static char_type(char const c)
{
    if (c == EOF || c == 0) return CHARACTER_TYPE_NULL;

    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return CHARACTER_TYPE_NORMAL;

    if (c >= '0' && c <= '9') return CHARACTER_TYPE_NUMBER;

    if (c == ' ' || c == '\t') return CHARACTER_TYPE_SPACE;

    switch (c)
    {
    case '*':
    case '/':
    case '+':
    case '=':
    case '^':
    case '<':
    case '>':
    case '.':
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '-':
    case ',':
    case ':':
    case '@':
    case '\'':
    case '"':
    case '\\':
    case '\n':
    case ';':
        return CHARACTER_TYPE_SYMBOL;
        default: break;
    }

    return CHARACTER_TYPE_NORMAL;
}

object_carrier_t stream_to_objects(FILE * const stream, char const FLAG_OF_END)
{
    object_carrier_t result = {0};

    char c = 0;
    int ctype = 0;

    struct
    {
        //
        unsigned int BodyRecording:1;
        unsigned int HeadAttributesRecording:1;
        //

        int Handler;
    } state = {0};

    char * name = 0;
    unsigned name_length = 0;

    do
    {
        c = fgetc(stream);
        ctype = char_type(c);

        if (state.BodyRecording)
        {
            switch (state.Handler)
            {
            }
        }else //不確定是否為Body或者是Head在紀錄了
        {
            if (ctype == CHARACTER_TYPE_NORMAL)
            {
                ++name_length;
                name = realloc(name, name_length * sizeof(char));
                name[name_length - 1] = c;
            }else
            {
                if (state.HeadAttributesRecording)
                {
                    if (c == '}') //Head屬性記錄結束
                    {
                        state.HeadAttributesRecording = 0;
                    }else
                    {
                        ++name_length;
                        name = realloc(name, name_length * sizeof(char));
                        name[name_length - 1] = c;
                    }
                }else
                {
                    if (c == '{') //開始記錄Head屬性
                    {
                        state.HeadAttributesRecording = 1;
                    }else //不規範在Head紀錄裡 理解為Body
                    {
                        state.BodyRecording = 1;
                    }
                }

            }
        }

    } while (c == FLAG_OF_END);

    return result;
}