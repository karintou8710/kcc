# 自作 c コンパイラ

Rui Ueyama さんの「低レイヤを知りたい人のための C コンパイラ作成入門」
を参考に、セルフホストを目指して作成しています。

## 実装済みの機能

- if, for, 配列, ポインター
- グローバル変数の定義
- 関数の定義、呼び出し

## TODO

- struct
- 初期化式
- else if, switch
- スコープ

# BNF

```
program = ( declaration_var | func_define )*

declaration_global = declaration_var ;

declaration_var = type_specifier ident type_suffix

type_specifier = int "*"*

type_suffix = "[" num "]" type_suffix | ε

declaration_param = type_specifier ident type_suffix

func_define = type_specifier ident
"(" (declaration_param ("," declaration_param)*)?  ")" compound_stmt

compound_stmt = { stmt* }

stmt   = expr? ";"
       | "return" expr ";"
       | "if" "(" expr ")" stmt ("else" stmt)?
       | "while" "(" expr ")" stmt
       | "for" "(" expr? ";" expr? ";" expr? ")" stmt
       | compound_stmt

expr = assign | declaration_var

assign = equality ("=" assign)? | equality ( "+=" | "-=" | "*=" | "/=" | "%=" ) equality

equality = relational ("==" relational | "!=" relational)*

relational = add ("<" add | "<=" add | ">" add | ">=" add)*

add = mul ("+" mul | "-" mul)*

mul = unary ("*" unary | "/" unary)*

unary   = "+"? postfix
        | "-"? postfix
        | "*" postfix   ("*" unaryでもいい？)
        | "&" postfix
        | "sizeof" unary

postfix = primary ("[" expr "]")*

funcall = "(" (expr ("," expr)*)? ")"

primary = "(" expr ")" | num | ident funcall?
```
