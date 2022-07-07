#include "kcc.h"

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

// continue, breakでどこに飛ぶのか値を保持
static int continue_label = -1;
static int logical_label = 0;

typedef enum RegKind {
    REG_RAX,
    REG_RDI,
} RegKind;

static void delete_prototype_func() {
    for (int i = 0; i < funcs->len; i++) {
        Function *fn = funcs->body[i];
        if (fn->is_prototype) {
            vec_delete(funcs, i);
            i--;
        }
    }
}

static char *get_argreg(int index, Type *ty) {
    if (ty->kind == TYPE_ARRAY)
        return argreg64[index];

    if (ty->size == 8) {
        return argreg64[index];
    } else if (ty->size == 4) {
        return argreg32[index];
    } else if (ty->size == 2) {
        return argreg16[index];
    } else if (ty->size == 1) {
        return argreg8[index];
    }

    error("size: %d はサポートしてない引数です。", ty->size);
}

static int size_to_regindex(int size) {
    if (size == 8) {
        return 0;  // 64bit
    } else if (size == 4) {
        return 1;  // 32bit
    } else if (size == 2) {
        return 2;  // 16bit
    } else if (size == 1) {
        return 3;  // 8bit
    }

    error("size_to_regindex() failure: [size=%d]サポートしてないレジスターのサイズです", size);
}

static char *proper_register(Type *ty, RegKind kind) {
    int size = ty->size;
    if (ty->kind == TYPE_ARRAY) {
        while (ty->ptr_to)
            ty = ty->ptr_to;
        size = ty->size;
    }

    int index = size_to_regindex(size);

    if (kind == REG_RAX) {
        return raxreg[index];
    } else if (kind == REG_RDI) {
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

static void push_num(long num) {
    printf("  mov rax, %ld\n", num);
    printf("  push rax\n");
}

static void pop() {
    printf("  pop rax\n");
}

static void pop_rdi() {
    printf("  pop rdi\n");
}

static void assign_lvar_offsets() {
    for (int i = 0; i < funcs->len; i++) {
        Function *fn = funcs->body[i];
        if (fn->locals->next_offset > 0) {
            fn->stack_size = fn->locals->next_offset;
        } else {
            fn->stack_size = fn->locals->offset;
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
        printf("  add rax, %ld\n", node->val);
        push();
        return;
    } else if (node->kind == ND_TERNARY) {
        gen(node);
        return;
    } else if (node->kind == ND_SUGER || node->kind == ND_STMT_EXPR) {
        gen(node);
        return;
    }

    error("左辺値がポインターまたは変数ではありません");
}

static void load(Type *ty) {
    if (ty->kind == TYPE_ARRAY || ty->kind == TYPE_STRUCT || ty->kind == TYPE_UNION) {
        // アドレスのまま読みこむようにする
        return;
    }

    if (ty->kind == TYPE_CHAR) {
        printf("  movsx eax, BYTE PTR [rax]\n");
        return;
    }

    printf("  mov %s, [rax]\n", proper_register(ty, REG_RAX));

    if (ty->size == 4) {
        printf("  cdqe\n");
    } else if (ty->size == 2) {
        printf("  cwde\n");
    } else if (ty->size == 1) {
        printf("  cbw\n");
    }
}

static void gen(Node *node) {
    // 入れ子ループに対応するためにローカル変数で深さを持つ
    int loop_count = label_loop_count;  // 現在のラベルの値を保持
    int if_count = label_if_count;

    if (node->kind == ND_NULL) {
        push();
        return;
    } else if (node->kind == ND_NUM) {
        push_num(node->val);
        return;
    } else if (node->kind == ND_STRING) {
        printf("  lea rax, [rip+.LC%ld]\n", node->val);
        push();
        return;
    } else if (node->kind == ND_STRUCT_MEMBER) {
        gen_addr(node);
        pop();
        load(node->type);
        push();
        return;
    } else if (node->kind == ND_VAR) {
        gen_lval(node);
        pop();
        add_type(node);
        load(node->type);
        push();
        return;
    } else if (node->kind == ND_ADDR) {
        gen_addr(node->lhs);
        return;
    } else if (node->kind == ND_DEREF) {
        gen(node->lhs);
        pop();
        add_type(node);
        load(node->type);
        push();
        return;
    } else if (node->kind == ND_ASSIGN) {
        gen_addr(node->lhs);
        gen(node->rhs);
        pop_rdi();
        pop();
        add_type(node->lhs);
        if (node->type->kind == TYPE_STRUCT || node->type->kind == TYPE_UNION) {
            // メモリコピー
            for (int i = 0; i < node->type->size; i++) {
                printf("  mov r8, [rdi+%d]\n", i);
                printf("  mov [rax+%d], r8\n", i);
            }
        } else {
            printf("  mov [rax], %s\n", proper_register(node->lhs->type, REG_RDI));
        }

        push_rdi();
        return;
    } else if (node->kind == ND_RETURN) {
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
    } else if (node->kind == ND_IF) {
        label_if_count++;
        gen(node->cond);
        pop();
        printf("  cmp rax, 0\n");
        if (node->els) {
            printf("  je  .Lifelse%04d\n", if_count);
            gen(node->then);
            pop();
            printf("  jmp .Lifend%04d\n", if_count);
            printf(".Lifelse%04d:\n", if_count);
            gen(node->els);
            pop();
            printf(".Lifend%04d:\n", if_count);
        } else {
            printf("  je  .Lifend%04d\n", if_count);
            gen(node->then);
            pop();
            printf(".Lifend%04d:\n", if_count);
        }

        push();  // 数合わせ
        return;
    } else if (node->kind == ND_TERNARY) {
        label_if_count++;
        gen(node->cond);
        pop();
        printf("  cmp rax, 0\n");
        printf("  je  .Lifelse%04d\n", if_count);
        gen(node->then);
        pop();
        printf("  jmp .Lifend%04d\n", if_count);
        printf(".Lifelse%04d:\n", if_count);
        gen(node->els);
        pop();
        printf(".Lifend%04d:\n", if_count);
        push();  // 数合わせ

        return;
    } else if (node->kind == ND_WHILE) {
        label_loop_count++;
        printf(".Lloopbegin%04d:\n", loop_count);

        gen(node->cond);
        pop();

        printf("  cmp rax, 0\n");
        printf("  je  .Lloopend%04d\n", loop_count);

        int tmp_label = continue_label;
        continue_label = loop_count;
        gen(node->body);
        pop();
        continue_label = tmp_label;

        // whileには必要ないが、for文との辻褄合わせに入れる
        printf(".Lloopinc%04d:\n", loop_count);
        printf("  jmp .Lloopbegin%04d\n", loop_count);
        printf(".Lloopend%04d:\n", loop_count);
        push();  // 数合わせ
        return;
    } else if (node->kind == ND_FOR) {
        label_loop_count++;
        if (node->init) {
            gen(node->init);
            pop();
        }

        printf(".Lloopbegin%04d:\n", loop_count);
        if (node->cond) {
            gen(node->cond);
            pop();
            printf("  cmp rax, 0\n");
            printf("  je  .Lloopend%04d\n", loop_count);
        }

        int tmp_label = continue_label;
        continue_label = loop_count;
        gen(node->body);
        pop();
        continue_label = tmp_label;

        printf(".Lloopinc%04d:\n", loop_count);
        if (node->inc) {
            gen(node->inc);
            pop();
        }
        printf("  jmp .Lloopbegin%04d\n", loop_count);
        printf(".Lloopend%04d:\n", loop_count);
        push();  // 数合わせ
        return;
    } else if (node->kind == ND_BREAK) {
        // loop_countは次の深さになっているので１を引く
        if (continue_label < 0) {
            error("forブロックの中でbreakを使用していません。");
        }
        push();  // 数合わせ
        printf("  jmp .Lloopend%04d\n", continue_label);
        return;
    } else if (node->kind == ND_CONTINUE) {
        // loop_countは次の深さになっているので１を引く
        if (continue_label < 0) {
            error("forブロックの中でcontinueを使用していません。");
        }
        push();  // 数合わせ
        printf("  jmp .Lloopinc%04d\n", continue_label);
        return;
    } else if (node->kind == ND_BLOCK || node->kind == ND_STMT_EXPR) {
        for (int i = 0; i < node->stmts->len; i++) {
            gen(node->stmts->body[i]);
            pop();
        }
        push();  // 数合わせ
        return;
    } else if (node->kind == ND_SUGER) {
        for (int i = 0; i < node->stmts->len; i++) {
            gen(node->stmts->body[i]);
            pop();
        }
        push();  // 数合わせ
        return;
    } else if (node->kind == ND_CALL) {
        if (strcmp(node->fn_name, "va_start") == 0) {
            /*
             * va_startをマクロとして実装できないので、内部で va_start(ap, fmt)を
             * *ap = *(struct __builtin_va_list *)__va_area__
             * に置換する。
             */
            gen(node->lhs);
            return;
        }

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
    } else if (node->kind == ND_LOGICAL_NOT) {
        gen(node->lhs);
        pop();
        printf("  test rax, rax\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        push();
        return;
    } else if (node->kind == ND_NOT) {
        gen(node->lhs);
        pop();
        printf("  not rax\n");
        push();
        return;
    } else if (node->kind == ND_CAST) {
        gen(node->lhs);
        pop();
        if (node->type->size == 8) {
            // キャストの必要なし
        } else if (node->type->size == 4) {
            // 4byteだと命令が異なる
            printf("  movsxd rax, eax\n");
        } else {
            printf("  movsx rax, %s\n", proper_register(node->type, REG_RAX));
        }
        push();
        return;
    } else if (node->kind == ND_LOGICAL_AND) {
        logical_label++;
        int label_num = logical_label;

        gen(node->lhs);
        pop();
        printf("  test rax, rax\n");
        printf("  je .Llogicalandfalse%04d\n", label_num);
        gen(node->rhs);
        pop();
        printf("  test rax, rax\n");
        printf("  je .Llogicalandfalse%04d\n", label_num);
        // true
        printf("  mov rax, 1\n");
        printf("  jmp .Llogicalandend%04d\n", label_num);
        // false
        printf(".Llogicalandfalse%04d:\n", label_num);
        printf("  mov rax, 0\n");

        printf(".Llogicalandend%04d:\n", label_num);
        push();
        return;
    } else if (node->kind == ND_LOGICAL_OR) {
        logical_label++;
        int label_num = logical_label;

        gen(node->lhs);
        pop();
        printf("  test rax, rax\n");
        printf("  jne .Llogicalandfalse%04d\n", label_num);
        gen(node->rhs);
        pop();
        printf("  test rax, rax\n");
        printf("  jne .Llogicalandfalse%04d\n", label_num);
        // false
        printf("  mov rax, 0\n");
        printf("  jmp .Llogicalandend%04d\n", label_num);
        // true
        printf(".Llogicalandfalse%04d:\n", label_num);
        printf("  mov rax, 1\n");

        printf(".Llogicalandend%04d:\n", label_num);
        push();
        return;
    }

    // 主に演算のATSで読みこまれる
    gen(node->lhs);
    gen(node->rhs);

    pop_rdi();
    pop();

    if (node->kind == ND_ADD) {
        printf("  add rax, rdi\n");
    } else if (node->kind == ND_SUB) {
        printf("  sub rax, rdi\n");
    } else if (node->kind == ND_MUL) {
        printf("  imul rax, rdi\n");
    } else if (node->kind == ND_DIV) {
        printf("  cqo\n");
        printf("  idiv rdi\n");
    } else if (node->kind == ND_MOD) {
        printf("  cqo\n");
        printf("  idiv rdi\n");
        printf("  mov rax, rdx\n");
    } else if (node->kind == ND_EQ) {
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
    } else if (node->kind == ND_NE) {
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
    } else if (node->kind == ND_LT) {
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
    } else if (node->kind == ND_LE) {
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
    } else if (node->kind == ND_AND) {
        printf("  and rax, rdi\n");
    } else if (node->kind == ND_OR) {
        printf("  or rax, rdi\n");
    } else if (node->kind == ND_XOR) {
        printf("  xor rax, rdi\n");
    } else if (node->kind == ND_LSHIFT) {
        printf("  mov rcx, rdi\n");
        printf("  sal rax, cl\n");
    } else if (node->kind == ND_RSHIFT) {
        printf("  mov rcx, rdi\n");
        printf("  sar rax, cl\n");
    }

    push();
}

void codegen() {
    // プロトタイプ関数を削除
    delete_prototype_func();

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
        // 外部ファイルで定義されるので何も出力しない
        if (var->is_extern) continue;

        printf("  .globl %s\n", var->name);

        // 宣言のみ
        if (var->ginit->len == 0) {
            /*
             * .commは外部ファイルで定義されるかわからない変数。
             * もし定義がされなければ、指定したサイズでメモリ確保される。
             */
            printf("  .comm %s, %d\n", var->name, sizeOfType(var->type));
            continue;
        }

        // 初期化式あり
        printf("%s:\n", var->name);

        for (int i = 0; i < var->ginit->len; i++) {
            GInitEl *g = var->ginit->body[i];

            // ポインターかラベル
            if (g->str) {
                printf("  .quad %s\n", g->str);
                continue;
            }

            int s = array_base_type_size(var->type);
            if (s == 8) {
                printf("  .quad %ld\n", g->val);
            } else if (s == 4) {
                printf("  .long %ld\n", g->val);
            } else if (s == 2) {
                printf("  .value %ld\n", g->val);
            } else if (s == 1) {
                printf("  .byte %ld\n", g->val);
            }
        }
    }

    printf(".text\n");
    // 先頭の式から順にコード生成
    for (int i = 0; i < funcs->len; i++) {
        current_fn = funcs->body[i];
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

        if (current_fn->va_area) {
            int gp = 0;
            for (Var *var = current_fn->params; var; var = var->next) {
                gp++;
            }
            int off = current_fn->va_area->offset;

            // __builtin_va_list
            printf("  mov DWORD PTR [rbp-%d], %d\n", off - 0, gp * 8);  // gp
            printf("  mov DWORD PTR [rbp-%d], 0\n", off - 4);           // fp
            printf("  mov [rbp-%d], rbp\n", off - 16);                  // reg_save_area
            printf("  sub QWORD PTR [rbp-%d], %d\n", off - 16, off - 24);

            // __va_save_area__
            printf("  mov [rbp-%d], rdi\n", off - 24);
            printf("  mov [rbp-%d], rsi\n", off - 32);
            printf("  mov [rbp-%d], rdx\n", off - 40);
            printf("  mov [rbp-%d], rcx\n", off - 48);
            printf("  mov [rbp-%d], r8\n", off - 56);
            printf("  mov [rbp-%d], r9\n", off - 64);
            printf("  movsd [rbp-%d], xmm0\n", off - 72);
            printf("  movsd [rbp-%d], xmm1\n", off - 80);
            printf("  movsd [rbp-%d], xmm2\n", off - 88);
            printf("  movsd [rbp-%d], xmm3\n", off - 96);
            printf("  movsd [rbp-%d], xmm4\n", off - 104);
            printf("  movsd [rbp-%d], xmm5\n", off - 112);
            printf("  movsd [rbp-%d], xmm6\n", off - 120);
            printf("  movsd [rbp-%d], xmm7\n", off - 128);
        }

        gen(current_fn->body);
        // 式の評価結果としてスタックに一つの値が残っている
        pop();

        // エピローグ
        // 最後の式の結果がRAXに残っているのでそれが返り値になる
        printf(".L.return.%s:\n", current_fn->name);
        printf("  leave\n");
        printf("  ret\n");
    }
}