# Language

## Syntax

```
fun hello(num arg1, num arg2) -> none {
    num a = add(arg1, arg2);
    if( a <= 10 ) {
        print("a:", a, "\n");
    } else {
        print("a:", a, "!\n");
    }
}

fun main() -> none {
    arr<num> list = [1,2,3,4,5];
    for(num item in list) {
        hello(item, 5);
    }
}
```

## Grammar

```
<program>           = <function-decl> { <function-decl> }

<function-decl>     = "fun", <symbol>, <function-body>

<function-body>     = "(", [ <exp-list> ], ")" <block>

<exp-list>          = <expression {"," <expression> }

<block>             = "{" { <statement>, `;´ } [ <blockstop>, `;´ ] "}"

<blockstop>         = "return" [<expression>] | "break"

<statement>         = <var-name>, "=", <expression>
                    | "if", "(", <expression>, ")", <block>
                    | "if", "(", <expression>, ")", <block> { "else", "if", <block> } ["else" <block>]
                    | "for", "(", <symbol>, "in", <symbol>, ")", <block>
                    | <func-call>

<var-name>			= <symbol>

<func-call>         = <symbol>, "(", [ <exp-list> ], ")"

<array-def>			= "[", [ <value> { ",", <value> } ], "]"

<value>				= <number>
					| <string>
					| <boolean>
					| <character>

<expression>        = <value>
                    | <var-name>
                    | <func-call>
					| <array-def>
                    | <expression> <binop> <expression>
                    | <unop> <expression>
                    | "(", <expression>, ")"

<binop>             =   "+" | "-"  | "*"   | "/" 
                    | "and" | "or" | "xor"
                    | "=="  | ">=" | "<="  | "!=" 
                    | ">"   | "<"

<unop>              = "not" | "-"

```

### TODO

* [ ] The parsing of language source code.
* [ ] IR based typechecking.
* [ ] OP_NATIVE_CALL via AST_CALL ( 'extern print(any val) -> none;' ).
* [ ] OP_BREAK needs an implementation.
* [X] Remove gvm_asm and clean up unused datatypes.
* [X] Remove or cleanup asmutils.
* [ ] Bytecode op args should be 32 bit.
* [ ] Compiler errors and warnings.
* [ ] Proper VM traps instead of asserts and exit(1).
* [X] Function arguments in VM needs to be treated like locals.
* [X] Use indices instead of strings for identifiers (locals, globals, function names, etc).
* [X] Remove the concept of globals. There should always be a stack-frame present.

### Lua EBNF (for reference)

```
	chunk ::= {stat [`;´]} [laststat [`;´]]

	block ::= chunk

	stat ::=  varlist `=´ explist | 
		 functioncall | 
		 do block end | 
		 while exp do block end | 
		 repeat block until exp | 
		 if exp then block {elseif exp then block} [else block] end | 
		 for Name `=´ exp `,´ exp [`,´ exp] do block end | 
		 for namelist in explist do block end | 
		 function funcname funcbody | 
		 local function Name funcbody | 
		 local namelist [`=´ explist] 

	laststat ::= return [explist] | break

	funcname ::= Name {`.´ Name} [`:´ Name]

	varlist ::= var {`,´ var}

	var ::=  Name | prefixexp `[´ exp `]´ | prefixexp `.´ Name 

	namelist ::= Name {`,´ Name}

	explist ::= {exp `,´} exp

	exp ::=  nil | false | true | Number | String | `...´ | function | 
		 prefixexp | tableconstructor | exp binop exp | unop exp 

	prefixexp ::= var | functioncall | `(´ exp `)´

	functioncall ::=  prefixexp args | prefixexp `:´ Name args 

	args ::=  `(´ [explist] `)´ | tableconstructor | String 

	function ::= function funcbody

	funcbody ::= `(´ [parlist] `)´ block end

	parlist ::= namelist [`,´ `...´] | `...´

	tableconstructor ::= `{´ [fieldlist] `}´

	fieldlist ::= field {fieldsep field} [fieldsep]

	field ::= `[´ exp `]´ `=´ exp | Name `=´ exp | exp

	fieldsep ::= `,´ | `;´

	binop ::= `+´ | `-´ | `*´ | `/´ | `^´ | `%´ | `..´ | 
		 `<´ | `<=´ | `>´ | `>=´ | `==´ | `~=´ | 
		 and | or

	unop ::= `-´ | not | `#´
```
### Reference info

#### EBNF

```<term> ::= [ "-" ] <factor>```
here the - is optional

```<args> ::= <arg> { "," <arg> }```
comma separated list, the {}-part is optional

```<expr> ::= <term> ("+" | "-")``` <expr>
allows both + and -

#### Regular extensions to BNF

- postfix * means "repeated 0 or more times"
- postfix + means "repeated 1 or more times"
- postfix ? means "0 or 1 times"

##### Python example
https://docs.python.org/3/reference/grammar.html

