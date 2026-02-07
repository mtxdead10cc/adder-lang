#include "adrcom/parser/co_parser.h"
#include "adrcom/parser/co_parser_types.h"
#include "adrcom/parser/co_tokenizer.h"
#include "adrcom/parser/co_srcmap.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>

#include <shared/sh_types.h>
#include <shared/sh_utils.h>

#include <adrcom/shared/co_trace.h>
#include <adrcom/shared/co_types.h>

#include <adrcom/ast/co_ast.h>

#include <shared/sh_diag.h>

ast_t* pa_parse_expression(parser_t* parser);
ast_t* pa_parse_statement(parser_t* parser);


typedef struct pa_diag_list_t {
    ast_tag_t    error_node_tag;
    parser_t*    parser;
    ast_diags_t* diagnostics;
} pa_diag_list_t;

pa_diag_list_t pa_diag_list(parser_t* parser, ast_tag_t error_node_tag) {
    return (pa_diag_list_t) {
        .error_node_tag = error_node_tag,
        .parser = parser,
        .diagnostics = NULL
    };
}

ast_t* pa_diag_list_to_node(pa_diag_list_t* list) {
    if(list->diagnostics == NULL)
        return NULL;
    assert(list->parser != NULL);
    ast_t* n = ast_leaf(list->parser->arena, list->error_node_tag);
    assert(n!=NULL);
    n->diagnostics = list->diagnostics;
    for(int i = 0; i < n->diagnostics->size; i++) {
        diag_t* diag = list->diagnostics->list[i];
        srcref_t ref = diag_collect_srcref(diag);
        ast_extend_source_range(n, ref);
    }
    return n;
}

void pa_diag_list_append(pa_diag_list_t* list, diag_t* diag) {

    if(diag == NULL)
        return;

    int initsize = list->diagnostics != NULL
        ? list->diagnostics->size : 0;

    list->diagnostics = ast_diags_add_diag(list->parser->arena,
        list->diagnostics, diag);

    if(list->diagnostics == NULL) {
        sh_log_error("pa_diag_list_append: out of memory");
    } else if (list->diagnostics->size != initsize + 1) {
        sh_log_error("pa_diag_list_append: failed to append to diagnostics");
    }
}

void pa_diag_list_unexpected_token_type(pa_diag_list_t* list, token_type_t expected, token_t actual) {
    pa_diag_list_append(list, par_error_unexpected_token_type(list->parser, expected, actual));
}

void pa_diag_list_invalid_token_format(pa_diag_list_t* list, token_t token) {
    pa_diag_list_append(list, par_error_invalid_token_format(list->parser, token));
}

void pa_diag_list_invalid_expression(pa_diag_list_t* list, token_t token, char* expected_str) {
    pa_diag_list_append(list, par_error_invalid_expression(list->parser, token, expected_str));
}

void pa_diag_list_invalid_statement(pa_diag_list_t* list, token_t token, char* expected_str) {
    pa_diag_list_append(list, par_error_invalid_statement(list->parser, token, expected_str));
}

void pa_diag_list_out_of_tokens(pa_diag_list_t* list) {
    pa_diag_list_append(list, par_error_out_of_tokens(list->parser));
}

/*void pa_diag_list_consume_until(pa_diag_list_t* list, token_type_t stop) {

    token_t actual = pa_current_token(list->parser);
    srcref_t aggregate = {0};
    
    while (pa_advance(list->parser)) {
        if( stop == actual.type )
            break;
        aggregate = srcref_combine(aggregate, actual.ref);
        actual = pa_current_token(list->parser);
    }

    if(srcref_is_valid(aggregate)) {
        diag_t* diag = diag_error(
            diag_phrase(_UNEXPECTED, _TOKEN, _SEQUENCE),
            diag_str(" at "), diag_refloc(aggregate),
            diag_str("\n  expected "), diag_str(token_get_type_name(stop) + 3),
            diag_str("\n  got ("), diag_refstr(aggregate), diag_str(")"));
        pa_diag_list_append(list, diag);
    }
}*/

void pa_diag_list_consume(pa_diag_list_t* list, token_type_t expected) {

    token_t actual = pa_current_token(list->parser);

    pa_advance(list->parser);

    if(actual.type == expected)
        return;

    pa_diag_list_append(list,
        par_error_unexpected_token_type(list->parser,
            expected, actual));
}

ast_t* pa_diag_list_transfer(pa_diag_list_t* list, ast_t* node) {
    if(list->diagnostics == NULL)
        return node;
    for(int i = 0; i < list->diagnostics->size; i++) {
        node->diagnostics = ast_diags_add_diag(list->parser->arena,
            node->diagnostics,
            list->diagnostics->list[i]);
    }
    return node;
}

ast_t* pa_parse_number(parser_t* parser) {

    token_t numeric_token = pa_current_token(parser);

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_NUMBER);

    float value = 0.0f;
    ast_t* node = NULL;

    if( srcref_as_float(numeric_token.ref, &value) ) {
        if( srcref_contains_char(numeric_token.ref, '.') ) {
            node = ast_float(parser->arena, value);
        } else {
            node = ast_int(parser->arena, value);
        }
        ast_extend_source_range(node, numeric_token.ref);
        return node;
    }

    pa_diag_list_invalid_token_format(&dial, numeric_token);
    return pa_diag_list_to_node(&dial);
}

ast_t* pa_parse_boolean(parser_t* parser) {
    token_t token = pa_current_token(parser);

    pa_diag_list_t dial = pa_diag_list(parser, AST_BOOL);
    pa_diag_list_consume(&dial, TT_BOOLEAN);

    bool value = false;
    if( srcref_as_bool(token.ref, &value) ) {
        ast_t* node = ast_bool(parser->arena, value);
        ast_extend_source_range(node, token.ref);
        return node;
    }

    pa_diag_list_invalid_token_format(&dial, token);
    return pa_diag_list_to_node(&dial);
}

ast_t* pa_parse_string(parser_t* parser) {
    token_t token = pa_current_token(parser);

    pa_diag_list_t dial = pa_diag_list(parser, AST_STRING);
    pa_diag_list_consume(&dial, TT_STRING);

    ast_t* node = ast_string(parser->arena, token.ref);
    return pa_diag_list_transfer(&dial, node);
}

ast_t* pa_try_parse_value(parser_t* parser) {
    token_t token = pa_current_token(parser);
    if( token.type == TT_BOOLEAN ) {
        return pa_parse_boolean(parser);
    } else if ( token.type == TT_NUMBER ) {
        return pa_parse_number(parser);
    } else if ( token.type == TT_STRING ) {
        return pa_parse_string(parser);
    }
    return NULL;
}

ast_t* pa_try_parse_var_name(parser_t* parser) {
    // SOURCE RANGE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    token_t token = pa_current_token(parser);
    if( pa_advance_if(parser, TT_SYMBOL) ) {
        return ast_variable_reference(parser->arena, token.ref);
    }
    return NULL;
}

ast_t* pa_try_parse_group(parser_t* parser) {

    token_t token = pa_current_token(parser);

    if( token.type != TT_OPEN_PAREN )
        return NULL;

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_OPEN_PAREN);

    // todo: error message if empty paren
    ast_t* inner = pa_parse_expression(parser);

    pa_diag_list_consume(&dial, TT_CLOSE_PAREN);

    if( ast_is_binop(inner) )
        inner->as.items[AST_BINOP_GREXP]->as._bool = true;

    // SOURCE RANGE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    return pa_diag_list_transfer(&dial, inner);
}

ast_t* pa_try_parse_func_call(parser_t* parser) {

    token_t func_name = pa_current_token(parser);
    token_t open_paren = pa_peek_token(parser, 1);
    
    if( func_name.type != TT_SYMBOL || open_paren.type != TT_OPEN_PAREN ) {
        return NULL;
    }

    pa_advance(parser);
    pa_advance(parser);

    ast_t* args = ast_arglist(parser->arena);
    srcref_t name = func_name.ref;

    if( pa_advance_if(parser, TT_CLOSE_PAREN) ) {
        ast_t* funcall = ast_function_call(parser->arena, name, args);
        ast_extend_source_range(funcall, func_name.ref);
        ast_extend_source_range(funcall, pa_peek_token(parser, -1).ref);
        return funcall;
    }
    
    do {
        ast_t* expr_res = pa_parse_expression(parser);
        ast_arglist_append(parser->arena, args, expr_res);
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_CLOSE_PAREN);
    ast_t* errarg = pa_diag_list_to_node(&dial);
    if( errarg != NULL )
        ast_arglist_append(parser->arena, args, errarg);

    ast_t* funcall_w_args = ast_function_call(parser->arena, name, args);
    ast_extend_source_range(funcall_w_args, func_name.ref);
    ast_extend_source_range(funcall_w_args, pa_peek_token(parser, -1).ref);

    return funcall_w_args;
}

ast_t* pa_try_parse_array_def(parser_t* parser) {

    token_t initial_token = pa_current_token(parser);
    if( initial_token.type != TT_OPEN_SBRACKET )
        return NULL;

    pa_advance(parser);

    ast_t* array = ast_array(parser->arena);
    
    if( pa_advance_if(parser, TT_CLOSE_SBRACKET) ) {
        ast_extend_source_range(array, initial_token.ref);
        ast_extend_source_range(array, pa_peek_token(parser, -1).ref);
        return array;
    }

    do {
        ast_t* expr_res = pa_parse_expression(parser);
        ast_array_append(parser->arena, array, expr_res);
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_CLOSE_SBRACKET);
    ast_t* errexpr = pa_diag_list_to_node(&dial);
    if( errexpr != NULL )
        ast_array_append(parser->arena, array, errexpr);

    ast_extend_source_range(array, initial_token.ref);
    ast_extend_source_range(array, pa_peek_token(parser, -1).ref);

    return array;
}

bool is_left_binop(ast_t* node) {
    if(ast_is_binop(node))
        return false;
    ast_t* left = node->as.items[0];
    assert(left != NULL);
    return ast_tag_is_binop(left->tag);
}

ast_t* pa_try_parse_unary_operation(parser_t* parser, token_type_t tt, ast_tag_t unary_tag) {

    // SOURCE RANGE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    if( pa_advance_if(parser, tt) ) {

        ast_t* inner = pa_parse_expression(parser);
        if( par_is_error(inner) )
            return inner;

        assert( par_is_nothing(inner) == false );

        // find the innermost left expression
        // and negate that
        ast_t* inner_exp = inner;

        // grouped expressions should be wrapped
        // so this recursion does not trigger 
        // on for example "-(a + b)".
        if( ast_tag_is_binop(inner_exp->tag) && ast_is_group_expr(inner_exp) == false ) {
            
            ast_t* leftbin = inner_exp;
            
            while( is_left_binop(leftbin) ) {
                leftbin = leftbin->as.items[AST_BINOP_LEFT];
            }

            assert(ast_is_binop(inner_exp));

            leftbin->as.items[AST_BINOP_LEFT] = ast_unary_operation(
                parser->arena,
                unary_tag,
                leftbin->as.items[AST_BINOP_LEFT]);

            return inner_exp;
        }

        return ast_unary_operation(
            parser->arena,
            unary_tag,
            inner_exp);
    }

    return NULL;
}

int get_precedence(ast_tag_t bin_op_type) {
    if(ast_tag_is_binop(bin_op_type) == false)
        return -1;
    return 100 - (int) bin_op_type;
}

bool should_reorder(ast_tag_t op, ast_t* right) {
    if(ast_is_binop(right) == false)
        return false;
    // the comparison '<=' is needed in order for
    // a - -b * c to evaluate to a + b * c 
    return get_precedence(op) >= get_precedence(right->tag);
}

ast_t* pa_try_parse_binary_operation(ast_t* lhs, parser_t* parser, token_type_t tt, ast_tag_t op) {

    assert(ast_tag_is_binop(op));

    // SOURCE RANGE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    if( pa_advance_if(parser, tt) ) {

        ast_t* right = pa_parse_expression(parser);
        ast_t* left = lhs;

        // handle operator precedence
        if( should_reorder(op, right) && ast_is_group_expr(right) == false ) {

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

            ast_tag_t outer_op = right->tag;
            ast_tag_t inner_op = op;

            return ast_binary_operation(parser->arena,
                outer_op,
                ast_binary_operation(parser->arena,
                    inner_op,
                    left,
                    right->as.items[AST_BINOP_LEFT], false),
                right->as.items[AST_BINOP_RIGHT], false);
        } else {
            return ast_binary_operation(parser->arena, op, left, right, false);
        }
    }
    return NULL;
}


ast_t* pa_parse_expression(parser_t* parser) {

    // parsing standard expressions

    ast_t* result = pa_try_parse_group(parser);

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
        result = pa_try_parse_unary_operation(parser, TT_BINOP_MINUS, AST_UNA_NEG);

    if( par_is_nothing(result) ) 
        result = pa_try_parse_unary_operation(parser, TT_UNOP_NOT, AST_UNA_NOT);

    /*if( par_is_nothing(result) == false ) {
        pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
        pa_diag_list_invalid_expression(&dial, pa_current_token(parser), NULL);
        return pa_diag_list_to_node(&dial);
    }*/

    // parsing binary operation expressions

    ast_t* bin_op_result = pa_try_parse_binary_operation(result, parser, TT_BINOP_AND, AST_BIN_AND);

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

ast_t* parse_type_descriptor(parser_t* parser) {

    token_t name = pa_current_token(parser);

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_SYMBOL);

    if( is_valid_type_name(name.ref) == false ) {
        pa_diag_list_invalid_expression(&dial,
            name, "unrecognized type name");
        if(pa_current_token(parser).type != TT_CMP_LT)
            return pa_diag_list_to_node(&dial);
    }

    ast_t* args = ast_arglist(parser->arena);

    if( pa_advance_if(parser, TT_CMP_LT) ) {

        do {
            ast_t* child = parse_type_descriptor(parser);
            ast_arglist_append(parser->arena, args, child);
        } while (pa_advance_if(parser, TT_SEPARATOR));

        pa_diag_list_consume(&dial, TT_CMP_GT);
        ast_t* errchild = pa_diag_list_to_node(&dial);
        if( errchild != NULL )
            ast_arglist_append(parser->arena, args, errchild);
    }

    ast_t* descr = ast_type_descriptor(parser->arena, name.ref, args);
    ast_extend_source_range(descr, name.ref);
    ast_extend_source_range(descr, pa_peek_token(parser, -1).ref);
    return descr;
}

ast_t* pa_parse_vardecl(parser_t* parser) {

    ast_t* typedescr = parse_type_descriptor(parser);

    token_t varname = pa_current_token(parser);

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_SYMBOL);

    ast_t* varref = ast_variable_reference(parser->arena, varname.ref);
    varref = pa_diag_list_transfer(&dial, varref);

    ast_t* vardecl = ast_variable_declaration(parser->arena, typedescr, varref);
    ast_extend_source_range_with_node(vardecl, typedescr);
    ast_extend_source_range(vardecl, varname.ref);
    return vardecl;
}

// TODO: should probably rething how assignment is parsed.
// type varname | varname should perhaps be valid
// statements by themselves without assignment?
ast_t* pa_try_parse_assignment(parser_t* parser) {

    if( pa_peek_token(parser, 1).type == TT_ASSIGN ) {              // <- this is a little "iffy"

        token_t varname = pa_current_token(parser);

        pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
        pa_diag_list_consume(&dial, TT_SYMBOL);
        
        if( pa_advance(parser) == false ) { // skip over '='
            pa_diag_list_out_of_tokens(&dial);
            return pa_diag_list_to_node(&dial);
        }

        ast_t* rhs = pa_parse_expression(parser); // rhs expr
        assert( par_is_nothing(rhs) == false );
        ast_t* varref = ast_variable_reference(parser->arena, varname.ref);
        ast_t* assign = ast_assignment(parser->arena, varref, rhs);

        ast_extend_source_range_with_node(assign, varref);
        ast_extend_source_range_with_node(assign, rhs);
        return pa_diag_list_transfer(&dial, assign);

    } else if( is_valid_type_name(pa_current_token(parser).ref) ) { // <- if the first token is a type ... also "iffy"

        ast_t* decl = pa_parse_vardecl(parser);
        assert(par_is_nothing(decl) == false);
        pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
        pa_diag_list_consume(&dial, TT_ASSIGN); // skip over '='

        ast_t* rhs = pa_parse_expression(parser); // rhs expr
        assert(par_is_nothing(rhs) == false);

        ast_t* assign = ast_assignment(parser->arena, decl, rhs);
        ast_extend_source_range_with_node(assign, decl);
        ast_extend_source_range_with_node(assign, rhs);
        return pa_diag_list_transfer(&dial, assign);

    } else {
        return NULL;
    }
}

ast_t* pa_parse_body(parser_t* parser) {

    token_t initial_token = pa_current_token(parser);

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_OPEN_CURLY);

    ast_t* block = ast_block(parser->arena);

    do {

        if( pa_current_token(parser).type == TT_CLOSE_CURLY )
            break;

        ast_t* stmt_res = pa_parse_statement(parser);

        if( par_is_nothing(stmt_res) ) {
            pa_diag_list_t dl = pa_diag_list(parser, AST_UNKNOWN);
            pa_diag_list_invalid_statement(&dl, pa_current_token(parser), NULL);
            stmt_res = pa_diag_list_to_node(&dl);
            pa_advance(parser);
            pa_advance_if(parser, TT_STATEMENT_END);
        }

        ast_block_append(parser->arena,
            block,
            stmt_res);
        
    } while( pa_is_at_end(parser) == false );

    token_t final_token = pa_current_token(parser);
    pa_diag_list_consume(&dial, TT_OPEN_CURLY);

    ast_extend_source_range(block, initial_token.ref);
    ast_extend_source_range(block, final_token.ref);

    return pa_diag_list_transfer(&dial, block);
}

ast_t* pa_try_parse_if_chain(parser_t* parser) {

    token_t initial_token = pa_current_token(parser);

    if( pa_advance_if(parser, TT_KW_IF) == false )
        return NULL;

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_OPEN_PAREN);

    ast_t* condition = pa_parse_expression(parser);
    assert( par_is_nothing(condition) == false );

    pa_diag_list_consume(&dial, TT_CLOSE_PAREN);

    ast_t* if_true = pa_parse_body(parser);
    ast_t* next = NULL;

    token_t final_token = pa_current_token(parser);

    if( pa_advance_if(parser, TT_KW_ELSE) ) {
        ast_t* result = pa_try_parse_if_chain(parser); // else if   
        if( par_is_nothing(result) ) {            
            result = pa_parse_body(parser);     // or last else
        }
        next = result;
    } else {
        next = ast_block(parser->arena); // empty / nothing
    }

    ast_t* if_chain = ast_if_chain(
        parser->arena,
        condition,
        if_true,
        next);

    ast_extend_source_range(if_chain, initial_token.ref);
    ast_extend_source_range(if_chain, final_token.ref);
    return pa_diag_list_transfer(&dial, if_chain);
}

ast_t* pa_try_parse_for_stmt(parser_t* parser) {
    
    if( pa_advance_if(parser, TT_KW_FOR) == false )
        return NULL;

    token_t initial_token = pa_current_token(parser);

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_OPEN_PAREN);

    ast_t* vardecl = pa_parse_vardecl(parser);

    assert( par_is_nothing(vardecl) == false );

    pa_diag_list_consume(&dial, TT_KW_IN);
 
    ast_t* collection = pa_parse_expression(parser);

    // todo: verify collection

    pa_diag_list_consume(&dial, TT_CLOSE_PAREN);

    ast_t* body = pa_parse_body(parser);

    ast_t* foreach = ast_foreach(
        parser->arena,
        vardecl,
        collection,
        body);

    ast_extend_source_range(foreach, initial_token.ref);
    ast_extend_source_range_with_node(foreach, vardecl);
    ast_extend_source_range_with_node(foreach, collection);
    ast_extend_source_range_with_node(foreach, body);

    return foreach;
}

ast_t* pa_try_parse_body_return(parser_t* parser) {

    token_t initial_token = pa_current_token(parser);

    if( pa_advance_if(parser, TT_KW_RETURN) ) {

        ast_t* result = NULL;

        if( pa_current_token(parser).type != TT_STATEMENT_END )
            result = pa_parse_expression(parser);

        ast_t* return_wrapper = NULL;

        if( par_is_nothing(result) )
            return_wrapper = ast_return(parser->arena, ast_block(parser->arena)); // empty block for "nothing"
        else
            return_wrapper = ast_return(parser->arena, result);

        ast_extend_source_range(return_wrapper, initial_token.ref);
        ast_extend_source_range(return_wrapper, pa_peek_token(parser, 0).ref);

        return return_wrapper;
    }

    return NULL;
}

ast_t* pa_parse_statement(parser_t* parser) {

    ast_t* result = pa_try_parse_assignment(parser);

    // todo: if-else? elifs?

    if( par_is_nothing(result) )
        result = pa_try_parse_if_chain(parser);

    if( par_is_nothing(result) )
        result = pa_try_parse_for_stmt(parser);

    if( par_is_nothing(result) )
        result = pa_try_parse_func_call(parser);

    if( par_is_nothing(result) )
        result = pa_try_parse_body_return(parser);

    // optional end-of-statement (for now)
    pa_advance_if(parser, TT_STATEMENT_END);

    return result;
}

void pa_parse_arglist(parser_t* parser, ast_t* arglist) {

    ast_extend_source_range(arglist, pa_current_token(parser).ref);

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_OPEN_PAREN);

    do {

        if( pa_current_token(parser).type == TT_CLOSE_PAREN )
            break;

        ast_t* vardecl = pa_parse_vardecl(parser);

        ast_arglist_append(parser->arena,
            arglist,
            vardecl);
        
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_diag_list_consume(&dial, TT_CLOSE_PAREN);
    ast_extend_source_range(arglist, pa_peek_token(parser, -1).ref);
    pa_diag_list_transfer(&dial, arglist);
}

ast_t* pa_parse_funimportdecl(parser_t* parser, uint32_t flags) {

    ast_t* rettypedescr = parse_type_descriptor(parser);

    token_t funname = pa_current_token(parser);

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_SYMBOL);

    ast_t* arglist = ast_arglist(parser->arena);
    pa_parse_arglist(parser, arglist);

    ast_t* funsign = ast_function_signature(
        parser->arena,
        rettypedescr,
        funname.ref,
        arglist,
        flags);

    ast_extend_source_range_with_node(funsign, rettypedescr);
    ast_extend_source_range(funsign, funname.ref);
    ast_extend_source_range_with_node(funsign, arglist);

    return funsign;
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

ast_t* pa_try_parse_fundef(parser_t* parser, uint32_t flags) {

    int eot = seek_end_of_type(parser, 0, 0);
    if( eot < 0 )
        return NULL;

    if( pa_peek_token(parser, eot + 1).type != TT_SYMBOL
     || pa_peek_token(parser, eot + 2).type != TT_OPEN_PAREN )
     return NULL;

    ast_t* rettypedescr = parse_type_descriptor(parser);

    token_t funname = pa_current_token(parser);

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_SYMBOL);

    ast_t* arglist = ast_arglist(parser->arena);
    pa_parse_arglist(parser, arglist);
    
    ast_t* body = pa_parse_body(parser);

    flags = srcref_equals_string(funname.ref, "main")
        ? (flags | AST_FUNSIGN_FFI_FLAG_EXPORT)
        :  flags;
    
    ast_t* funsign = ast_function_signature(parser->arena,
        rettypedescr,
        funname.ref,
        arglist,
        flags);

    funsign = pa_diag_list_transfer(&dial, funsign);

    ast_t* fundef = ast_function_definition(parser->arena, funsign, body);

    ast_extend_source_range_with_node(fundef, rettypedescr);
    ast_extend_source_range_with_node(fundef, funsign);
    ast_extend_source_range_with_node(fundef, body);

    return fundef;
}

ast_t* pa_try_parse_preproc_directive(parser_t* parser) {

    token_t token = pa_current_token(parser);

    if( token.type != TT_IMPORT && token.type != TT_EXPORT )
        return NULL;

    token_t directive = pa_current_token(parser);

    pa_advance(parser);

    if( token.type == TT_IMPORT ) {

        ast_t* fundef = pa_parse_funimportdecl(parser,
            AST_FUNSIGN_FFI_FLAG_IMPORT);

        ast_extend_source_range(fundef, token.ref);

        pa_advance_if(parser, TT_STATEMENT_END); // optional end of statement

        return fundef;

    } else if ( token.type == TT_EXPORT ) {

        ast_t* fundef = pa_try_parse_fundef(parser,
            AST_FUNSIGN_FFI_FLAG_EXPORT);

        ast_extend_source_range(fundef, token.ref);

        pa_advance_if(parser, TT_STATEMENT_END);        // optional end of statement

        return fundef;
    }

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_SYMBOL);
    
    pa_diag_list_invalid_statement(&dial,
        directive,
        "\n  Expected"
        "\n  - import (from host) declaration"
        "\n  - export (to host) declaration");

    pa_advance(parser);

    return pa_diag_list_to_node(&dial);
}

ast_t* pa_parse_toplevel_statement(parser_t* parser) {

    ast_t* result = pa_try_parse_fundef(parser, 0);

    if( par_is_nothing(result) )
        result = pa_try_parse_preproc_directive(parser);

    return result;
}

ast_t* pa_parse_program(parser_t* parser) {

    pa_diag_list_t dial = pa_diag_list(parser, AST_UNKNOWN);
    pa_diag_list_consume(&dial, TT_INITIAL);

    ast_t* result = NULL;
    ast_t* body = ast_block(parser->arena);

    body = pa_diag_list_transfer(&dial, body);

    do {

        result = pa_parse_toplevel_statement(parser);

        if( result == NULL ) {
            
            dial = pa_diag_list(parser, AST_UNKNOWN);

            pa_diag_list_invalid_statement(&dial,
                pa_current_token(parser),
                "\n  Expected top-level statement such as"
                "\n  - function declaration"
                "\n  - import (from host) declaration"
                "\n  - export (to host) declaration");

            result = pa_diag_list_to_node(&dial);
            pa_advance(parser);
            pa_advance_if(parser, TT_STATEMENT_END);
        }

        ast_block_append(parser->arena, body, result); 

    } while(    !pa_is_at_end(parser)
             && !pa_advance_if(parser, TT_FINAL) );

    return body;
}