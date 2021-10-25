#include "9cc.h"

/*
 * ASTからスタックマシンを使ってアセンブリを生成する
 * 
 * スタックマシンの使用
 * 
 * 全てのノードは必ず一つだけの要素が残るようにpushする
 */

static void gen(Node *node);

static char *argreg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static char *argreg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static char *argreg16[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static char *argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *raxreg[] = {"rax", "eax", "ax", "al"};  // size: 8, 4, 2, 1
static char *rdireg[] = {"rdi", "edi", "di", "dil"}; // size: 8, 4, 2, 1
static Function *current_fn;

static char *get_argreg(int index, int size) {
    switch (size) {
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
    
    error("サポートしてない引数です。");
}

static char *s_to_rax(int size) {
    switch (size) {
        case 8:
            return raxreg[0]; // rax
            break;
        case 4:
            return raxreg[1]; // eax
            break;
        case 2:
            return raxreg[2]; // ax
            break;
        case 1:
            return raxreg[3]; // al
            break;
    }
    
    error("サポートしてないレジスターのサイズです");
}

static char *s_to_rdi(int size) {
    switch (size) {
        case 8:
            return rdireg[0]; // rdi
            break;
        case 4:
            return rdireg[1]; // edi
            break;
        case 2:
            return rdireg[2]; // di
            break;
        case 1:
            return rdireg[3]; // dil
            break;
    }
    
    error("サポートしてないレジスターのサイズです");
}

static void assign_lvar_offsets() {
    for (int i=0; funcs[i]; i++) {
        funcs[i]->stack_size = funcs[i]->locals->offset;
    }
}

// 左辺値は変数である必要がある
// ローカル変数のアドレスを生成
static void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません");
    }
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->lvar->offset);
    printf("  push rax\n");
}

// ポインター変数への代入への対応
static void gen_addr(Node *node) {
    if (node->kind == ND_DEREF) {
        gen_addr(node->lhs);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    } else if (node->kind == ND_LVAR) {
        gen_lval(node);
        return;
    }

    error("左辺値がポインターまたは変数ではありません");
}

static void gen(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov %s, [rax]\n", s_to_rax(node->lvar->type->size));
            printf("  push rax\n");
            return;
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n"); // pointerなのでrax
            printf("  push rax\n");
            return;
        case ND_ASSIGN:
            gen_addr(node->lhs);
            gen(node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], %s\n", s_to_rdi(node->lhs->lvar->type->size));
            printf("  push rdi\n");
            return;
        case ND_RETURN:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  jmp .L.return.%s\n", current_fn->name);
            // returnは終了なので数合わせなし
            return;
        case ND_IF:
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            if (node->els) {
                printf("  je  .Lelse%04d\n", label_if_count);
                gen(node->then);
                printf("  jmp .Lend%04d\n", label_if_count);
                printf(".Lelse%04d:\n", label_if_count);
                gen(node->els);
                printf(".Lend%04d:\n", label_if_count);
            } else {
                printf("  je  .Lend%04d\n", label_if_count);
                gen(node->then);
                printf("  pop rax\n"); // 数合わせ
                printf(".Lend%04d:\n", label_if_count);
                printf("push 0\n"); // 数合わせ
            }
            
            label_if_count++;
            return;
        case ND_WHILE:
            printf(".Lbegin%04d:\n", label_loop_count);
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%04d\n", label_loop_count);
            gen(node->body);
            printf("  jmp .Lbegin%04d\n", label_loop_count);
            printf(".Lend%04d:\n", label_loop_count);
            label_loop_count++;
            return;
        case ND_FOR:
            if (node->init) {
                gen(node->init);
            }
            printf(".Lbegin%04d:\n", label_loop_count);
            if (node->cond) {
                gen(node->cond);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je  .Lend%04d\n", label_loop_count);
            }
            gen(node->body);
            if (node->inc) {
                gen(node->inc);
            }
            printf("  jmp .Lbegin%04d\n", label_loop_count);
            printf(".Lend%04d:\n", label_loop_count);
            return;
        case ND_BLOCK:
            for (int i=0;i<node->stmts->len;i++) {
                gen(node->stmts->body[i]);
                printf("  pop rax\n");
            }
            printf("  push 0\n"); // 数合わせ
            return;
        case ND_CALL: {
            int nargs = node->args->len;
            for (int i=0;i<nargs;i++) {
                gen(node->args->body[i]);
            }
            for (int i=nargs-1;i>=0;i--) {
                LVar *l = node->args->body[i];
                printf("  pop %s\n", argreg64[i]);
            }
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->fn_name);
            printf("  push rax\n"); // 数合わせ
            return;
        }
            
    }

    // 主に演算のATSで読みこまれる
    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

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
    }

    printf("  push rax\n");
}

void codegen() {
    // 各関数のoffsetを計算
    assign_lvar_offsets();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    
    // 先頭の式から順にコード生成
    for (int i = 0; funcs[i]; i++) {
        current_fn = funcs[i];
        printf(".globl %s\n", current_fn->name);
        printf("%s:\n", current_fn->name);

        // プロローグ
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", current_fn->stack_size);

        int j=0;
        for (LVar *var=current_fn->params;var;var=var->next) {
            printf("  mov rax, rbp\n");
            printf("  sub rax, %d\n", var->offset);
            printf("  mov [rax], %s\n", get_argreg(j++, var->type->size));
        }

        gen(current_fn->body);
        // 式の評価結果としてスタックに一つの値が残っている
        printf("  pop rax\n");

        // エピローグ
        // 最後の式の結果がRAXに残っているのでそれが返り値になる
        printf(".L.return.%s:\n", current_fn->name);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
}