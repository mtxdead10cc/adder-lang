#include "co_parser.h"
#include "co_tokenizer.h"
#include "co_cres.h"
#include "co_ast.h"
#include "sh_types.h"
#include "sh_utils.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool pa_init(parser_t* parser, char* text, size_t text_length, char* filepath) {

    parser->result = (cres_t) {0};
    
    if( tokens_init(&parser->collection, 16) == false ) {
        cres_set_error(&parser->result, R_ERR_OUT_OF_MEMORY);
        return false;
    }

    if( text == NULL ) {
        cres_set_error(&parser->result, R_ERR_INTERNAL);
        return false;
    }

    tokenizer_args_t args = (tokenizer_args_t) {
        .filepath = filepath,
        .text = text,
        .text_length = text_length,
        .include_comments = false,
        .include_spaces = false,
        .resultptr = &parser->result
    };

    if( tokenizer_analyze(&parser->collection, &args) == false ) {
        tokens_destroy(&parser->collection);
        parser->collection.count = 0;
        return false;
    }

    parser->cursor = 0;
    return cres_is_ok(&parser->result);
}

void pa_destroy(parser_t* parser) {
    if( parser == NULL ) {
        return;
    }
    tokens_destroy(&parser->collection);
}

bool pa_is_at_end(parser_t* parser) {
    return parser->cursor >= parser->collection.count;
}

bool pa_advance(parser_t* parser) {
    if( pa_is_at_end(parser) == false ) {

        parser->cursor ++;

#if PA_DEBUG > 1
        printf(">> advancing to %s ", token_get_type_name(pa_current_token(parser).type));
        srcref_print(pa_current_token(parser).ref);
        printf("\n");
#endif

        return true;
    }

#if PA_DEBUG > 1
    printf(">> parser is at end\n");
#endif

    return false;
}

bool pa_advance_if(parser_t* parser, token_type_t type) {
    if( pa_current_token(parser).type == type ) {
        return pa_advance(parser);
    }
    return false;
}

bool pa_advance_if_not(parser_t* parser, token_type_t type) {
    if( pa_current_token(parser).type != type) {
        return pa_advance(parser);
    }
    return false;
}

token_t pa_current_token(parser_t* parser) {
    return parser->collection.tokens[parser->cursor];
}

token_t pa_peek_token(parser_t* parser, int lookahead) {
    int diff = parser->collection.count - parser->cursor;
    if( diff < lookahead  ) {
        lookahead = diff;
    }
    return parser->collection.tokens[parser->cursor + lookahead];
}


void pa_set_error_unexpected_token_type(parser_t* parser, token_type_t expected, token_t actual) {
    if( cres_set_error(&parser->result, R_ERR_TOKEN) ) {
        cres_set_src_location(&parser->result, actual.ref);
        cres_msg_add_costr(&parser->result, "unexpected token, expected ");
        cres_msg_add_token_type_name(&parser->result, expected);
        cres_msg_add_costr(&parser->result, " but found ");
        cres_msg_add_token(&parser->result, actual);
    }
}

void pa_set_error_invalid_token_format(parser_t* parser, token_t token) {
    if( cres_set_error(&parser->result, R_ERR_TOKEN) ) {
        cres_set_src_location(&parser->result, token.ref);
        cres_msg_add_costr(&parser->result, "unexpected token format: ");
        cres_msg_add_token(&parser->result, token);
    }
}

pa_result_t pa_consume(parser_t* parser, token_type_t expected) {
    if( cres_has_error(&parser->result) ) {
        return par_error(&parser->result);
    }
    if( pa_is_at_end(parser) ) {
        return par_out_of_tokens();
    }
    token_t actual = pa_current_token(parser);
    if( expected != actual.type ) {
        pa_set_error_unexpected_token_type(parser, expected, actual);
        return par_error(&parser->result);
    }
    if( pa_advance(parser) == false ) {
        return par_out_of_tokens();
    }
    return par_nothing();
}

pa_result_t pa_parse_expression(parser_t* parser);
pa_result_t pa_parse_statement(parser_t* parser);

pa_result_t pa_parse_number(parser_t* parser) {
    token_t token = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_NUMBER);
    if( par_is_nothing(result) == false ) {
        return result;
    }
    float value = 0.0f;
    if( srcref_as_float(token.ref, &value) ) {
        return par_node(ast_number(value));
    }
    pa_set_error_invalid_token_format(parser, token);
    return par_error(&parser->result);
}

pa_result_t pa_parse_boolean(parser_t* parser) {
    token_t token = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_BOOLEAN);
    if( par_is_nothing(result) == false ) {
        return result;
    }
    bool value = false;
    if( srcref_as_bool(token.ref, &value) ) {
        return par_node(ast_bool(value));
    }
    pa_set_error_invalid_token_format(parser, token);
    return par_error(&parser->result);
}

pa_result_t pa_parse_string(parser_t* parser) {
    token_t token = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_STRING);
    if( par_is_nothing(result) == false ) {
        return result;
    }
    return par_node(ast_string(token.ref));
}

pa_result_t pa_try_parse_value(parser_t* parser) {
    token_t token = pa_current_token(parser);
    if( token.type == TT_BOOLEAN ) {
        return pa_parse_boolean(parser);
    } else if ( token.type == TT_NUMBER ) {
        return pa_parse_number(parser);
    } else if ( token.type == TT_STRING ) {
        return pa_parse_string(parser);
    }
    return par_nothing();
}

pa_result_t pa_try_parse_var_name(parser_t* parser) {
    token_t token = pa_current_token(parser);
    if( pa_advance_if(parser, TT_SYMBOL) ) {
        return par_node(ast_varref(token.ref));
    }
    return par_nothing();
}

pa_result_t pa_try_parse_group(parser_t* parser) {
    token_t token = pa_current_token(parser);
    if( token.type != TT_OPEN_PAREN )
        return par_nothing();
    if( pa_advance(parser) == false )
        return par_out_of_tokens();
    // todo: error message if empty paren
    pa_result_t result_inner = pa_parse_expression(parser);
    if( par_is_error(result_inner) ) {
        return result_inner;
    }
    pa_result_t result_consume = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_error(result_consume) ) {
        return result_consume;
    }
    return result_inner;
}

pa_result_t pa_try_parse_func_call(parser_t* parser) {

    token_t func_name = pa_current_token(parser);
    token_t open_paren = pa_peek_token(parser, 1);
    
    if( func_name.type != TT_SYMBOL || open_paren.type != TT_OPEN_PAREN ) {
        return par_nothing();
    }

    if( pa_advance(parser) == false ) {
        return par_out_of_tokens();
    }

    if( pa_advance(parser) == false ) {
        return par_out_of_tokens();
    }

    ast_node_t* args = ast_block();

    if( pa_advance_if(parser, TT_CLOSE_PAREN) ) {
        return par_node(ast_funcall(func_name.ref, args));
    }
    
    do {
        pa_result_t expr_res = pa_parse_expression(parser);
        if( par_is_node(expr_res) == false ) {
            ast_free(args);
            return expr_res;
        }
        ast_block_add(args, par_extract_node(expr_res));
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_result_t result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_nothing(result) == false ) {
        ast_free(args);
        return result;
    } else {
        return par_node(ast_funcall(func_name.ref, args));
    }
}

pa_result_t pa_try_parse_array_def(parser_t* parser) {

    if( pa_current_token(parser).type != TT_OPEN_SBRACKET ) {
        return par_nothing();
    }

    if( pa_advance(parser) == false ) {
        return par_out_of_tokens();
    }

    ast_node_t* array = ast_array();

    if( pa_advance_if(parser, TT_CLOSE_SBRACKET) ) {
        return par_node(array);
    }

    do {
        pa_result_t expr_res = pa_parse_expression(parser);
        if( par_is_node(expr_res) == false ) {
            ast_free(array);
            return expr_res;
        }
        ast_array_add(array, par_extract_node(expr_res));
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_result_t result = pa_consume(parser, TT_CLOSE_SBRACKET);

    if( par_is_nothing(result) == false ) {
        ast_free(array);
        return result;
    } else {
        return par_node(array);
    }
}

pa_result_t pa_try_parse_unary_operation(parser_t* parser, token_type_t tt, ast_unop_type_t op) {
    if( pa_advance_if(parser, tt) ) {
        pa_result_t inner = pa_parse_expression(parser);
        if( par_is_error(inner) )
            return inner;
        assert( par_is_nothing(inner) == false );
        return par_node(ast_unnop(op, par_extract_node(inner)));
    }
    return par_nothing();
}


pa_result_t pa_try_parse_binary_operation(pa_result_t lhs, parser_t* parser, token_type_t tt, ast_binop_type_t op) {
    if( pa_advance_if(parser, tt) ) {
        pa_result_t rhs = pa_parse_expression(parser);
        if( par_is_error(rhs) )
            return rhs;
        assert( par_is_nothing(rhs) == false );
        return par_node(ast_binop(op, 
            par_extract_node(lhs),
            par_extract_node(rhs)));
    }
    return par_nothing();
}


pa_result_t pa_parse_expression(parser_t* parser) {

    // parsing standard expressions

    pa_result_t result = pa_try_parse_group(parser);

    if( par_is_nothing(result) ) 
        result = pa_try_parse_func_call(parser);

    if( par_is_nothing(result) ) 
        result = pa_try_parse_value(parser);

    if( par_is_nothing(result) )  
        result = pa_try_parse_var_name(parser);

    if( par_is_nothing(result) )  
        result = pa_try_parse_array_def(parser);

    // parsing unary operator expressions

    if( par_is_nothing(result) ) 
        result = pa_try_parse_unary_operation(parser, TT_BINOP_MINUS, AST_UN_NEG);

    if( par_is_nothing(result) ) 
        result = pa_try_parse_unary_operation(parser, TT_UNOP_NOT, AST_UN_NOT);

    if( par_is_error(result) ) {
        return result;
    }

    if( par_is_node(result) == false ) {
        if( cres_set_error(&parser->result, R_ERR_EXPR) ) {
            token_t token = pa_current_token(parser);
            cres_set_src_location(&parser->result, token.ref);
            cres_msg_add_token(&parser->result, token);
            cres_msg_add_costr(&parser->result, " could not be made into an expression.");
        }
        return par_error(&parser->result);
    }

    // parsing binary operation expressions

    // todo: handle operator precedence

    pa_result_t bin_op_result = pa_try_parse_binary_operation(result, parser, TT_BINOP_AND, AST_BIN_AND);

    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_BINOP_OR, AST_BIN_OR);

    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_BINOP_MUL, AST_BIN_MUL);

    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_BINOP_DIV, AST_BIN_DIV);

    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_BINOP_MOD, AST_BIN_MOD);

    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_BINOP_MINUS, AST_BIN_SUB);

    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_BINOP_PLUS, AST_BIN_ADD);

    // TODO: THESE SHOULD NOT CONTINUE RECURSIVELY!!! ---------------------------

    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_CMP_EQ, AST_BIN_EQ);

    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_CMP_NEQ, AST_BIN_NEQ);

    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_CMP_LT, AST_BIN_LT);

    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_CMP_GT, AST_BIN_GT);
    
    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_CMP_GT_EQ, AST_BIN_GT_EQ);

    if( par_is_nothing(bin_op_result) )
        bin_op_result = pa_try_parse_binary_operation(result, parser, TT_CMP_LT_EQ, AST_BIN_LT_EQ);

    // -------------------------------------------------------------------------

    if( par_is_error(bin_op_result) ) {
        return bin_op_result;
    }

    if( par_is_node(bin_op_result) ) {
        return bin_op_result;
    } else {
        return result;
    }
}

// todo: types should perhaps not be encoded
// in the ast this way since it makes it hard
// to handle user defined types.

ast_value_type_t pa_value_type(srcref_t ref) {
    ast_value_type_t vtype = AST_VALUE_TYPE_NONE;
    if( srcref_equals_string(ref, "num") )
        vtype = AST_VALUE_TYPE_NUMBER;
    else if( srcref_equals_string(ref, "bol") )
        vtype = AST_VALUE_TYPE_BOOL;
    else if( srcref_equals_string(ref, "str") )
        vtype = AST_VALUE_TYPE_STRING;
    return vtype;
}

pa_result_t pa_parse_vardecl(parser_t* parser) {
    token_t typename = pa_current_token(parser);
    token_t varname = pa_peek_token(parser, 1);

    pa_result_t result = pa_consume(parser, TT_SYMBOL);

    if( par_is_nothing(result) )
        result = pa_consume(parser, TT_SYMBOL);

    if( par_is_error(result) )
        return result;

    assert( par_is_nothing(result) );

    return par_node(ast_vardecl(varname.ref,
                        pa_value_type(typename.ref)));
}

pa_result_t pa_try_parse_assignment(parser_t* parser) {
    if( pa_peek_token(parser, 1).type == TT_ASSIGN ) {
        token_t varname = pa_current_token(parser);
        pa_result_t result = pa_consume(parser, TT_SYMBOL);
        if( par_is_error(result) )
            return result;
        if( pa_advance(parser) == false ) // skip over '='
            return par_out_of_tokens(); 
        pa_result_t rhs = pa_parse_expression(parser); // rhs expr
        if( par_is_error(rhs) )
            return rhs;
        assert( par_is_nothing(rhs) == false );
        return par_node(ast_assign(
                ast_varref(varname.ref),
                par_extract_node(rhs)));

    } else if( pa_peek_token(parser, 2).type == TT_ASSIGN ) {
        pa_result_t decl = pa_parse_vardecl(parser);
        if( par_is_error(decl) )
            return decl;
        assert(par_is_nothing(decl) == false);
        pa_result_t assign = pa_consume(parser, TT_ASSIGN); // skip over '='
        if( par_is_error(assign) )
            return assign;
        pa_result_t rhs = pa_parse_expression(parser); // rhs expr
        if( par_is_error(rhs) )
            return rhs;
        assert(par_is_nothing(rhs) == false);
        return par_node(ast_assign(
                par_extract_node(decl),
                par_extract_node(rhs)));
    } else {
        return par_nothing();
    }
}

pa_result_t pa_parse_body(parser_t* parser) {

    pa_result_t result = pa_consume(parser, TT_OPEN_CURLY);
    if( par_is_error(result) )
        return result;

    ast_node_t* body_block = ast_block();

    do {
        if( pa_current_token(parser).type == TT_CLOSE_CURLY )
            break;

        pa_result_t stmt_res = pa_parse_statement(parser);
        if( par_is_node(stmt_res) == false ) {
            ast_free(body_block);
            return stmt_res;
        }

        ast_block_add(body_block, par_extract_node(stmt_res));
        
    } while( pa_is_at_end(parser) == false );

    result = pa_consume(parser, TT_CLOSE_CURLY);

    if( par_is_error(result) )
        return result;
    
    return par_node(body_block);
}

pa_result_t pa_try_parse_if_chain(parser_t* parser) {
    if( pa_advance_if(parser, TT_KW_IF) == false )
        return par_nothing();

    pa_result_t result = pa_consume(parser, TT_OPEN_PAREN);
    if( par_is_error(result) )
        return result;
    assert( par_is_nothing(result) );

    result = pa_parse_expression(parser);
    if( par_is_error(result) )
        return result;
    assert( par_is_nothing(result) == false );
    ast_node_t* condition = par_extract_node(result);

    result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_error(result) )
        return result;
    assert( par_is_nothing(result) );

    result = pa_parse_body(parser);
    if( par_is_error(result) )
        return result;
    assert( par_is_node(result) );

    ast_node_t* if_true = par_extract_node(result);

    ast_node_t* next = NULL;

    if( pa_advance_if(parser, TT_KW_ELSE) ) {
        result = pa_try_parse_if_chain(parser); // else if   
        if( par_is_nothing(result) ) {            
            result = pa_parse_body(parser);     // or last else
        }
        if( par_is_error(result) )
            return result;
        next = par_extract_node(result);
    } else {
        next = ast_block();                     // empty / nothing
    }

    return par_node(ast_if(condition, if_true, next));
}

pa_result_t pa_try_parse_for_stmt(parser_t* parser) {
    if( pa_advance_if(parser, TT_KW_FOR) == false )
        return par_nothing();

    pa_result_t result = pa_consume(parser, TT_OPEN_PAREN);
    if( par_is_error(result) )
        return result;
    assert( par_is_nothing(result) );

    result = pa_parse_vardecl(parser);
    if( par_is_error(result) )
        return result;
    assert( par_is_nothing(result) == false );
    ast_node_t* vardecl = par_extract_node(result);

    result = pa_consume(parser, TT_KW_IN);
    if( par_is_error(result) )
        return result;
    assert( par_is_nothing(result) );

    result = pa_parse_expression(parser);
    if( par_is_error(result) )
        return result;
    assert( par_is_node(result) );
    ast_node_t* collection = par_extract_node(result);

    result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_error(result) )
        return result;
    assert( par_is_nothing(result) );

    // todo: verify collection

    result = pa_parse_body(parser);
    if( par_is_error(result) )
        return result;
    assert( par_is_node(result) );

    ast_node_t* body = par_extract_node(result);

    return par_node(ast_foreach(vardecl, collection, body));
}

pa_result_t pa_try_parse_body_break(parser_t* parser) {
    if( pa_advance_if(parser, TT_KW_BREAK) )
        return par_node(ast_break());
    return par_nothing();
}

pa_result_t pa_try_parse_body_return(parser_t* parser) {
    if( pa_advance_if(parser, TT_KW_RETURN) ) {
        pa_result_t result = pa_parse_expression(parser);
        if( par_is_error(result) )
            return result;
        if( par_is_nothing(result) )
            return par_node(ast_return(ast_block())); // empty block for "nothing"
        else
            return par_node(ast_return(par_extract_node(result)));
    }
    return par_nothing();
}

pa_result_t pa_parse_statement(parser_t* parser) {

    pa_result_t result = pa_try_parse_assignment(parser);

    // todo: if-else? elifs?

    if( par_is_nothing(result) )
        result = pa_try_parse_if_chain(parser);

    if( par_is_nothing(result) )
        result = pa_try_parse_for_stmt(parser);

    if( par_is_nothing(result) )
        result = pa_try_parse_func_call(parser);

    if( par_is_nothing(result) )
        result = pa_try_parse_body_return(parser);

    if( par_is_nothing(result) )
        result = pa_try_parse_body_break(parser);

    if( par_is_error(result) )
        return result;

    if( par_is_node(result) == false ) {
        if( cres_set_error(&parser->result, R_ERR_STATEMENT) ) {
            token_t token = pa_current_token(parser);
            cres_set_src_location(&parser->result, token.ref);
            cres_msg_add_token(&parser->result, token);
            cres_msg_add_costr(&parser->result, " is not a valid statement.");
        }
        return par_error(&parser->result);
    }

    // optional end-of-statement (for now)
    pa_advance_if(parser, TT_STATEMENT_END);

    return result;
}

pa_result_t pa_parse_funsign(parser_t* parser) {
    token_t rettype = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_SYMBOL);
    if( par_is_error(result) )
        return result;

    token_t funname = pa_current_token(parser);
    result = pa_consume(parser, TT_SYMBOL);
    if( par_is_error(result) )
        return result;
    
    return par_node(ast_funsign(funname.ref,
        pa_value_type(rettype.ref)));
}

pa_result_t pa_try_parse_fundecl(parser_t* parser) {

    if( pa_peek_token(parser, 0).type != TT_SYMBOL
     || pa_peek_token(parser, 1).type != TT_SYMBOL
     || pa_peek_token(parser, 2).type != TT_OPEN_PAREN )
        return par_nothing();

    pa_result_t result = pa_parse_funsign(parser);
    if( par_is_error(result) )
        return result;

    ast_node_t* funsign = par_extract_node(result);
    
    result = pa_consume(parser, TT_OPEN_PAREN);
    if( par_is_error(result) )
        return result;

    ast_node_t* argspec = ast_block();

    do {
        if( pa_current_token(parser).type == TT_CLOSE_PAREN )
            break;

        pa_result_t vardecl = pa_parse_vardecl(parser);
        if( par_is_node(vardecl) == false ) {
            ast_free(argspec);
            return vardecl;
        }

        ast_block_add(argspec, par_extract_node(vardecl));
        
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_error(result) )
        return result;

    result = pa_parse_body(parser);
    if( par_is_error(result) )
        return result;

    return par_node(ast_fundecl( funsign,
                                 argspec,
                                 par_extract_node(result) ));
}

pa_result_t pa_parse_program(parser_t* parser) {

    pa_result_t consume_result = pa_consume(parser, TT_INITIAL);
    if( par_is_error(consume_result) )
        return consume_result;

    pa_result_t program_result;
    ast_node_t* program = ast_block();
    do {
        program_result = pa_try_parse_fundecl(parser);
        if( par_is_node(program_result) ) {
            ast_block_add(program,
                par_extract_node(program_result));
        } else {
            break;
        }
    } while( pa_is_at_end(parser) == false );

    consume_result = pa_consume(parser, TT_FINAL);

    if( par_is_error(program_result) ) {
        ast_free(program);
        return program_result;
    }

    if( par_is_error(consume_result) ) {
        ast_free(program);
        return consume_result;
    }

    return par_node(program);
}
