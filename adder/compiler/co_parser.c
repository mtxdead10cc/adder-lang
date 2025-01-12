#include "co_parser.h"
#include "co_tokenizer.h"
#include "co_trace.h"
#include "co_ast.h"
#include "sh_types.h"
#include "sh_utils.h"
#include <sh_log.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pa_result_t pa_init(parser_t* parser, arena_t* arena, trace_t* trace, char* text, size_t text_length, char* filepath) {

    parser->trace = trace;

    trace_set_current_source_path(trace, filepath);
    
    if( tokens_init(&parser->collection, 16) == false ) {
        trace_out_of_memory_error(trace);
        return par_error();
    }

    if( text == NULL ) {
        trace_msg_t* msg = trace_create_message(trace, TM_ERROR, trace_no_ref());
        trace_msg_append_costr(msg, "the input text buffer pointer was null.");
        return par_error();
    }

    tokenizer_args_t args = (tokenizer_args_t) {
        .filepath = filepath,
        .text = text,
        .text_length = text_length,
        .include_comments = false,
        .include_spaces = false,
        .trace = trace
    };

    if( tokenizer_analyze(&parser->collection, &args) == false ) {
        tokens_destroy(&parser->collection);
        parser->collection.count = 0;
        trace_msg_t* msg = trace_create_message(trace, TM_ERROR, trace_no_ref());
        trace_msg_append_costr(msg, "the input text buffer pointer was null.");
        return par_error();
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
        return true;
    }
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
    trace_msg_t* msg = trace_create_message(parser->trace, TM_ERROR, trace_no_ref());
    trace_msg_append_costr(msg, "unexpected end of token stream.");
    return par_error();
}

pa_result_t pa_error_unexpected_token_type(parser_t* parser, token_type_t expected, token_t actual) {
    trace_msg_t* msg = trace_create_message(parser->trace, TM_ERROR, actual.ref);
    trace_msg_append_costr(msg, "unexpected token, expected ");
    trace_msg_append_token_type_name(msg, expected);
    trace_msg_append_costr(msg, " but found ");
    trace_msg_append_token_type_name(msg, actual.type);
    trace_msg_append_costr(msg, " ('");
    trace_msg_append(msg,
        srcref_ptr(actual.ref),
        srcref_len(actual.ref));
    trace_msg_append_costr(msg, "')");
    return par_error();
}

pa_result_t pa_error_invalid_token_format(parser_t* parser, token_t token) {
    trace_msg_t* msg = trace_create_message(parser->trace, TM_ERROR, token.ref);   
    trace_msg_append_costr(msg, "unexpected token format: ");
    trace_msg_append_token_type_name(msg, token.type);
    trace_msg_append_costr(msg, " ('");
    trace_msg_append(msg,
        srcref_ptr(token.ref),
        srcref_len(token.ref));
    trace_msg_append_costr(msg, "')");
    return par_error();
}

pa_result_t _pa_set_error(parser_t* parser, token_t token, char* expected_str) {
    trace_msg_t* msg = trace_create_message(parser->trace, TM_ERROR, token.ref);
    trace_msg_append_costr(msg, "unexpected statement: ");
    trace_msg_append_token_type_name(msg, token.type);
    trace_msg_append_costr(msg, " ('");
    trace_msg_append(msg,
        srcref_ptr(token.ref),
        srcref_len(token.ref));
    trace_msg_append_costr(msg, "') ");
    if( expected_str != NULL ) {
        trace_msg_append(msg, expected_str, strlen(expected_str));
    }
    return par_error();
}

pa_result_t pa_error_invalid_expression(parser_t* parser, token_t token, char* expected_str) {
    return _pa_set_error(parser, token, expected_str);
}

pa_result_t pa_error_invalid_statement(parser_t* parser, token_t token, char* expected_str) {
    return _pa_set_error(parser, token, expected_str);
}


pa_result_t pa_consume(parser_t* parser, token_type_t expected) {
    if( trace_get_error_count(parser->trace) > 0 ) {
        return par_error();
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
        if( srcref_contains_char(token.ref, '.') )
            return par_node(ast_float(parser->arena, value), &token.ref);
        else
            return par_node(ast_int(parser->arena, value), &token.ref);
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
        return par_node(ast_bool(parser->arena, value), &token.ref);
    }
    return pa_error_invalid_token_format(parser, token);
}

pa_result_t pa_parse_string(parser_t* parser) {
    token_t token = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_STRING);
    if( par_is_nothing(result) == false ) {
        return result;
    }
    return par_node(ast_string(parser->arena, token.ref), &token.ref);
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
        return par_node(ast_varref(parser->arena, token.ref), NULL);
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
    result_inner.group_expression = true; // mark as group expression
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

    ast_node_t* args = ast_arglist(parser->arena);

    if( pa_advance_if(parser, TT_CLOSE_PAREN) ) {
        return par_node(ast_funcall(parser->arena, func_name.ref, args), NULL);
    }
    
    do {
        pa_result_t expr_res = pa_parse_expression(parser);
        if( par_is_node(expr_res) == false ) {
            return expr_res;
        }
        ast_arglist_add(parser->arena, args, par_extract_node(expr_res));
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_result_t result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_nothing(result) == false ) {
        return result;
    } else {
        return par_node(ast_funcall(parser->arena, func_name.ref, args), NULL);
    }
}

pa_result_t pa_try_parse_array_def(parser_t* parser) {

    if( pa_current_token(parser).type != TT_OPEN_SBRACKET ) {
        return par_nothing();
    }

    pa_advance(parser);

    srcref_t ref = pa_current_token(parser).ref;

    ast_node_t* array = ast_array(parser->arena);

    if( pa_advance_if(parser, TT_CLOSE_SBRACKET) ) {
        return par_node(array, &ref);
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
        return par_node(array, &ref);
    }
}

pa_result_t pa_try_parse_unary_operation(parser_t* parser, token_type_t tt, ast_unop_type_t op) {
    token_t token = pa_current_token(parser);
    if( pa_advance_if(parser, tt) ) {
        pa_result_t inner = pa_parse_expression(parser);
        if( par_is_error(inner) )
            return inner;
        assert( par_is_nothing(inner) == false );
        // find the innermost left expression
        // and negate that
        ast_node_t* iexp = par_extract_node(inner);
        // grouped expressions should be wrapped
        // so this recursion does not trigger 
        // on for example "-(a + b)".
        if( iexp->type == AST_BINOP && inner.group_expression == false ) {
            ast_node_t* current = iexp;
            while( current->u.n_binop.left->type == AST_BINOP ) {
                current = current->u.n_binop.left;
            }
            current->u.n_binop.left = ast_unnop(parser->arena, op,
                current->u.n_binop.left);
            return par_node(iexp, &token.ref);
        }
        return par_node(ast_unnop(parser->arena, op, iexp), &token.ref);
    }
    return par_nothing();
}

int get_precedence(ast_binop_type_t bin_op_type) {
    return 100 - (int) bin_op_type;
}

bool should_reorder(ast_binop_type_t op, ast_node_t* right) {
    if( right->type == AST_BINOP )
        return get_precedence(op) >= get_precedence(right->u.n_binop.type);
    return false;
}

ast_node_t* get_left(ast_node_t* node) {
    if( node->type == AST_BINOP )
        return node->u.n_binop.left;
    return node;
}

ast_node_t* get_right(ast_node_t* node) {
    if( node->type == AST_BINOP )
        return node->u.n_binop.right;
    return node;
}

pa_result_t pa_try_parse_binary_operation(pa_result_t lhs, parser_t* parser, token_type_t tt, ast_binop_type_t op) {
    //token_t token = pa_current_token(parser);
    if( pa_advance_if(parser, tt) ) {
        pa_result_t rhs = pa_parse_expression(parser);
        if( par_is_error(rhs) )
            return rhs;
        assert( par_is_nothing(rhs) == false );
        arena_t* a = parser->arena;
        ast_node_t* left = par_extract_node(lhs);
        ast_node_t* right = par_extract_node(rhs);
        // handle operator precedence
        if( should_reorder(op, right) && rhs.group_expression == false ) {
            /* (A $ (B # C)) -> ((A $ B) # C)) =
                ($ 
                 LHS: A
                 RHS: (#
                        LHS: B
                        RHS: C))
                -> 
                (# 
                 LHS: ($
                        LHS: A
                        RHS: B)
                 RHS: C) */
            ast_binop_type_t outer_op = right->u.n_binop.type;
            ast_binop_type_t inner_op = op;
            return par_node(ast_binop(a,
                outer_op, 
                ast_binop(a,
                    inner_op,
                    left,
                    get_left(right)),
                get_right(right)), NULL);
        } else {
            return par_node(
                ast_binop(a,
                    op,
                    left,
                    right),
                NULL);
        }
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
    // UNLESS there is an AND or an OR in between.

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

bool is_valid_type_name(srcref_t ref) {
    if( srcref_equals_string(ref, LANG_TYPENAME_ARRAY) )
        return true;
    if( srcref_equals_string(ref, LANG_TYPENAME_BOOL) )
        return true;
    if( srcref_equals_string(ref, LANG_TYPENAME_CHAR) )
        return true;
    if( srcref_equals_string(ref, LANG_TYPENAME_INT) )
        return true;
    if( srcref_equals_string(ref, LANG_TYPENAME_FLOAT) )
        return true;
    if( srcref_equals_string(ref, LANG_TYPENAME_STRING) )
        return true;
    if( srcref_equals_string(ref, LANG_TYPENAME_VOID) )
        return true;
    return false;
}

pa_result_t parse_type_annotation(parser_t* parser, ast_annot_t** annot) {
    token_t name = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_SYMBOL);
    if( par_is_error(result) )
        return result;
    if( is_valid_type_name(name.ref) == false ) {
        return pa_error_invalid_expression(parser, name, "unrecognized type name");
    }
    *annot = ast_annot(parser->arena, name.ref);
    if( pa_advance_if(parser, TT_CMP_LT) ) {
        do {
            ast_annot_t* child = NULL;
            result = parse_type_annotation(parser, &child);
            if( par_is_error(result) )
                return result;
            assert( child != NULL );
            ast_annot_add_child(parser->arena, (*annot), child);
        } while (pa_advance_if(parser, TT_SEPARATOR));
        return pa_consume(parser, TT_CMP_GT);
    }
    return par_nothing();
}

pa_result_t pa_parse_vardecl(parser_t* parser) {
    ast_annot_t* typename;
    pa_result_t result = parse_type_annotation(parser, &typename);
    if( par_is_error(result) )
        return result;

    token_t varname = pa_current_token(parser);
    result = pa_consume(parser, TT_SYMBOL);

    if( par_is_error(result) )
        return result;

    assert( par_is_nothing(result) );
    return par_node(
        ast_tyannot(parser->arena,
            typename,
            ast_varref(parser->arena,
                varname.ref)),
        NULL);
}

// TODO: should probably rething how assignment is parsed.
// type varname | varname should perhaps be valid
// statements by themselves without assignment?
pa_result_t pa_try_parse_assignment(parser_t* parser) {
    if( pa_peek_token(parser, 1).type == TT_ASSIGN ) {              // <- this is a little "iffy"
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
                par_extract_node(rhs)), NULL);

    } else if( is_valid_type_name(pa_current_token(parser).ref) ) { // <- if the first token is a type ... also "iffy"
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
                par_extract_node(rhs)), NULL);
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
    
    return par_node(body_block, NULL);
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

    return par_node(ast_if(parser->arena, condition, if_true, next), NULL);
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

    return par_node(ast_foreach(parser->arena, vardecl, collection, body), NULL);
}

pa_result_t pa_try_parse_body_break(parser_t* parser) {
    srcref_t ref = pa_current_token(parser).ref;
    if( pa_advance_if(parser, TT_KW_BREAK) )
        return par_node(ast_break(parser->arena), &ref);
    return par_nothing();
}

pa_result_t pa_try_parse_body_return(parser_t* parser) {
    srcref_t ref = pa_current_token(parser).ref;
    if( pa_advance_if(parser, TT_KW_RETURN) ) {
        pa_result_t result = par_nothing();
        if( pa_current_token(parser).type != TT_STATEMENT_END )
            result = pa_parse_expression(parser);
        if( par_is_error(result) )
            return result;
        if( par_is_nothing(result) )
            return par_node(ast_return(parser->arena, ast_block(parser->arena)), &ref); // empty block for "nothing"
        else
            return par_node(ast_return(parser->arena, par_extract_node(result)), &ref);
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

pa_result_t pa_parse_arglist(parser_t* parser) {

    pa_result_t result = pa_consume(parser, TT_OPEN_PAREN);
    if( par_is_error(result) )
        return result;

    ast_node_t* argspec = ast_arglist(parser->arena);

    do {
        if( pa_current_token(parser).type == TT_CLOSE_PAREN )
            break;

        pa_result_t vardecl = pa_parse_vardecl(parser);
        if( par_is_node(vardecl) == false ) {
            return vardecl;
        }

        ast_arglist_add(parser->arena, argspec, par_extract_node(vardecl));
        
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_error(result) )
        return result;

    return par_node(argspec, NULL);
}

pa_result_t pa_parse_funimportdecl(parser_t* parser) {

    ast_annot_t* retannot;
    pa_result_t result = parse_type_annotation(parser, &retannot);
    if( par_is_error(result) )
        return result;

    token_t funname = pa_current_token(parser);
    result = pa_consume(parser, TT_SYMBOL);
    if( par_is_error(result) )
        return result;

    result = pa_parse_arglist(parser);
    if( par_is_error(result) )
        return result;
    
    return par_node(
            ast_tyannot(parser->arena,
                retannot,
                ast_funexdecl(parser->arena,
                    funname.ref,
                    par_extract_node(result))),
            &funname.ref);
}

int seek_end_of_type(parser_t* parser, int offs, int n) {
    token_t token = pa_peek_token(parser, offs);
    if ( token.type == TT_CMP_LT && offs > 0 ) {
        return seek_end_of_type(parser, offs+1, n + 1);
    } else if ( token.type == TT_CMP_GT && offs > 0 ) {
        if( (n - 1) == 0 )
            return offs;
        return seek_end_of_type(parser, offs+1, n - 1);
    } else if ( is_valid_type_name(token.ref) ) {
        int next = seek_end_of_type(parser, offs+1, n);
        if( next > offs )
            return next;
        if( n == 0 )
            return offs;
    }
    return -1;
}

pa_result_t pa_try_parse_fundecl(parser_t* parser) {

    int eot = seek_end_of_type(parser, 0, 0);
    if( eot < 0 )
        return par_nothing();

    if( pa_peek_token(parser, eot + 1).type != TT_SYMBOL
     || pa_peek_token(parser, eot + 2).type != TT_OPEN_PAREN )
     return par_nothing();

    ast_annot_t* retannot;
    pa_result_t result = parse_type_annotation(parser, &retannot);
    if( par_is_error(result) )
        return result;

    token_t funname = pa_current_token(parser);
    result = pa_consume(parser, TT_SYMBOL);
    if( par_is_error(result) )
        return result;

    result = pa_parse_arglist(parser);
    if( par_is_error(result) )
        return result;

    ast_node_t* arglist = par_extract_node(result);
    
    result = pa_parse_body(parser);
    if( par_is_error(result) )
        return result;

    ast_node_t* body = par_extract_node(result);

    ast_node_t* fun = ast_fundecl(
        parser->arena,
        funname.ref,
        arglist,
        body);

    if( srcref_equals_string(funname.ref, "main") )
        ast_fundecl_set_exported(fun);

    return par_node(
        ast_tyannot(parser->arena,
            retannot,
            fun),
        &funname.ref);
}

pa_result_t pa_try_parse_preproc_directive(parser_t* parser) {
    token_t token = pa_current_token(parser);
    if( token.type != TT_IMPORT && token.type != TT_EXPORT )
        return par_nothing();

    token_t directive = pa_current_token(parser);

    pa_advance(parser);

    if( token.type == TT_IMPORT ) {

        pa_result_t result = pa_parse_funimportdecl(parser);
        if( par_is_error(result) )
            return result;

        pa_advance_if(parser, TT_STATEMENT_END); // optional end of statement

        return result;

    } else if ( token.type == TT_EXPORT ) {

        pa_result_t result = pa_try_parse_fundecl(parser);
        if( par_is_error(result) )
            return result;

        ast_node_t* tyannot = par_extract_node(result);
        assert(tyannot != NULL && tyannot->type == AST_TYANNOT);
        ast_fundecl_set_exported(tyannot->u.n_tyannot.expr); // set the exported flag
        pa_advance_if(parser, TT_STATEMENT_END); // optional end of statement
        srcref_t ref = ast_extract_srcref(tyannot);
        srcref_combine(ref, token.ref);
        return par_node(tyannot, &ref);
    }
    
    return pa_error_invalid_statement(parser,
        directive,
        "\n  Expected"
        "\n  - import (from host) declaration"
        "\n  - export (to host) declaration");
}

pa_result_t pa_parse_toplevel_statement(parser_t* parser) {
    pa_result_t result = pa_try_parse_fundecl(parser);
    if( par_is_nothing(result) )
        result = pa_try_parse_preproc_directive(parser);
    if( par_is_error(result) || par_is_node(result) )
        return result;
    return pa_error_invalid_statement(parser,
        pa_current_token(parser),
        "\n  Expected top-level statement such as"
        "\n  - function declaration"
        "\n  - import (from host) declaration"
        "\n  - export (to host) declaration");
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

    return par_node(program, NULL);
}
