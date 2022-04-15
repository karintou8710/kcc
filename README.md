# 自作 c コンパイラ

Rui Ueyama さんの「低レイヤを知りたい人のための C コンパイラ作成入門」
を参考に、セルフホストを目指して作成しています。

## 実装済みの機能

- if, for, 配列, ポインター
- グローバル変数の定義
- 関数の定義、呼び出し
- 構造体の基礎

## TODO

- 初期化式
- else if, switch
- スコープ
- typedef 
- enum

# BNF

```
<program> = ( <declaration_global> | <func_define> )*
<declaration_global> = <declaration> ";"
<initialize> = <assign>
<pointer> = "*"*
<declaration_var> = <pointer> <ident> <type_suffix> ("=" <initialize>)?
<declaration> = <type_specifier> <declaration_var> ("," <declaration_var>)*
<struct_declaration> = <type_specifier> <pointer> <ident> ";"
<type_specifier> = "int"
                 | "char"
                 | "void"
                 | "struct" <ident>
                 | "struct" <ident> "{" <struct_declaration>* "}"
<type_suffix> = "[" <num> "]" <type_suffix> | ε
<declaration_param> = <type_specifier> <pointer> <ident> <type_suffix>
<func_define> = <type_specifier> <pointer> <ident>
                "(" (<declaration_param> ("," <declaration_param>)* | "void" | ε)  ")"
                <compound_stmt>
<compound_stmt> = { <stmt>* }
<stmt> = <expr>? ";"
       | "return" <expr>? ";"
       | "if" "(" <expr> ")" <stmt> ("else" <stmt>)?
       | "while" "(" <expr> ")" <stmt>
       | "for" "(" <expr>? ";" <expr>? ";" <expr>? ")" <stmt>
       | ("continue" | "break")
       | <compound_stmt>
<expr> = <assign> | <declaration>
<assign> = <logical_expression> ("=" <assign>)?
         | <logical_expression> ( "+=" | "-=" | "*=" | "/=" | "%=" ) <logical_expression>
<logical_expression> = <equality> ("&&" <equality> | "||" <equality>)*
<equality> = <relational> ("==" <relational> | "!=" <relational>)*
<relational> = <add> ("<" <add> | "<=" <add> | ">" <add> | ">=" <add>)*
<add> = <mul> ("+" <mul> | "-" <mul>)*
<mul> = <unary> ("*" <unary> | "/" <unary> | "%" <unary> )*
<unary> = "+"? <postfix>
        | "-"? <postfix>
        | "*" <unary>
        | "&" <postfix>
        | "sizeof" <unary>
        | "sizeof" "(" <type_specifier> <pointer> ")"
        | ("++" | "--") <postfix>
        | <postfix> ("++" | "--")
        | "!" <unary>
<postfix> = <primary>  ( ("[" <expr> "]") | "." | "->" ) *
<funcall> = "(" (<expr> ("," <expr>)*)? ")"
<primary> = "(" <expr> ")" | <num> | <string> | <ident> <funcall>?
```
