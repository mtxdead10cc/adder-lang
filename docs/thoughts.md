# Random thoughts and notes

Some old notes, from early on in the project.
Not sure this is of much use to anyone. 

## Syntax

```
void hello(float arg1, float arg2) {
    float a = add(arg1, arg2);
    if( a <= 10 ) {
        print("a:", a, "\n");
    } else {
        print("a:", a, "!\n");
    }
}

void main() {
    arr<int> list = [1,2,3,4,5];
    for(num item in list) {
        hello(float(item), float(5));
    }
}
```

## Types
How I think it should be ...  
```
    type  ::= int
            | float
            | bool 
            | char 
            | string 
            | array<type> 
            | user-type 
            | none
```
In the VM the following should be true:
* int, float, bool, char are all just ```<number>```
* string is actually ```array<char>```
* any array is actually ```<reference(startaddress, length)>```
* user-type is some sort of reference

## Grammar

```
<program>           = <function-decl> { <function-decl> }

<function-decl>     = <type-name>, <symbol>, <function-body>

<function-body>     = "(", [ <exp-list> ], ")" <block>

<exp-list>          = <expression {"," <expression> }

<block>             = "{" { [ <statement>, `;´ ] } "}"

<blockstop>         = "return" [<expression>] | "break"

<statement>         = <var-name>, "=", <expression>
                    | "if", "(", <expression>, ")", <block>
                    | "if", "(", <expression>, ")", <block> { "else", "if", <block> } ["else" <block>]
                    | "for", "(", <symbol>, "in", <symbol>, ")", <block>
                    | <func-call>
					| <blockstop>

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

* [ ] OP_BREAK needs an implementation.
* [ ] Compiler errors and warnings.
* [ ] Proper VM traps instead of asserts and exit(1).


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

