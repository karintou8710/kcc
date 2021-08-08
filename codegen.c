#include "9cc.h"

static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static Function *current_fn;

static void assign_lvar_offsets() {
    for (int i=0; funcs[i]; i++) {
        int offset = 0;
        for (LVar *var=funcs[i]->locals; var; var=var->next) {
            offset += 8;
            var->offset -= 8;
        }
        funcs[i]->stack_size = offset;
    }
}

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません");
    }
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void gen(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
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
                printf(".Lend%04d:\n", label_if_count);
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
                printf("  pop %s\n", argreg[i]);
            }
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->fn_name);
            printf("  push rax\n"); // 数合わせ
            return;
        }
            
    }

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
        // 変数26個分の領域を確保する
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", current_fn->stack_size);

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