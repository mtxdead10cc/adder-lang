# Language

## Syntax

```
fun hello(arg1, arg2) {
    a = add(arg1, arg2);
    if( a <= 10 ) {
        print("a:", a, "\n");
    } else {
        print("a:", a, "!\n");
    }
}

fun main() {
    list = [1,2,3,4,5];
    for(item in list) {
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

<statement>         = <symbol>, "=", <expression>
                    | <symbol>, "=", <symbol> <function-body>
                    | "if", "(", <expression>, ")", <block>
                    | "if", "(", <expression>, ")", <block> { "else", "if", <block> } ["else" <block>]
                    | "for", "(", <symbol>, "in", <symbol>, ")", <block>
                    | <func-call>
                    | "[", [ <expression> { ",", <expression> } ], "]"

<func-call>         = <symbol>, "(", [ <expression>, {"," <expression> } ], ")"

<expression>        = <number>
                    | <symbol>
                    | <string>
                    | <boolean>
                    | <func-call>
                    | <expression> <binop> <expression>
                    | <unop> <expression>
                    | "(", <expression>, ")"

<binop>             =   "+" | "-"  | "*"   | "/" 
                    | "and" | "or" | "xor"
                    | "=="  | ">=" | "<="  | "!=" 
                    | ">"   | "<"

<unop>              = "not" | "-"

```

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


