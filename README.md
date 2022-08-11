# kcc

セルフホストを達成した C89 サブセットのコンパイラーです。
Rui Ueyama さんの「低レイヤを知りたい人のための C コンパイラ作成入門」
を参考に開発しています。

## Implemented

- struct, union, typedef, enum, const
- ブロックスコープ
- 配列・構造体の初期化式
- 可変長引数
- \_Bool
- 型の入れ子定義, 関数ポインタ

## TODO

- 名前空間ごとの変数管理

## Build

```bash
# build
$ git clone https://github.com/karintou8710/kcc
$ cd kcc
$ make

# test 1st generation
$ make test1

# test all generation and selfhost
$ make testall

# test selfhost
$ make diff

# run any code with 1st generation
# recommend to use [#include "input.h"]
$ vi input.c
$ make run1
```

## BNF

```
<program> = ( <declaration> | <func_define> )*
<declaration> = <declaration_specifier> <declaration_var> ("," <declaration_var>)* ";"
<declaration_var> = <declarator> ("=" <initialize>)?
<declaration_specifier> = (<storage_class> | <type_specifier> | <type_qualifier>)+
<storage_class>  = "typedef" | "entern"
<type_specifier> = "char"
                 | "short"
                 | "int"
                 | "long"
                 | "void"
                 | "_Bool"
                 | ("struct" | "union") <ident>
                 | ("struct" | "union") <ident>? "{" <struct_declaration>* "}"
                 | "enum" <ident>
                 | "enum" <ident>? "{" <enumerator_list> "}"
                 | "signed"
                 | "unsigned"
<type_qualifier> = "const"
<initialize> = <assign>
             | "{" <initialize> ("," <initialize>)* "}"
<declarator> = <pointer> <ident> <type_suffix>
             | <pointer> "(" <declarator> ")" <type_suffix>
<abstruct_declarator> = <pointer> <type_suffix>
                      | <pointer> "(" <declarator> ")" <type_suffix>
<pointer> = ("*" <type_qualifier>?) *
<type_name> = <declaration_specifier> <abstruct_declarator>
<type_suffix> = "[" <const_expr>? "]" <type_suffix>
              | "(" (<declaration_param> ("," <declaration_param>)* ("," "...")? | "void" | ε)  ")" <type_suffix>
              | ε
<struct_declaration> = <declaration_specifier> <declarator> ("," <declarator>)* ";"
<enumerator_list> = <enumerator> (",", <enumerator>)* ","?
<enumerator> = <ident>
             | <ident> "=" <conditional>
<declaration_param> = <declaration_specifier> (<abstruct_declarator> | <declarator>)
<declaration_params> = "(" (<declaration_param> ("," <declaration_param>)* ("," "...")? | "void" | ε)  ")"
<func_define> = <declaration_specifier> <declarator> <compound_stmt>
<compound_stmt> = "{" (<declaration> | <stmt>)* "}"
<stmt> = <expr>? ";"
       | "return" <expr>? ";"
       | "if" "(" <expr> ")" <stmt> ("else" <stmt>)?
       | "while" "(" <expr> ")" <stmt>
       | "do" <stmt> "while" "(" <expr> ")" ";"
       | "for" "(" <expr>? ";" <expr>? ";" <expr>? ")" <stmt>
       | "switch" "(" <expression> ")" <statement>
       | ("continue" | "break") ";"
       | "goto" <ident> ";"
       | <labeled>
       | <compound_stmt>
<labeled> = "case" <const_expr> ":" <statement>
          | "default" ":" <statement>
          | <ident> ":" <statement>
<expr> = <assign> ("," <assign>)* | <declaration>
<assign> = <conditional>
         | <conditional> ( "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>=" ) <assign>
<conditional> = <logical_or> | <logical_or> "?" <expression> ":" <conditional>
<logical_or> = <logical_and> ("||" <logical_and>)*
<logical_and> = <inclusive_or> ("&&" <inclusive_or>)*
<inclusive_or> = <exclusive_or> ( "|" <exclusive_or> )*
<exclusive_or> = <bin_and> ( "^" <bin_and> )*
<bin_and> = <equality> ( "&" <equality> )*
<equality> = <relational> ("==" <relational> | "!=" <relational>)*
<relational> = <shift> ("<" <shift> | "<=" <shift> | ">" <shift> | ">=" <shift>)*
<shift> = <add> (">>" <add> | "<<" <add>)*
<add> = <mul> ("+" <mul> | "-" <mul>)*
<mul> = <cast> ("*" <cast> | "/" <cast> | "%" <cast> )*
<cast> = "(" <type_name> ")" <cast>
       | <unary>
<unary> = <postfix>
        | "sizeof" <unary>
        | "sizeof" "(" <type_name> ")"
        | ("++" | "--") <postfix>
        | <unary> ("++" | "--")
        | ("!" | "~" | "+" | "-" | "*" | "&") <cast>
<postfix> = <primary>  ( ("[" <expr> "]") | "." | "->" | <funcall> ) *
<funcall> = "(" (<assign> ("," <assign>)*)? ")"
<primary> = "(" <expr> ")" | "(" <compound_stmt> ")" | <num> | <string> | <ident>
```
