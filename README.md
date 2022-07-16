# kcc

Rui Ueyama さんの「低レイヤを知りたい人のための C コンパイラ作成入門」
を参考に開発しています。

## Implemented

- 制御構文, 配列, ポインター
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

- 構造体の初期化式
- 名前空間ごとの変数管理
- 匿名構造体
- 関数ポインタ
- 型の入れ子定義

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
                 | ("struct" | "union") <ident> "{" <struct_declaration>* "}"
                 | "enum" <ident>
                 | "enum" <ident>? "{" <enumerator_list> "}"
                 | "signed"
                 | "unsigned"
<type_qualifier> = "const"
<initialize> = <assign>
             | "{" <initialize> ("," <initialize>)* "}"
<declarator> = <pointer> <ident> <type_suffix>
<abstruct_declarator> = <pointer> <type_suffix>
<pointer> = ("*" <type_qualifier>?) *
<type_name> = <declaration_specifier> <abstruct_declarator>
<type_suffix> = "[" <const_expr>? "]" <type_suffix> | ε
<struct_declaration> = <declaration_specifier> <declarator> ";"
<enumerator_list> = <enumerator> (",", <enumerator>)* ","?
<enumerator> = <ident>
             | <ident> "=" <conditional>
<declaration_param> = <declaration_specifier> (<abstruct_declarator> | <declarator>)
<func_define> = <declaration_specifier> <pointer> <ident>
                "(" (<declaration_param> ("," <declaration_param>)* ("," "...")? | "void" | ε)  ")"
                <compound_stmt>
<compound_stmt> = "{" (<declaration> | <stmt>)* "}"
<stmt> = <expr>? ";"
       | "return" <expr>? ";"
       | "if" "(" <expr> ")" <stmt> ("else" <stmt>)?
       | "while" "(" <expr> ")" <stmt>
       | "do" <stmt> "while" "(" <expr> ")" ";"
       | "for" "(" <expr>? ";" <expr>? ";" <expr>? ")" <stmt>
       | "switch" "(" <expression> ")" <statement>
       | ("continue" | "break") ";"
       | <labeled>
       | <compound_stmt>
<labeled> = "case" <const_expr> ":" <statement>
          | "default" ":" <statement>
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
