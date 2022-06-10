# kcc

Rui Ueyama さんの「低レイヤを知りたい人のための C コンパイラ作成入門」
を参考に、セルフホストを目指して作成しています。

## Implemented

- if, for, 配列, ポインター
- グローバル変数の定義
- 関数の定義、呼び出し
- 構造体の基礎
- ブロックスコープ
- 配列の初期化式
- typedef, enum
- #include "header"

## TODO

- switch
- 可変長引数
- 構造体の初期化式
- \_Bool

## Build

```bash
# build
$ git clone https://github.com/karintou8710/kcc
$ cd kcc
$ make

# test
$ make test
```

## BNF

```
<program> = ( <declaration_global> | <func_define> )*
<declaration_global> = <declaration> ";"
<initialize> = <assign>
             | "{" <initialize> ("," <initialize>)* "}"
<pointer> = "*"*
<declaration_var> = <pointer> <ident> <type_suffix> ("=" <initialize>)?
<declaration> = <type_specifier> <declaration_var> ("," <declaration_var>)*
<struct_declaration> = <type_specifier> <pointer> <ident> ";"
<storage_class>  = "typedef" | "entern"
<type_specifier> = <storage_class>? "int"
                 | <storage_class>? "char"
                 | <storage_class>? "void"
                 | <storage_class>? "short"
                 | <storage_class>? "long" "long"? "int"?
                 | <storage_class>? "struct" <ident>
                 | <storage_class>? "struct" <ident> "{" <struct_declaration>* "}"
                 | <storage_class>? "enum" <ident>
                 | <storage_class>? "enum" <ident>? "{" <enumerator_list> "}"
<type_name> = <type_specifier> <pointer> <type_suffix>
<enumerator_list> = <enumerator> (",", <enumerator>)* ","?
<enumerator> = <ident>
             | <ident> "=" <conditional>
<type_suffix> = "[" <num>? "]" <type_suffix> | ε
<declaration_param> = <type_specifier> <pointer> <ident> <type_suffix>
<func_define> = <type_specifier> <pointer> <ident>
                "(" (<declaration_param> ("," <declaration_param>)* ("," "...")? | "void" | ε)  ")"
                <compound_stmt>
<compound_stmt> = { <stmt>* }
<stmt> = <expr>? ";"
       | "return" <expr>? ";"
       | "if" "(" <expr> ")" <stmt> ("else" <stmt>)?
       | "while" "(" <expr> ")" <stmt>
       | "for" "(" <expr>? ";" <expr>? ";" <expr>? ")" <stmt>
       | ("continue" | "break")
       | <compound_stmt>
<expr> = <assign> ("," <assign>)* | <declaration>
<assign> = <conditional> ("=" <assign>)?
         | <conditional> ( "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>=" ) <conditional>
<conditional> = <logical_expression> | <logical_expression> "?" <assign> ":" <conditional>
<logical_expression> = <inclusive_or> ("&&" <inclusive_or> | "||" <inclusive_or>)*
<inclusive_or> = <exclusive_or> ( "|" <exclusive_or> )*
<exclusive_or> = <and> ( "^" <and> )*
<and> = <equality> ( "&" <equality> )*
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
<postfix> = <primary>  ( ("[" <expr> "]") | "." | "->" ) *
<funcall> = "(" (<assign> ("," <assign>)*)? ")"
<primary> = "(" <expr> ")" | "(" <compound_stmt> ")" | <num> | <string> | <ident> <funcall>?
```
