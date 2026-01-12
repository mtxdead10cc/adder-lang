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
#include <adrcom/ast/co_ast_builder.h>

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
        if( srcref_contains_char(token.ref, '.') ) {
            return par_node(ast_float(parser->arena, value));
        } else {
            return par_node(ast_int(parser->arena, value));
        }
    }
    return par_error_invalid_token_format(parser, token);
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
    return par_error_invalid_token_format(parser, token);
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
        return par_node(ast_variable_reference(parser->arena, token.ref));
    }
    return par_nothing();
}

pa_result_t pa_try_parse_group(parser_t* parser) {
    token_t token = pa_current_token(parser);
    if( token.type != TT_OPEN_PAREN )
        return par_nothing();
    if( pa_advance(parser) == false )
        return par_error_out_of_tokens(parser);
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

    ast_t* args = ast_arglist(parser->arena, 4);
    srcref_t name = func_name.ref;

    if( pa_advance_if(parser, TT_CLOSE_PAREN) ) {
        return par_node(ast_function_call(parser->arena, name, args));
    }
    
    do {
        pa_result_t expr_res = pa_parse_expression(parser);
        if( par_is_node(expr_res) == false ) {
            return expr_res;
        }
        ast_arglist_append(parser->arena, args, par_extract_node(expr_res));
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_result_t result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_nothing(result) == false ) {
        return result;
    } else {
        return par_node(ast_function_call(parser->arena, name, args));
    }
}

pa_result_t pa_try_parse_array_def(parser_t* parser) {

    if( pa_current_token(parser).type != TT_OPEN_SBRACKET ) {
        return par_nothing();
    }

    pa_advance(parser);

    ast_t* array = ast_array(parser->arena, 4);
    
    if( pa_advance_if(parser, TT_CLOSE_SBRACKET) ) {
        return par_node(array);
    }

    do {
        pa_result_t expr_res = pa_parse_expression(parser);
        if( par_is_node(expr_res) == false ) {
            return expr_res;
        }
        ast_array_append(parser->arena, array, par_extract_node(expr_res));
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_result_t result = pa_consume(parser, TT_CLOSE_SBRACKET);

    if( par_is_nothing(result) == false ) {
        return result;
    } else {
        return par_node(array);
    }
}

bool is_left_binop(ast_t* node) {
    if(ast_is_binop(node))
        return false;
    ast_t* left = node->as.items[0];
    assert(left != NULL);
    return ast_tag_is_binop(left->tag);
}

pa_result_t pa_try_parse_unary_operation(parser_t* parser, token_type_t tt, ast_tag_t unary_tag) {

    if( pa_advance_if(parser, tt) ) {

        pa_result_t inner = pa_parse_expression(parser);
        if( par_is_error(inner) )
            return inner;

        assert( par_is_nothing(inner) == false );

        // find the innermost left expression
        // and negate that
        ast_t* inner_exp = par_extract_node(inner);

        // grouped expressions should be wrapped
        // so this recursion does not trigger 
        // on for example "-(a + b)".
        if( ast_tag_is_binop(inner_exp->tag) && inner.group_expression == false ) {
            
            ast_t* left_leaf = inner_exp;
            
            while( is_left_binop(left_leaf) ) {
                left_leaf = left_leaf->as.items[0];
            }

            inner_exp->as.items[0] = ast_unary_operation(
                parser->arena,
                unary_tag,
                left_leaf);

            return par_node(inner_exp);
        }

        return par_node(
            ast_unary_operation(
                parser->arena,
                unary_tag,
                inner_exp));
    }

    return par_nothing();
}

int get_precedence(ast_tag_t bin_op_type) {
    if(ast_tag_is_binop(bin_op_type) == false)
        return -1;
    return 100 - (int) bin_op_type;
}

bool should_reorder(ast_tag_t op, ast_t* right) {
    return get_precedence(op) >= get_precedence(right->tag);
}

pa_result_t pa_try_parse_binary_operation(pa_result_t lhs, parser_t* parser, token_type_t tt, ast_tag_t op) {
    
    if( pa_advance_if(parser, tt) ) {
        pa_result_t rhs = pa_parse_expression(parser);
        if( par_is_error(rhs) )
            return rhs;

        assert( par_is_nothing(rhs) == false );

        ast_t* left = par_extract_node(lhs);
        ast_t* right = par_extract_node(rhs);

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
            ast_tag_t outer_op = right->tag;
            ast_tag_t inner_op = op;
            return par_node(
                ast_binary_operation(parser->arena, outer_op,
                    ast_binary_operation(parser->arena, inner_op,
                        left, right->as.items[0]),
                    right->as.items[1]));
        } else {
            return par_node(ast_binary_operation(parser->arena, op, left, right));
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
        result = pa_try_parse_unary_operation(parser, TT_BINOP_MINUS, AST_UNA_NEG);

    if( par_is_nothing(result) ) 
        result = pa_try_parse_unary_operation(parser, TT_UNOP_NOT, AST_UNA_NOT);

    if( par_is_error(result) ) {
        return result;
    }

    if( par_is_node(result) == false ) {
        return par_error_invalid_expression(parser, pa_current_token(parser), NULL);
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

pa_result_t parse_type_descriptor(parser_t* parser) {

    token_t name = pa_current_token(parser);
    
    pa_result_t result = pa_consume(parser, TT_SYMBOL);
    if( par_is_error(result) )
        return result;

    if( is_valid_type_name(name.ref) == false ) {
        return par_error_invalid_expression(parser, name,
            "unrecognized type name");
    }

    ast_t* args = ast_arglist(parser->arena, 1);

    if( pa_advance_if(parser, TT_CMP_LT) ) {

        do {
            result = parse_type_descriptor(parser);
            if( par_is_error(result) )
                return result;

            ast_t* child = par_extract_node(result);
            ast_arglist_append(parser->arena, args, child);

        } while (pa_advance_if(parser, TT_SEPARATOR));

        return pa_consume(parser, TT_CMP_GT);
    }
    return par_node(ast_type_descriptor(parser->arena, name.ref, args));
}

pa_result_t pa_parse_vardecl(parser_t* parser) {
    pa_result_t result = parse_type_descriptor(parser);
    if( par_is_error(result) )
        return result;

    ast_t* typedescr = par_extract_node(result);

    token_t varname = pa_current_token(parser);
    result = pa_consume(parser, TT_SYMBOL);

    if( par_is_error(result) )
        return result;

    assert( par_is_nothing(result) );

    return par_node(ast_variable_declaration(parser->arena, typedescr, varname.ref));
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
            return par_error_out_of_tokens(parser);
        pa_result_t rhs = pa_parse_expression(parser); // rhs expr
        if( par_is_error(rhs) )
            return rhs;
        assert( par_is_nothing(rhs) == false );

        return par_node(ast_assignment(parser->arena,
            ast_variable_reference(parser->arena, varname.ref),
            par_extract_node(rhs)));

    } else if( is_valid_type_name(pa_current_token(parser).ref) ) { // <- if the first token is a type ... also "iffy"

        pa_result_t decl = pa_parse_vardecl(parser);
        if( par_is_error(decl) )
            return decl;
        assert(par_is_nothing(decl) == false);
        pa_result_t assignres = pa_consume(parser, TT_ASSIGN); // skip over '='
        if( par_is_error(assignres) )
            return assignres;
        pa_result_t rhs = pa_parse_expression(parser); // rhs expr
        if( par_is_error(rhs) )
            return rhs;
        assert(par_is_nothing(rhs) == false);

        return par_node(ast_assignment(parser->arena,
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

    ast_t* block = ast_block(parser->arena, 4);

    do {
        if( pa_current_token(parser).type == TT_CLOSE_CURLY )
            break;

        pa_result_t stmt_res = pa_parse_statement(parser);
        if( par_is_node(stmt_res) == false ) {
            return stmt_res;
        }

        ast_block_append(parser->arena,
            block, par_extract_node(stmt_res));
        
    } while( pa_is_at_end(parser) == false );

    result = pa_consume(parser, TT_CLOSE_CURLY);

    if( par_is_error(result) )
        return result;
    
    return par_node(block);
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
    ast_t* condition = par_extract_node(result);

    result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_error(result) )
        return result;
    assert( par_is_nothing(result) );

    result = pa_parse_body(parser);
    if( par_is_error(result) )
        return result;
    assert( par_is_node(result) );

    ast_t* if_true = par_extract_node(result);

    ast_t* next = NULL;

    if( pa_advance_if(parser, TT_KW_ELSE) ) {
        result = pa_try_parse_if_chain(parser); // else if   
        if( par_is_nothing(result) ) {            
            result = pa_parse_body(parser);     // or last else
        }
        if( par_is_error(result) )
            return result;
        next = par_extract_node(result);
    } else {
        next = ast_block(parser->arena, 0); // empty / nothing
    }

    return par_node(
        ast_if_chain(
            parser->arena,
            condition,
            if_true,
            next));
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

    ast_t* vardecl = par_extract_node(result);

    result = pa_consume(parser, TT_KW_IN);
    if( par_is_error(result) )
        return result;
    assert( par_is_nothing(result) );

    result = pa_parse_expression(parser);
    if( par_is_error(result) )
        return result;
    assert( par_is_node(result) );
    ast_t* collection = par_extract_node(result);

    result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_error(result) )
        return result;
    assert( par_is_nothing(result) );

    // todo: verify collection

    result = pa_parse_body(parser);
    if( par_is_error(result) )
        return result;
    assert( par_is_node(result) );

    ast_t* body = par_extract_node(result);

    return par_node(
        ast_foreach(
            parser->arena,
            vardecl,
            collection,
            body));
}

pa_result_t pa_try_parse_body_return(parser_t* parser) {
    if( pa_advance_if(parser, TT_KW_RETURN) ) {
        pa_result_t result = par_nothing();
        if( pa_current_token(parser).type != TT_STATEMENT_END )
            result = pa_parse_expression(parser);
        if( par_is_error(result) )
            return result;
        ast_t* return_wrapper;
        if( par_is_nothing(result) )
            return_wrapper = ast_return(parser->arena, ast_block(parser->arena, 0)); // empty block for "nothing"
        else
            return_wrapper = ast_return(parser->arena, par_extract_node(result));
        return par_node(return_wrapper);
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

    if( par_is_error(result) )
        return result;

    if( par_is_node(result) == false ) {
        return par_error_invalid_statement(parser, pa_current_token(parser), NULL);
    }

    // optional end-of-statement (for now)
    pa_advance_if(parser, TT_STATEMENT_END);

    return result;
}

pa_result_t pa_parse_arglist(parser_t* parser, ast_t* arglist) {

    pa_result_t result = pa_consume(parser, TT_OPEN_PAREN);
    if( par_is_error(result) )
        return result;

    do {
        if( pa_current_token(parser).type == TT_CLOSE_PAREN )
            break;

        pa_result_t vardecl = pa_parse_vardecl(parser);
        if( par_is_node(vardecl) == false ) {
            return vardecl;
        }

        ast_arglist_append(parser->arena,
            arglist,
            par_extract_node(vardecl));
        
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_error(result) )
        return result;

    return par_nothing();
}

pa_result_t pa_parse_funimportdecl(parser_t* parser) {

    pa_result_t result = parse_type_descriptor(parser);
    if( par_is_error(result) )
        return result;

    ast_t* rettypedescr = par_extract_node(result);

    token_t funname = pa_current_token(parser);
    result = pa_consume(parser, TT_SYMBOL);
    if( par_is_error(result) )
        return result;

    ast_t* arglist = ast_arglist(parser->arena, 4);
    result = pa_parse_arglist(parser, arglist);
    if( par_is_error(result) )
        return result;

    ast_t* fundecl = ast_function_declaration(
        parser->arena,
        rettypedescr,
        funname.ref,
        arglist,
        AST_FLAG_IMPORT);

    return par_node(fundecl);
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

pa_result_t pa_try_parse_fundef(parser_t* parser) {

    int eot = seek_end_of_type(parser, 0, 0);
    if( eot < 0 )
        return par_nothing();

    if( pa_peek_token(parser, eot + 1).type != TT_SYMBOL
     || pa_peek_token(parser, eot + 2).type != TT_OPEN_PAREN )
     return par_nothing();

    pa_result_t result = parse_type_descriptor(parser);
    if( par_is_error(result) )
        return result;

    ast_t* rettypedescr = par_extract_node(result);

    token_t funname = pa_current_token(parser);
    result = pa_consume(parser, TT_SYMBOL);
    if( par_is_error(result) )
        return result;

    ast_t* arglist = ast_arglist(parser->arena, 4);
    result = pa_parse_arglist(parser, arglist);
    if( par_is_error(result) )
        return result;
    
    result = pa_parse_body(parser);
    if( par_is_error(result) )
        return result;

    ast_t* body = par_extract_node(result);

    ast_t* fundecl = ast_function_declaration(parser->arena,
        rettypedescr,
        funname.ref,
        arglist,
        AST_FLAG_NOFLAG);

    if( srcref_equals_string(funname.ref, "main") ) {
        assert(fundecl->as.items[2]->tag == AST_FLAGS);
        fundecl->as.items[2]->as.flags = AST_FLAG_EXPORT;
    }

    ast_t* fundef = ast_function_definition(parser->arena, fundecl, body); 

    return par_node(fundef);
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

        pa_result_t result = pa_try_parse_fundef(parser);
        if( par_is_error(result) )
            return result;

        ast_t* fundef = par_extract_node(result);

        assert(fundef->as.items[0]->tag == AST_TYDESCR);
        assert(fundef->as.items[0]->as.items[1]->tag == AST_FUNDECL);
        assert(fundef->as.items[0]->as.items[1]->as.items[2]->tag == AST_FLAGS);

        fundef->as.items[0]->as.items[1]->as.items[2]->as.flags |= AST_FLAG_EXPORT;

        pa_advance_if(parser, TT_STATEMENT_END);        // optional end of statement

        return par_node(fundef);
    }
    
    return par_error_invalid_statement(parser,
        directive,
        "\n  Expected"
        "\n  - import (from host) declaration"
        "\n  - export (to host) declaration");
}

pa_result_t pa_parse_toplevel_statement(parser_t* parser) {
    pa_result_t result = pa_try_parse_fundef(parser);
    if( par_is_nothing(result) )
        result = pa_try_parse_preproc_directive(parser);
    if( par_is_error(result) || par_is_node(result) )
        return result;
    return par_error_invalid_statement(parser,
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

    ast_t* body = ast_block(parser->arena, 32);

    do {
        result = pa_parse_toplevel_statement(parser);
        if( par_is_node(result) ) {
            ast_block_append(parser->arena,
                body, par_extract_node(result));
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

    return par_node(body);
}