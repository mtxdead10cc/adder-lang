#ifndef GVM_PARSER_H_
#define GVM_PARSER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include "gvm_types.h"
#include "gvm_utils.h"
#include "gvm_tokenizer.h"

typedef struct parser_t {
    token_collection_t  collection;
    size_t              cursor;
} parser_t;

bool parser_init(parser_t* parser, char* text, size_t text_length, char* filepath);

#endif // GVM_PARSER_H_