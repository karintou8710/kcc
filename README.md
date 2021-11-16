# 自作cコンパイラ

Rui Ueyamaさんの「低レイヤを知りたい人のためのCコンパイラ作成入門」
を参考に作成しています。

# BNF

```
program = ( declaration_var | func_define )*

declaration_global = declaration_var ;

declaration_var = declaration_specifier ident type_suffix

declaration_specifier = int "*"*

type_suffix = "[" num "]" type_suffix | ε

declaration_param = declaration_specifier ident type_suffix

func_define = declaration_specifier ident
"(" (declaration_param ("," declaration_param)*)?  ")" compound_stmt

compound_stmt = { stmt* }

stmt   = expr? ";"
       | "return" expr ";"
       | "if" "(" expr ")" stmt ("else" stmt)?
       | "while" "(" expr ")" stmt
       | "for" "(" expr? ";" expr? ";" expr? ")" stmt
       | compound_stmt

expr = assign | declaration_var

assign = equality ("=" assign)?

equality = relational ("==" relational | "!=" relational)*

relational = add ("<" add | "<=" add | ">" add | ">=" add)*

add = mul ("+" mul | "-" mul)*

mul = unary ("*" unary | "/" unary)*

unary   = "+"? array_suffix
        | "-"? array_suffix
        | "*" array_suffix   ("*" unaryでもいい？)
        | "&" array_suffix
        | "sizeof" unary

array_suffix = primary ("[" expr "]")*

funcall = "(" (expr ("," expr)*)? ")"

primary = "(" expr ")" | num | ident funcall?
```




