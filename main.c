#include <stdio.h>
#include <stdlib.h>

#include "word.h"

int main(void)
{
    FILE * f = fopen("./test.mp","r");

    word_hash_table_t word_hash_table = set_word_hash_table((word_hash_table_t){0},16);

    word_hash_carrier_t const WordCarrier = stream_to_word(f,EOF,&word_hash_table);

    for (int i = 0;i<WordCarrier.hashes_length;i++)
    {
        unsigned long const hash = WordCarrier.hashes[i];

        word_hash_item_t const word_hash_item = get_word_hash_item(word_hash_table,hash);

        printf("%lu `%s` %d\n",hash,word_hash_item.word.name,word_hash_item.word.word_type);
    }

    return 0;
}
