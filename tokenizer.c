#include <stdio.h>
#include "tokenizer.h"

#include <stdlib.h>
#include <string.h>


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

    char last_c = 0;
    int last_ctype = 0;


    struct
    {
        //
        unsigned int BodyRecording:1;
        unsigned int HeadAttributesRecording:1;
        char * HeadAttributesKeyNameRecording;
        unsigned int HeadAttributesValueUnitRecording:1;
        //

        section_attribute_t SectionAttributes;
        head_attribute_t HeadAttributes;
        dimension_t Dimension ;

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
                if (state.HeadAttributesRecording) //已經在紀錄屬性了
                {
                    if (c == '}') //Head屬性記錄結束
                    {
                        state.HeadAttributesRecording = 0;
                    }else
                    {
                        if (state.HeadAttributesKeyNameRecording) //紀錄value或是,
                        {
                            if (c == ',')
                            {
                                state.HeadAttributesKeyNameRecording = 0;

                                //name是單位
                                ++name_length;
                                name = realloc(name, name_length * sizeof(char));
                                name[name_length - 1] = 0;

                                state.Dimension.unit = memcpy(
                                malloc(sizeof(char) * name_length),
                                name,
                                sizeof(char) * name_length
                                );

                                name_length = 0;
                                free(name);
                                name = NULL;

                                if (strcmp(state.HeadAttributesKeyNameRecording,"width")==0)
                                {
                                    state.HeadAttributes.width = state.Dimension;
                                    state.Dimension = (dimension_t){0};
                                }
                            }else
                            {
                                if (last_ctype == CHARACTER_TYPE_NUMBER && ctype == CHARACTER_TYPE_NORMAL)
                                {
                                    state.HeadAttributesValueUnitRecording = 1;

                                    //name是純值
                                    ++name_length;
                                    name = realloc(name, name_length * sizeof(char));
                                    name[name_length - 1] = 0;

                                    state.Dimension.value = strtol(name,0,10);

                                    name_length = 0;
                                    free(name);
                                    name = NULL;
                                }

                                if (state.HeadAttributesValueUnitRecording)
                                {
                                    ++name_length;
                                    name = realloc(name, name_length * sizeof(char));
                                    name[name_length - 1] = c;
                                }
                            }
                        }else //紀錄key或是=
                        {
                            if (c == '=')
                            {
                                ++name_length;
                                name = realloc(name, name_length * sizeof(char));
                                name[name_length - 1] = 0;

                                state.HeadAttributesKeyNameRecording = memcpy(
                                malloc(sizeof(char) * name_length),
                                name,
                                sizeof(char) * name_length
                                );

                                name_length = 0;
                                free(name);
                                name = NULL;
                            }else
                            {
                                ++name_length;
                                name = realloc(name, name_length * sizeof(char));
                                name[name_length - 1] = c;
                            }
                        }

                    }
                }else //還沒紀錄屬性
                {
                    if (c == '{') //開始記錄Head屬性
                    {
                        state.HeadAttributesRecording = 1;

                    }else //不規範在Head紀錄裡 理解為Body 結束當前Head
                    {
                        state.BodyRecording = 1;

                        ++name_length;
                        name = realloc(name, name_length * sizeof(char));
                        name[name_length - 1] = 0;

                        ++result.objects_count;
                        result.objects = realloc(
                            result.objects,
                            result.objects_count * sizeof(object_t));
                        result.objects[result.objects_count - 1] = (object_t){
                            .head = (head_t){
                                .head = memcpy(
                                malloc(sizeof(char) * name_length),
                                name,
                                sizeof(char) * name_length
                                ),
                                .attributes = state.HeadAttributes,
                            }
                        };

                        state.HeadAttributes = (head_attribute_t){0};

                        name_length = 0;
                        free(name);
                        name = NULL;
                    }
                }

            }
        }

        last_c = c;
        last_ctype = ctype;
    } while (c == FLAG_OF_END);

    return result;
}