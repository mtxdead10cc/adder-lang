#include "gvm_parser.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gvm_types.h"
#include "gvm_utils.h"


bool parser_init(parser_t* parser, char* text, size_t text_length, char* filepath) {
    assert(false && "Not implemented");
    return false;
}

bool parser_is_at_end(parser_t* parser) {
    return parser->cursor >= (parser->collection.count - 1);
}
