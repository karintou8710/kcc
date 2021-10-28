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

typedef enum
{
    REG_RAX,
    REG_RDI,
} RegKind;

static char *get_argreg(int index, int size)
{
    switch (size)
    {
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

static int size_to_regindex(int size)
{
    switch (size)
    {
    case 8:
        return 0; // 64bit
        break;
    case 4:
        return 1; // 32bit
        break;
    case 2:
        return 2; // 16bit
        break;
    case 1:
        return 3; // 8bit
        break;
    }

    error("サポートしてないレジスターのサイズです");
}

static char *proper_register(Type *ty, RegKind kind)
{
    int size = ty->size;
    if (ty->kind == TYPE_ARRAY)
    {
        while (ty->ptr_to)
            ty = ty->ptr_to;
        size = ty->size;
    }

    int index = size_to_regindex(size);

    switch (kind)
    {
    case REG_RAX:
        return raxreg[index];
    case REG_RDI:
        return rdireg[index];
    }

    error("サポートしていないレジスターです");
}

static void push()
{
    printf("  push rax\n");
}

static void push_rdi() {
    printf("  push rdi\n");
}

static void push_num(int num) {
    printf("  push %d\n", num);
}

static void pop()
{
    printf("  pop rax\n");
}

static void pop_rdi()
{
    printf("  pop rdi\n");
}

static void assign_lvar_offsets()
{
    for (int i = 0; funcs[i]; i++)
    {
        funcs[i]->stack_size = funcs[i]->locals->offset;
    }
}

// 左辺値は変数である必要がある
// ローカル変数のアドレスを生成
static void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
    {
        error("代入の左辺値が変数ではありません");
    }
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->lvar->offset);
    push();
}

// ポインター変数への代入への対応
static void gen_addr(Node *node)
{
    if (node->kind == ND_DEREF)
    {
        gen(node->lhs);
        return;
    }
    else if (node->kind == ND_LVAR)
    {
        gen_lval(node);
        return;
    }
    else if (node->kind == ND_ADD || node->kind == ND_SUB)
    {
        gen(node);
        return;
    }

    error("左辺値がポインターまたは変数ではありません");
}

static void load(Type *ty)
{
    if (ty->kind == TYPE_ARRAY)
    {
        // アドレスのまま読みこむようにする
        return;
    }

    printf("  mov %s, [rax]\n", proper_register(ty, REG_RAX));
}

static void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        push_num(node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        pop();
        add_type(node);
        load(node->type);
        push();
        return;
    case ND_ADDR:
        gen_lval(node->lhs);
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
        pop();
        printf("  jmp .L.return.%s\n", current_fn->name);
        // returnは終了なので数合わせなし
        return;
    case ND_IF:
        label_if_count++;
        gen(node->cond);
        pop();
        printf("  cmp rax, 0\n");
        if (node->els)
        {
            printf("  je  .Lelse%04d\n", label_if_count);
            gen(node->then);
            printf("  jmp .Lend%04d\n", label_if_count);
            printf(".Lelse%04d:\n", label_if_count);
            gen(node->els);
            printf(".Lend%04d:\n", label_if_count);
        }
        else
        {
            printf("  je  .Lend%04d\n", label_if_count);
            gen(node->then);
            pop(); // 数合わせ
            printf(".Lend%04d:\n", label_if_count);
            push(); // 数合わせ
        }

        label_if_count--;
        return;
    case ND_WHILE:
        label_loop_count++;
        printf(".Lbegin%04d:\n", label_loop_count);
        gen(node->cond);
        pop();
        printf("  cmp rax, 0\n");
        printf("  je  .Lend%04d\n", label_loop_count);
        gen(node->body);
        printf("  jmp .Lbegin%04d\n", label_loop_count);
        printf(".Lend%04d:\n", label_loop_count);
        label_loop_count--;
        return;
    case ND_FOR:
        label_loop_count++;
        if (node->init)
        {
            gen(node->init);
        }
        printf(".Lbegin%04d:\n", label_loop_count);
        if (node->cond)
        {
            gen(node->cond);
            pop();
            printf("  cmp rax, 0\n");
            printf("  je  .Lend%04d\n", label_loop_count);
        }
        gen(node->body);
        if (node->inc)
        {
            gen(node->inc);
        }
        printf("  jmp .Lbegin%04d\n", label_loop_count);
        printf(".Lend%04d:\n", label_loop_count);
        label_loop_count--;
        return;
    case ND_BLOCK:
        for (int i = 0; i < node->stmts->len; i++)
        {
            gen(node->stmts->body[i]);
            pop();
        }
        push(); // 数合わせ
        return;
    case ND_CALL:
    {
        int nargs = node->args->len;
        for (int i = 0; i < nargs; i++)
        {
            gen(node->args->body[i]);
        }
        for (int i = nargs - 1; i >= 0; i--)
        {
            LVar *l = node->args->body[i];
            printf("  pop %s\n", argreg64[i]);
        }
        printf("  mov rax, 0\n");
        printf("  call %s\n", node->fn_name);
        push(); // 数合わせ
        return;
    }
    }

    // 主に演算のATSで読みこまれる
    gen(node->lhs);
    gen(node->rhs);

    pop_rdi();
    pop();

    switch (node->kind)
    {
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
    }

    push();
}

void codegen()
{
    // 各関数のoffsetを計算
    assign_lvar_offsets();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");

    // 先頭の式から順にコード生成
    for (int i = 0; funcs[i]; i++)
    {
        current_fn = funcs[i];
        printf(".globl %s\n", current_fn->name);
        printf("%s:\n", current_fn->name);

        // プロローグ
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", current_fn->stack_size);

        int j = 0;
        for (LVar *var = current_fn->params; var; var = var->next)
        {
            printf("  mov rax, rbp\n");
            printf("  sub rax, %d\n", var->offset);
            printf("  mov [rax], %s\n", get_argreg(j++, var->type->size));
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