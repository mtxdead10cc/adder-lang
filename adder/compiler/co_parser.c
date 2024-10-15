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

pa_result_t pa_init(parser_t* parser, arena_t* arena, char* text, size_t text_length, char* filepath) {

    parser->result = (cres_t) { 0 };
    
    if( tokens_init(&parser->collection, 16) == false ) {
        cres_set_error(&parser->result, R_ERR_OUT_OF_MEMORY);
        return par_error(parser);
    }

    if( text == NULL ) {
        cres_set_error(&parser->result, R_ERR_INTERNAL);
        return par_error(parser);
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
        return par_error(parser);
    }

    parser->arena = arena;
    parser->cursor = 0;
    return par_nothing();
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

pa_result_t pa_error_out_of_tokens(parser_t* parser) {
    if( cres_set_error(&parser->result, R_ERR_TOKEN) ) {
        cres_msg_add_costr(&parser->result, "unexpected end of token stream.");
    }
    return par_error(parser);
}

pa_result_t pa_error_unexpected_token_type(parser_t* parser, token_type_t expected, token_t actual) {
    if( cres_set_error(&parser->result, R_ERR_TOKEN) ) {
        cres_set_src_location(&parser->result, actual.ref);
        cres_msg_add_costr(&parser->result, "unexpected token, expected ");
        cres_msg_add_token_type_name(&parser->result, expected);
        cres_msg_add_costr(&parser->result, " but found ");
        cres_msg_add_token_type_name(&parser->result, actual.type);
        cres_msg_add_costr(&parser->result, " ('");
        cres_msg_add(&parser->result,
            srcref_ptr(actual.ref),
            srcref_len(actual.ref));
        cres_msg_add_costr(&parser->result, "')");
    }
    return par_error(parser);
}

pa_result_t pa_error_invalid_token_format(parser_t* parser, token_t token) {
    if( cres_set_error(&parser->result, R_ERR_TOKEN) ) {
        cres_set_src_location(&parser->result, token.ref);
        cres_msg_add_costr(&parser->result, "unexpected token format: ");
        cres_msg_add_token_type_name(&parser->result, token.type);
        cres_msg_add_costr(&parser->result, " ('");
        cres_msg_add(&parser->result,
            srcref_ptr(token.ref),
            srcref_len(token.ref));
        cres_msg_add_costr(&parser->result, "')");
    }
    return par_error(parser);
}

pa_result_t _pa_set_error(parser_t* parser, cres_code_t rescode, token_t token, char* expected_str) {
    if( cres_set_error(&parser->result, rescode) ) {
        cres_set_src_location(&parser->result, token.ref);
        cres_msg_add_costr(&parser->result, "unexpected statement: ");
        cres_msg_add_token_type_name(&parser->result, token.type);
        cres_msg_add_costr(&parser->result, " ('");
        cres_msg_add(&parser->result,
            srcref_ptr(token.ref),
            srcref_len(token.ref));
        cres_msg_add_costr(&parser->result, "') ");
        if( expected_str != NULL ) {
            cres_msg_add(&parser->result, expected_str, strlen(expected_str));
        }
    }
    return par_error(parser);
}

pa_result_t pa_error_invalid_expression(parser_t* parser, token_t token, char* expected_str) {
    return _pa_set_error(parser, R_ERR_EXPR, token, expected_str);
}

pa_result_t pa_error_invalid_statement(parser_t* parser, token_t token, char* expected_str) {
    return _pa_set_error(parser, R_ERR_STATEMENT, token, expected_str);
}


pa_result_t pa_consume(parser_t* parser, token_type_t expected) {
    if( cres_has_error(&parser->result) ) {
        return par_error(parser);
    }
    if( pa_is_at_end(parser) ) {
        return pa_error_out_of_tokens(parser);
    }
    token_t actual = pa_current_token(parser);
    if( expected != actual.type ) {
        return pa_error_unexpected_token_type(parser, expected, actual);
    }
    pa_advance(parser); // do not check eof here
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
        return par_node(ast_float(parser->arena, value));
    }
    return pa_error_invalid_token_format(parser, token);
}

pa_result_t pa_parse_boolean(parser_t* parser) {
    token_t token = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_BOOLEAN);
    if( par_is_nothing(result) == false ) {
        return result;
    }
    bool value = false;
    if( srcref_as_bool(token.ref, &value) ) {
        return par_node(ast_bool(parser->arena, value));
    }
    return pa_error_invalid_token_format(parser, token);
}

pa_result_t pa_parse_string(parser_t* parser) {
    token_t token = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_STRING);
    if( par_is_nothing(result) == false ) {
        return result;
    }
    return par_node(ast_string(parser->arena, token.ref));
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
        return par_node(ast_varref(parser->arena, token.ref));
    }
    return par_nothing();
}

pa_result_t pa_try_parse_group(parser_t* parser) {
    token_t token = pa_current_token(parser);
    if( token.type != TT_OPEN_PAREN )
        return par_nothing();
    if( pa_advance(parser) == false )
        return pa_error_out_of_tokens(parser);
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

    pa_advance(parser);
    pa_advance(parser);

    ast_node_t* args = ast_block(parser->arena);

    if( pa_advance_if(parser, TT_CLOSE_PAREN) ) {
        return par_node(ast_funcall(parser->arena, func_name.ref, args));
    }
    
    do {
        pa_result_t expr_res = pa_parse_expression(parser);
        if( par_is_node(expr_res) == false ) {
            return expr_res;
        }
        ast_block_add(parser->arena, args, par_extract_node(expr_res));
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_result_t result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_nothing(result) == false ) {
        return result;
    } else {
        return par_node(ast_funcall(parser->arena, func_name.ref, args));
    }
}

pa_result_t pa_try_parse_array_def(parser_t* parser) {

    if( pa_current_token(parser).type != TT_OPEN_SBRACKET ) {
        return par_nothing();
    }

    pa_advance(parser);

    ast_node_t* array = ast_array(parser->arena);

    if( pa_advance_if(parser, TT_CLOSE_SBRACKET) ) {
        return par_node(array);
    }

    do {
        pa_result_t expr_res = pa_parse_expression(parser);
        if( par_is_node(expr_res) == false ) {
            return expr_res;
        }
        ast_array_add(parser->arena, array, par_extract_node(expr_res));
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_result_t result = pa_consume(parser, TT_CLOSE_SBRACKET);

    if( par_is_nothing(result) == false ) {
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
        return par_node(ast_unnop(parser->arena, op, par_extract_node(inner)));
    }
    return par_nothing();
}


pa_result_t pa_try_parse_binary_operation(pa_result_t lhs, parser_t* parser, token_type_t tt, ast_binop_type_t op) {
    if( pa_advance_if(parser, tt) ) {
        pa_result_t rhs = pa_parse_expression(parser);
        if( par_is_error(rhs) )
            return rhs;
        assert( par_is_nothing(rhs) == false );
        return par_node(ast_binop(parser->arena, op, 
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
        return pa_error_invalid_expression(parser, pa_current_token(parser), NULL);
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

pa_result_t pa_parse_vardecl(parser_t* parser) {
    token_t typename = pa_current_token(parser);
    token_t varname = pa_peek_token(parser, 1);

    pa_result_t result = pa_consume(parser, TT_SYMBOL);

    if( par_is_nothing(result) )
        result = pa_consume(parser, TT_SYMBOL);

    if( par_is_error(result) )
        return result;

    assert( par_is_nothing(result) );

    return par_node(ast_vardecl(parser->arena, varname.ref,
                        srcref_as_sstr(typename.ref)));
}

pa_result_t pa_try_parse_assignment(parser_t* parser) {
    if( pa_peek_token(parser, 1).type == TT_ASSIGN ) {
        token_t varname = pa_current_token(parser);
        pa_result_t result = pa_consume(parser, TT_SYMBOL);
        if( par_is_error(result) )
            return result;
        if( pa_advance(parser) == false ) // skip over '='
            return pa_error_out_of_tokens(parser);
        pa_result_t rhs = pa_parse_expression(parser); // rhs expr
        if( par_is_error(rhs) )
            return rhs;
        assert( par_is_nothing(rhs) == false );
        return par_node(ast_assign(parser->arena, 
                ast_varref(parser->arena, varname.ref),
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
        return par_node(ast_assign(parser->arena, 
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

    ast_node_t* body_block = ast_block(parser->arena);

    do {
        if( pa_current_token(parser).type == TT_CLOSE_CURLY )
            break;

        pa_result_t stmt_res = pa_parse_statement(parser);
        if( par_is_node(stmt_res) == false ) {
            return stmt_res;
        }

        ast_block_add(parser->arena, body_block, par_extract_node(stmt_res));
        
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
        next = ast_block(parser->arena);                     // empty / nothing
    }

    return par_node(ast_if(parser->arena, condition, if_true, next));
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

    return par_node(ast_foreach(parser->arena, vardecl, collection, body));
}

pa_result_t pa_try_parse_body_break(parser_t* parser) {
    if( pa_advance_if(parser, TT_KW_BREAK) )
        return par_node(ast_break(parser->arena));
    return par_nothing();
}

pa_result_t pa_try_parse_body_return(parser_t* parser) {
    if( pa_advance_if(parser, TT_KW_RETURN) ) {
        pa_result_t result = pa_parse_expression(parser);
        if( par_is_error(result) )
            return result;
        if( par_is_nothing(result) )
            return par_node(ast_return(parser->arena, ast_block(parser->arena))); // empty block for "nothing"
        else
            return par_node(ast_return(parser->arena, par_extract_node(result)));
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
        return pa_error_invalid_statement(parser, pa_current_token(parser), NULL);
    }

    // optional end-of-statement (for now)
    pa_advance_if(parser, TT_STATEMENT_END);

    return result;
}

pa_result_t pa_parse_funsign(parser_t* parser, ast_funsign_type_t decltype) {
    token_t rettype = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_SYMBOL);
    if( par_is_error(result) )
        return result;

    token_t funname = pa_current_token(parser);
    result = pa_consume(parser, TT_SYMBOL);
    if( par_is_error(result) )
        return result;

    result = pa_consume(parser, TT_OPEN_PAREN);
    if( par_is_error(result) )
        return result;

    ast_node_t* argspec = ast_block(parser->arena);

    do {
        if( pa_current_token(parser).type == TT_CLOSE_PAREN )
            break;

        pa_result_t vardecl = pa_parse_vardecl(parser);
        if( par_is_node(vardecl) == false ) {
            return vardecl;
        }

        ast_block_add(parser->arena, argspec, par_extract_node(vardecl));
        
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_error(result) )
        return result;
    
    return par_node(ast_funsign(parser->arena, funname.ref,
        argspec, decltype,
        srcref_as_sstr(rettype.ref)));
}

pa_result_t pa_try_parse_fundecl(parser_t* parser) {

    if( pa_peek_token(parser, 0).type != TT_SYMBOL
     || pa_peek_token(parser, 1).type != TT_SYMBOL
     || pa_peek_token(parser, 2).type != TT_OPEN_PAREN )
        return par_nothing();

    pa_result_t result = pa_parse_funsign(parser, AST_FUNSIGN_INTERN);
    if( par_is_error(result) )
        return result;

    ast_node_t* funsign = par_extract_node(result);
    
    result = pa_parse_body(parser);
    if( par_is_error(result) )
        return result;

    return par_node(ast_fundecl( parser->arena, funsign,
                                 par_extract_node(result) ));
}

pa_result_t pa_try_parse_preproc_directive(parser_t* parser) {
    if( pa_advance_if(parser, TT_PREPROC) == false )
        return par_nothing();
    token_t directive = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_SYMBOL);
    if( par_is_error(result) )
        return result;
    if( srcref_equals_string(directive.ref, "extern") == false ) {
        return pa_error_invalid_statement(parser, directive, " expected 'extern'.");
    }
    result = pa_parse_funsign(parser, AST_FUNSIGN_EXTERN);
    if( par_is_error(result) )
        return result;
    pa_advance_if(parser, TT_STATEMENT_END); // optional end of statement
    return result;
}

pa_result_t pa_parse_toplevel_statement(parser_t* parser) {
    pa_result_t result = pa_try_parse_fundecl(parser);
    if( par_is_nothing(result) )
        result = pa_try_parse_preproc_directive(parser);
    if( par_is_error(result) || par_is_node(result) )
        return result;
    return pa_error_invalid_statement(parser,
        pa_current_token(parser),
        " expected top-level statement (function "
        "declaration or preprocessor directive).");
}

pa_result_t pa_parse_program(parser_t* parser) {

    pa_result_t consume_result = pa_consume(parser, TT_INITIAL);
    if( par_is_error(consume_result) )
        return consume_result;

    pa_result_t result;
    ast_node_t* program = ast_block(parser->arena);

    do {
        result = pa_parse_toplevel_statement(parser);
        if( par_is_node(result) ) {
            ast_block_add(parser->arena, program,
                par_extract_node(result));
        } else {
            break;
        }
    } while(    !pa_is_at_end(parser)
             && !pa_advance_if(parser, TT_FINAL) );

    if( par_is_error(result) ) {
        return result;
    }

    if( par_is_error(consume_result) ) {
        return result;
    }

    return par_node(program);
}
