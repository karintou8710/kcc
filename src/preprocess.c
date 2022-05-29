#include "kcc.h"

Token *preprocess(Token *tok) {
    Token *bef, *head = memory_alloc(sizeof(Token));
    head->next = tok;
    bef = head;

    Token *entry = head->next;
    while (entry) {
        if (entry->kind == TK_INCLUDE) {
            if (entry->is_standard) {
                // 標準ヘッダー
                bef->next = entry->next;  // includeトークンを読み飛ばす
                entry = bef;
            } else {
                // 現在のディレクトリ
                char *include_input = read_file(entry->str);
                Token *include_token = tokenize(include_input);
                include_token = preprocess(include_token);
                if (include_token->kind == TK_EOF) {
                    // 空ファイルの場合
                    bef->next = entry->next;  // includeトークンを読み飛ばす
                    entry = bef;
                } else {
                    bef->next = include_token;
                    for (Token *t = include_token; t->next; t = t->next) {
                        if (t->next->kind == TK_EOF) {
                            t->next = entry->next;
                            break;
                        }
                    }
                }
            }
        }
        bef = entry;
        entry = entry->next;
    }

    return head->next;
}
