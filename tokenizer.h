#ifndef TOKENIZER_H
#define TOKENIZER_H

enum character_type_e
{
    CHARACTER_TYPE_NULL,
    CHARACTER_TYPE_NORMAL,
    CHARACTER_TYPE_NUMBER,
    CHARACTER_TYPE_SYMBOL,
    CHARACTER_TYPE_SPACE,
};

enum handler_type_e
{
    HANDLER_TYPE_NORMAL,
    HANDLER_TYPE_STRING,
};

typedef struct dimension_s
{
    char* unit;
    double value;
} dimension_t;

typedef struct section_attribute_s
{
    unsigned int bold:1;
    unsigned int italic:1;
    unsigned int underline:1;
    unsigned int spoiler:1;
} section_attribute_t;

typedef struct section_s
{
    char* content;
    section_attribute_t attributes;
} section_t;

typedef struct head_attribute_s
{
    dimension_t width;
    dimension_t height;
    char* method;
} head_attribute_t;

typedef struct head_s
{
    char* head;
    head_attribute_t attributes;
} head_t;

typedef struct body_s
{
    section_t * sections;
    unsigned sections_count;
} body_t;

typedef struct object_s
{
    head_t head;
    body_t body;
} object_t;

typedef struct object_carrier_s
{
    object_t* objects;
    unsigned objects_count;
} object_carrier_t;

#endif
