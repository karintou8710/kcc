#include "9cc.h"

/*
 * ASTからスタックマシンを使ってアセンブリを生成する
 *
 * スタックマシンの使用
 *
 * 全てのノードは必ず一つだけの要素が残るようにpushする
 */

static void gen(Node *node);
static void load(Type *ty);

static char *argreg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static char *argreg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static char *argreg16[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static char *argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};

static char *raxreg[] = {"rax", "eax", "ax", "al"};   // size: 8, 4, 2, 1
static char *rdireg[] = {"rdi", "edi", "di", "dil"};  // size: 8, 4, 2, 1
static Function *current_fn;

// continue, breakに使う
int now_loop_count = 0;

typedef enum {
    REG_RAX,
    REG_RDI,
} RegKind;

static char *get_argreg(int index, Type *ty) {
    if (ty->kind == TYPE_ARRAY)
        return argreg64[index];

    switch (ty->size) {
        case 8:
            return argreg64[index];
            break;
        case 4:
            return argreg32[index];
            break;
        case 2:
            return argreg16[index];
            break;
        case 1:
            return argreg8[index];
            break;
    }

    error("size: %d はサポートしてない引数です。", ty->size);
}

static int size_to_regindex(int size) {
    switch (size) {
        case 8:
            return 0;  // 64bit
            break;
        case 4:
            return 1;  // 32bit
            break;
        case 2:
            return 2;  // 16bit
            break;
        case 1:
            return 3;  // 8bit
            break;
    }

    error("size_to_regindex() failure: サポートしてないレジスターのサイズです");
}

static char *proper_register(Type *ty, RegKind kind) {
    int size = ty->size;
    if (ty->kind == TYPE_ARRAY) {
        while (ty->ptr_to)
            ty = ty->ptr_to;
        size = ty->size;
    }

    int index = size_to_regindex(size);

    switch (kind) {
        case REG_RAX:
            return raxreg[index];
        case REG_RDI:
            return rdireg[index];
    }

    error("サポートしていないレジスターです");
}

static void push() {
    printf("  push rax\n");
}

static void push_rdi() {
    printf("  push rdi\n");
}

static void push_num(int num) {
    printf("  push %d\n", num);
}

static void pop() {
    printf("  pop rax\n");
}

static void pop_rdi() {
    printf("  pop rdi\n");
}

static void assign_lvar_offsets() {
    for (int i = 0; funcs[i]; i++) {
        if (funcs[i]->locals->next_offset > 0) {
            funcs[i]->stack_size = funcs[i]->locals->next_offset;
        } else {
            funcs[i]->stack_size = funcs[i]->locals->offset;
        }
    }
}

// 左辺値は変数である必要がある
// ローカル変数のアドレスを生成
static void gen_lval(Node *node) {
    if (node->kind != ND_VAR) {
        error("代入の左辺値が変数ではありません");
    }

    if (node->var->is_global) {
        printf("  lea rax, [rip+%s]\n", node->var->name);
    } else {
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", node->var->offset);
    }

    push();
}

// ポインター変数への代入への対応
static void gen_addr(Node *node) {
    if (node->kind == ND_DEREF) {
        gen(node->lhs);
        return;
    } else if (node->kind == ND_VAR) {
        gen_lval(node);
        return;
    } else if (node->kind == ND_ADD || node->kind == ND_SUB) {
        gen(node);
        return;
    } else if (node->kind == ND_STRUCT_MEMBER) {
        gen_addr(node->lhs);
        pop();
        printf("  add rax, %d\n", node->val);
        push();
        return;
    } else if (node->kind == ND_TERNARY) {
        gen(node);
        return;
    }

    error("左辺値がポインターまたは変数ではありません");
}

static void load(Type *ty) {
    if (ty->kind == TYPE_ARRAY) {
        // アドレスのまま読みこむようにする
        return;
    }

    if (ty->kind == TYPE_CHAR) {
        printf("  movsx eax, BYTE PTR [rax]\n");
        return;
    }

    printf("  mov %s, [rax]\n", proper_register(ty, REG_RAX));
    switch (ty->size) {
        case 4:
            printf("  cdqe\n");
            break;
        case 2:
            printf("  cwde\n");
            break;
        case 1:
            printf("  cbw\n");
            break;
    }
}

static void gen(Node *node) {
    // 入れ子ループに対応するためにローカル変数で深さを持つ
    int loop_count = label_loop_count;  // ループカウントの一時保存にも使う
    int if_count = label_if_count;

    switch (node->kind) {
        case ND_NULL:
            push();
            return;
        case ND_NUM:
            push_num(node->val);
            return;
        case ND_STRING:
            printf("  lea rax, [rip+.LC%d]\n", node->val);
            push();
            return;
        case ND_STRUCT_MEMBER:
            gen_addr(node);
            pop();
            add_type(node);
            load(node->type);
            push();
            return;
        case ND_VAR:
            gen_lval(node);
            pop();
            add_type(node);
            load(node->type);
            push();
            return;
        case ND_ADDR:
            gen_addr(node->lhs);
            return;
        case ND_DEREF:
            gen(node->lhs);
            pop();
            add_type(node);
            load(node->type);
            push();
            return;
        case ND_ASSIGN:
            gen_addr(node->lhs);
            gen(node->rhs);
            pop_rdi();
            pop();
            add_type(node->lhs);
            printf("  mov [rax], %s\n", proper_register(node->lhs->type, REG_RDI));
            push_rdi();
            return;
        case ND_RETURN:
            gen(node->lhs);
            pop_rdi();
            if (current_fn->ret_type->kind == TYPE_CHAR) {
                printf("  movsx rax, dil\n");
            } else if (current_fn->ret_type->kind != TYPE_VOID) {
                if (current_fn->ret_type->size < 8) {
                    printf("  movsx rax, %s\n", proper_register(current_fn->ret_type, REG_RDI));
                } else if (current_fn->ret_type->size == 8) {
                    printf("  mov rax, rdi\n");
                } else {
                    error("gen() failure: ND_RETURN can't return over 8 size.");
                }
            }

            printf("  jmp .L.return.%s\n", current_fn->name);
            // returnは終了なので数合わせなし
            return;
        case ND_IF:
            label_if_count++;
            gen(node->cond);
            pop();
            printf("  cmp rax, 0\n");
            if (node->els) {
                printf("  je  .Lifelse%04d\n", if_count);
                gen(node->then);
                printf("  jmp .Lifend%04d\n", if_count);
                printf(".Lifelse%04d:\n", if_count);
                gen(node->els);
                printf(".Lifend%04d:\n", if_count);
            } else {
                printf("  je  .Lifend%04d\n", if_count);
                gen(node->then);
                pop();  // 数合わせ
                printf(".Lifend%04d:\n", if_count);
                push();  // 数合わせ
            }

            return;
        case ND_TERNARY:
            label_if_count++;
            gen(node->cond);
            pop();
            printf("  cmp rax, 0\n");
            printf("  je  .Lifelse%04d\n", if_count);
            gen(node->then);
            printf("  jmp .Lifend%04d\n", if_count);
            printf(".Lifelse%04d:\n", if_count);
            gen(node->els);
            printf(".Lifend%04d:\n", if_count);

            return;
        case ND_WHILE:
            label_loop_count++;
            printf(".Lloopbegin%04d:\n", loop_count);
            gen(node->cond);
            pop();
            printf("  cmp rax, 0\n");
            printf("  je  .Lloopend%04d\n", loop_count);

            // 次のループカウントにする
            now_loop_count = label_loop_count;
            gen(node->body);
            // 元のループカウントに戻す
            now_loop_count = loop_count;

            // whileには必要ないが、for文との辻褄合わせに入れる
            printf(".Lloopinc%04d:\n", loop_count);
            printf("  jmp .Lloopbegin%04d\n", loop_count);
            printf(".Lloopend%04d:\n", loop_count);
            return;
        case ND_FOR:
            label_loop_count++;
            if (node->init) {
                gen(node->init);
            }
            printf(".Lloopbegin%04d:\n", loop_count);
            if (node->cond) {
                gen(node->cond);
                pop();
                printf("  cmp rax, 0\n");
                printf("  je  .Lloopend%04d\n", loop_count);
            }

            now_loop_count = label_loop_count;
            gen(node->body);
            now_loop_count = loop_count;

            printf(".Lloopinc%04d:\n", loop_count);
            if (node->inc) {
                gen(node->inc);
            }
            printf("  jmp .Lloopbegin%04d\n", loop_count);
            printf(".Lloopend%04d:\n", loop_count);
            return;
        case ND_BREAK:
            // loop_countは次の深さになっているので１を引く
            if (now_loop_count - 1 < 0) {
                error("forブロックの中でbreakを使用していません。");
            }
            push();  // 数合わせ
            printf("  jmp .Lloopend%04d\n", now_loop_count - 1);
            return;
        case ND_CONTINUE:
            // loop_countは次の深さになっているので１を引く
            if (now_loop_count - 1 < 0) {
                error("forブロックの中でbreakを使用していません。");
            }
            push();  // 数合わせ
            printf("  jmp .Lloopinc%04d\n", now_loop_count - 1);
            return;
        case ND_BLOCK:
            for (int i = 0; i < node->stmts->len; i++) {
                gen(node->stmts->body[i]);
                pop();
            }
            push();  // 数合わせ
            return;
        case ND_SUGER:
            for (int i = 0; i < node->stmts->len; i++) {
                gen(node->stmts->body[i]);
                pop();
            }
            push();  // 数合わせ
            return;
        case ND_CALL: {
            int nargs = node->args->len;
            for (int i = 0; i < nargs; i++) {
                gen(node->args->body[i]);
            }
            for (int i = nargs - 1; i >= 0; i--) {
                Var *l = node->args->body[i];
                printf("  pop %s\n", argreg64[i]);
            }
            // rspを16の倍数にアライメントしてからコールする
            printf("  mov rax, 0\n");
            printf("  push rbp\n");
            printf("  mov rbp, rsp\n");
            printf("  and rsp, -16\n");
            printf("  call %s\n", node->fn_name);
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            push();  // 数合わせ
            return;
        }
        case ND_LOGICALNOT:
            gen(node->lhs);
            pop();
            printf("  test rax, rax\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            push();
            return;
    }

    // 主に演算のATSで読みこまれる
    gen(node->lhs);
    gen(node->rhs);

    pop_rdi();
    pop();

    switch (node->kind) {
        case (ND_ADD):
            printf("  add rax, rdi\n");
            break;
        case (ND_SUB):
            printf("  sub rax, rdi\n");
            break;
        case (ND_MUL):
            printf("  imul rax, rdi\n");
            break;
        case (ND_DIV):
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case (ND_MOD):
            printf("  cqo\n");
            printf("  idiv rdi\n");
            printf("  mov rax, rdx\n");
            break;
        case (ND_EQ):
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case (ND_NE):
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case (ND_LT):
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case (ND_LE):
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
        case (ND_LOGICAL_AND):
            printf("  cmp rax, 0\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            printf("  cmp rdi, 0\n");
            printf("  setne dil\n");
            printf("  movzb rdi, dil\n");
            printf("  and rax, rdi\n");
            break;
        case (ND_LOGICAL_OR):
            printf("  cmp rax, 0\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            printf("  cmp rdi, 0\n");
            printf("  setne dil\n");
            printf("  movzb rdi, dil\n");
            printf("  or rax, rdi\n");
            break;
    }

    push();
}

void codegen() {
    // 各関数のoffsetを計算
    assign_lvar_offsets();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");

    printf(".data\n");

    // 文字列リテラルの生成
    for (int i = 0; i < string_literal->len; i++) {
        Token *tok = (Token *)string_literal->body[i];
        printf(".LC%d:\n", tok->str_literal_index);
        printf("  .string \"%s\"\n", tok->str);
    }

    // グローバル変数の生成
    for (Var *var = globals; var != NULL; var = var->next) {
        printf("%s:\n", var->name);
        // 宣言のみ
        if (var->ginit->len == 0) {
            printf("  .zero %d\n", var->type->size);
            continue;
        }

        // 初期化式あり
        for (int i = 0; i < var->ginit->len; i++) {
            GInit_el *g = var->ginit->body[i];

            // ポインターかラベル
            if (g->str) {
                printf("  .quad %s\n", g->str);
                continue;
            }

            int s = array_base_type_size(var->type);
            if (s == 8) {
                printf("  .quad %d\n", g->val);
            } else if (s == 4) {
                printf("  .long %d\n", g->val);
            } else if (s == 2) {
                printf("  .value %d\n", g->val);
            } else if (s == 1) {
                printf("  .byte %d\n", g->val);
            }
        }
    }

    printf(".text\n");
    // 先頭の式から順にコード生成
    for (int i = 0; funcs[i]; i++) {
        current_fn = funcs[i];
        printf(".globl %s\n", current_fn->name);
        printf("%s:\n", current_fn->name);

        // プロローグ
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", current_fn->stack_size);

        int j = 0;
        for (Var *var = current_fn->params; var; var = var->next) {
            printf("  mov rax, rbp\n");
            printf("  sub rax, %d\n", var->offset);
            Type *ty = var->type;
            if (var->type->kind == TYPE_ARRAY)
                ty = new_ptr_type(var->type);
            printf("  mov [rax], %s\n", get_argreg(j++, ty));
        }

        gen(current_fn->body);
        // 式の評価結果としてスタックに一つの値が残っている
        pop();

        // エピローグ
        // 最後の式の結果がRAXに残っているのでそれが返り値になる
        printf(".L.return.%s:\n", current_fn->name);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
}