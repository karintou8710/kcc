# kcc

Rui Ueyama さんの「低レイヤを知りたい人のための C コンパイラ作成入門」
を参考に開発しています。

## Implemented

- if, for, 配列, ポインター
- グローバル変数の定義
- 関数の定義、呼び出し
- struct, union
- ブロックスコープ
- 配列の初期化式
- typedef, enum
- #include "header"
- 可変長引数
- \_Bool

## TODO

- switch
- 構造体の初期化式

## Build

```bash
# build
$ git clone https://github.com/karintou8710/kcc
$ cd kcc
$ make

# test
$ make testall

# diff between gen2 and gen3
$ make diff
```

## BNF

```
<program> = ( <declaration> | <func_define> )*
<declaration> = <type_specifier> <declaration_var> ("," <declaration_var>)*
<declaration_var> = <pointer> <ident> <type_suffix> ("=" <initialize>)?
<initialize> = <assign>
             | "{" <initialize> ("," <initialize>)* "}"
<pointer> = "*"*
<storage_class>  = "typedef" | "entern"
<type_specifier> = <storage_class>? "int"
                 | <storage_class>? "char"
                 | <storage_class>? "void"
                 | <storage_class>? "short"
                 | <storage_class>? "_Bool"
                 | <storage_class>? "long" "long"? "int"?
                 | <storage_class>? ("struct" | "union") <ident>
                 | <storage_class>? ("struct" | "union") <ident> "{" <struct_declaration>* "}"
                 | <storage_class>? "enum" <ident>
                 | <storage_class>? "enum" <ident>? "{" <enumerator_list> "}"
<type_name> = <type_specifier> <pointer> <type_suffix>
<type_suffix> = "[" <num>? "]" <type_suffix> | ε
<struct_declaration> = <type_specifier> <pointer> <ident> ";"
<enumerator_list> = <enumerator> (",", <enumerator>)* ","?
<enumerator> = <ident>
             | <ident> "=" <conditional>
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
<conditional> = <logical_or> | <logical_or> "?" <assign> ":" <conditional>
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
<postfix> = <primary>  ( ("[" <expr> "]") | "." | "->" ) *
<funcall> = "(" (<assign> ("," <assign>)*)? ")"
<primary> = "(" <expr> ")" | "(" <compound_stmt> ")" | <num> | <string> | <ident> <funcall>?
```
