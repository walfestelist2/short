#include <string.h>

#include "alloc.h"
#include "gen.h"
#include "lexer.h"
#include "utils.h"

enum sh_reg {
    LREG,
    RREG
};

enum sh_val_t {
    BYTE,
    VAR
};

static inline struct sh_tok peek (struct sh_gen *G) {
    return G->toks[G->toks_pc];
}

static inline struct sh_tok consume (struct sh_gen *G) {
    if (G->toks[G->toks_pc].type == TK_NEWLINE) {
        G->line++;
    }
    return G->toks[G->toks_pc++];
}

static inline struct sh_tok look (struct sh_gen *G) {
    return G->toks[G->toks_pc + 1];
}

const char *sh_lex_to_string (enum sh_tok_type type);

static inline void print_pc (struct sh_gen *G, const char *s) {
    printf("%s\n", s);
    printf("PC: %zu (%s)\n", G->toks_pc, sh_lex_to_string(peek(G).type));
}

static const char *to_string (enum sh_opcode oc) {
    switch (oc) {
        case BC_NOP      : return "NOP";
        case BC_BMOVL    : return "BMOVL";
        case BC_BMOVR    : return "BMOVR";
        case BC_VMOVL    : return "VMOVL";
        case BC_VMOVR    : return "VMOVR";
        case BC_BREDEFL  : return "BREDEFL";
        case BC_BREDEFR  : return "BREDEFR";
        case BC_VREDEFL  : return "VREDEFL";
        case BC_VREDEFR  : return "VREDEFR";
        case BC_BASSIGN  : return "BASSIGN";
        case BC_BPLUS    : return "BPLUS";
        case BC_BMINUS   : return "BMINUS";
        case BC_BMULTIPLY: return "BMULTIPLY";
        case BC_BDIVIDE  : return "BDIVIDE";
        case BC_BREM     : return "BREM";
        case BC_BXOR     : return "BXOR";
        case BC_BAND     : return "BAND";
        case BC_BOR      : return "BOR";
        case BC_BRSHIFT  : return "BRSHIFT";
        case BC_BLSHIFT  : return "BLSHIFT";
        case BC_VASSIGN  : return "VASSIGN";
        case BC_VPLUS    : return "VPLUS";
        case BC_VMINUS   : return "VMINUS";
        case BC_VMULTIPLY: return "VMULTIPLY";
        case BC_VDIVIDE  : return "VDIVIDE";
        case BC_VREM     : return "VREM";
        case BC_VXOR     : return "VXOR";
        case BC_VAND     : return "VAND";
        case BC_VOR      : return "VOR";
        case BC_VRSHIFT  : return "VRSHIFT";
        case BC_VLSHIFT  : return "VLSHIFT";
        case BC_WRITE    : return "WRITE";
        case BC_READ     : return "READ";
        case BC_EXIT     : return "EXIT";
        default          : return "part of a literal";
    }
}

static void print (struct sh_gen *G) {
    for (sh_size i = 0; i < G->bc.count; i++) {
        printf("[%d]: 0x%02X (%s)\n", i, G->bc.bytes[i], to_string(G->bc.bytes[i]));
    }
}

static enum sh_opcode tok_type2bc (enum sh_tok_type tok_type, enum sh_val_t val_type) { 
    if (val_type == BYTE) {
        switch (tok_type) {
            case TK_ASSIGN: return BC_BASSIGN;
            default: break;
        }
    }
    /* if VAR */
    switch (tok_type) {
        case TK_ASSIGN: return BC_VASSIGN;
        default: return 0;
    }
}

static inline int is_newline (enum sh_tok_type t) {
    return t == TK_NEWLINE || t == TK_DOT || t == TK_EOF;
}

static inline int is_assignment (enum sh_tok_type t) {
    return t == TK_VAR || t == TK_BYTE || t == TK_LBRACKET || t == TK_LBRACE;
}

static inline int is_command (enum sh_tok_type t) {
    return t == TK_WRITE || t == TK_READ || t == TK_EXIT;
}

static void init (struct sh_gen *G, struct sh_lexer *L, sh_size bc_cap) {
    struct sh_tok *toks = (struct sh_tok*)L->toks.data;
    struct sh_bc bc;
    bc.bytes = sh_malloc(bc_cap);
    bc.count = 0;
    bc.capacity = bc_cap;

    G->bc = bc;
    G->toks = toks;
    G->toks_pc = 0;
    G->line = 1;
}

static void push (struct sh_gen *G, uint8_t u8) {
    if (G->bc.count + 1 > G->bc.capacity) {
        G->bc.capacity *= 2;
        G->bc.bytes = sh_realloc(G->bc.bytes, G->bc.capacity);
    }
    memcpy(G->bc.bytes + G->bc.count, &u8, 1);
    G->bc.count++;
}

static void pushvar (struct sh_gen *G, sh_var v) {
    sh_size var_size = sizeof(sh_var);
    if (G->bc.count + var_size > G->bc.capacity) {
        sh_assert(G->bc.capacity >= var_size, "byte code capacity should be at least %zu", var_size);
        G->bc.capacity *= 2;
        G->bc.bytes = sh_realloc(G->bc.bytes, G->bc.capacity);
    }
    memcpy(G->bc.bytes + G->bc.count, &v, var_size);
    G->bc.count += var_size;
}

static inline sh_var var_addr (sh_var value) {
    return value * sizeof(sh_var);
}

static inline sh_var byte_addr (sh_var value) {
    return value * sizeof(sh_byte);
}


/* generates the most optimal mov instruction */
static void push_mov (struct sh_gen *G, enum sh_reg r, sh_var value) {
    if (r == LREG) {
        if (value < 256) {
            push(G, BC_BMOVL);
            push(G, value);
            return;
        }

        push(G, BC_VMOVL);
        pushvar(G, value);
        
    } else {
        if (value < 256) {
            push(G, BC_BMOVR);
            push(G, value);
            return;
        }

        push(G, BC_VMOVR);
        pushvar(G, value);
    }
}

static void push_redef (struct sh_gen *G, enum sh_reg r, enum sh_val_t t) {
    if (r == LREG) {
        if (t == BYTE) push(G, BC_BREDEFL);
        else push(G, BC_VREDEFL);
    } else {
        if (t == BYTE) push(G, BC_BREDEFR);
        else push(G, BC_VREDEFR);
    }
}

static void lit (struct sh_gen *G, enum sh_reg r) {
    switch (peek(G).type) {
        case TK_LIT: {
            push_mov(G, r, peek(G).value); 
            consume(G);
        } break;
        case TK_VAR: {
            push_mov(G, r, peek(G).value);
            push_redef(G, r, VAR);
            consume(G);
        } break; 
        case TK_BYTE: {
            push_mov(G, r, peek(G).value);
            push_redef(G, r, BYTE);
            consume(G);
        } break;
        case TK_LBRACKET: {
            consume(G);
            lit(G, r);
            push_redef(G, r, BYTE);
            if (peek(G).type != TK_RBRACKET) {
                shG_error(G, "unclosed bracket");
            }
        } break;
        case TK_LBRACE: {
            consume(G);
            lit(G, r);
            push_redef(G, r, VAR);
            if (peek(G).type != TK_RBRACE) {
                shG_error(G, "unclosed brace");
            }
        } break;
        case TK_LANGLE: {
            consume(G);
            if (peek(G).type != TK_VAR && peek(G).type != TK_BYTE) {
                shG_error(G, "expected variable or byte");
            }
            if (peek(G).type != TK_VAR) push_mov(G, r, peek(G).value);
            else push_mov(G, r, peek(G).value * 8);
            consume(G);
            if (peek(G).type != TK_RANGLE) {
                shG_error(G, "unclosed angle");
            }
        } break;
        default: shG_error(G, "expected literal, got %s", sh_lex_to_string(peek(G).type));
    }
}

static void lvalue (struct sh_gen *G) {

}

static sh_var rvalue (struct sh_gen *G) {
    return 123;
}

static enum sh_tok_type op (struct sh_gen *G) {
    return consume(G).type;
}

static void assignment (struct sh_gen *G, enum sh_val_t t) {
    lit(G, LREG);
    enum sh_tok_type op_ = op(G);
    lit(G, RREG);

    if (t == BYTE) push(G, BC_BASSIGN);
    else push(G, BC_VASSIGN);
}

static void command (struct sh_gen *G) {
    switch (peek(G).type) {
        case TK_WRITE: push(G, BC_WRITE); break;
        case TK_READ: push(G, BC_READ); break;
        case TK_EXIT: push(G, BC_EXIT); break;
        default: shG_error(G, "unknown command");
    }

    consume(G);

    lit(G, LREG);
}

static void skip_newline (struct sh_gen *G) {
    if (peek(G).type != TK_NEWLINE && peek(G).type != TK_DOT && peek(G).type != TK_EOF) {
        shG_error(G, "expected newline");
    }

    consume(G);
}

static void stmt (struct sh_gen *G) {
    if (is_assignment(peek(G).type)) {
        if (peek(G).type == TK_BYTE || peek(G).type == TK_LBRACKET) assignment(G, BYTE);
        else assignment(G, VAR);
    } else if (is_command(peek(G).type)) {
        command(G);
    }
    else {
        shG_error(G, "expected statement");
    }

    skip_newline(G);
}

struct sh_bc sh_gen (struct sh_lexer *L) {
    struct sh_gen G;
    init(&G, L, KiB(64));

    while (peek(&G).type != TK_EOF) {
        stmt(&G);
    }

    print(&G);

    sh_arena_remove(&L->toks);

    return G.bc;
}
