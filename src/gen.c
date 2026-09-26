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
        case BC_BDEREFL  : return "BDEREFL";
        case BC_BDEREFR  : return "BDEREFR";
        case BC_VDEREFL  : return "VDEREFL";
        case BC_VDEREFR  : return "VDEREFR";
        case BC_SETLBL   : return "SETLBL";
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
        case BC_BCMP     : return "BCMP";
		case BC_JMP      : return "JMP";
		case BC_JE       : return "JE";
		case BC_JNE      : return "JNE";
		case BC_JL       : return "JL";
		case BC_JG       : return "JG";
		case BC_JLE      : return "JLE";
		case BC_JGE      : return "JGE";
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
        case BC_VCMP     : return "VCMP";
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
    return t == TK_NEWLINE || t == TK_DOT;
}

static inline int is_assignment (enum sh_tok_type t) {
    return t == TK_VAR || t == TK_BYTE || t == TK_LBRACKET || t == TK_LBRACE;
}

static inline int is_command (enum sh_tok_type t) {
    return t == TK_WRITE || t == TK_READ || t == TK_EXIT;
}

static inline int is_jump (enum sh_tok_type t) {
	return t == TK_IF || t == TK_JUMP;
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

static void push8 (struct sh_gen *G, uint64_t u64) {
	push(G, (uint8_t)(u64 >> 56));
    push(G, (uint8_t)(u64 >> 48));
    push(G, (uint8_t)(u64 >> 40));
    push(G, (uint8_t)(u64 >> 32));
    push(G, (uint8_t)(u64 >> 24));
    push(G, (uint8_t)(u64 >> 16));
    push(G, (uint8_t)(u64 >> 8));
    push(G, (uint8_t)(u64 & 0xFF));
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
        if (t == BYTE) push(G, BC_BDEREFL);
        else push(G, BC_VDEREFL);
    } else {
        if (t == BYTE) push(G, BC_BDEREFR);
        else push(G, BC_VDEREFR);
    }
}

static void push_op (struct sh_gen *G, enum sh_val_t t, enum sh_tok_type op) {
	if (t == BYTE) {
		switch (op) {
			case TK_ASSIGN:    push(G, BC_BASSIGN); break;
			case TK_PLUS:      push(G, BC_BPLUS); break;
			case TK_MINUS:     push(G, BC_BMINUS); break;
			case TK_STAR:      push(G, BC_BMULTIPLY); break;
			case TK_SLASH:     push(G, BC_BDIVIDE); break;
			case TK_CARET:     push(G, BC_BREM); break;
			case TK_PIPE:      push(G, BC_BXOR); break;
			case TK_AND:       push(G, BC_BAND); break;
			case TK_LSHIFT:    push(G, BC_BLSHIFT); break;
			case TK_RSHIFT:    push(G, BC_BRSHIFT); break;
			case TK_QUESTION:  push(G, BC_BCMP); break;
			default: shG_error(G, "unknown operator");
		}
	} else if (t == VAR) {
		switch (op) {
			case TK_ASSIGN:    push(G, BC_VASSIGN); break;
			case TK_PLUS:      push(G, BC_VPLUS); break;
			case TK_MINUS:     push(G, BC_VMINUS); break;
			case TK_STAR:      push(G, BC_VMULTIPLY); break;
			case TK_SLASH:     push(G, BC_VDIVIDE); break;
			case TK_CARET:     push(G, BC_VREM); break;
			case TK_PIPE:      push(G, BC_VXOR); break;
			case TK_AND:       push(G, BC_VAND); break;
			case TK_LSHIFT:    push(G, BC_VLSHIFT); break;
			case TK_RSHIFT:    push(G, BC_VRSHIFT); break;
			case TK_QUESTION:  push(G, BC_VCMP); break;
			default: shG_error(G, "unknown operator");
		}
	} else {
		shG_error(G, "unknown type"); /* unreachable */
	}
}

static void push_cond_op (struct sh_gen *G, enum sh_tok_type cond_op) {
	switch (cond_op) {
		case TK_EQUAL:     push(G, BC_JE); break;
		case TK_NOT_EQUAL: push(G, BC_JNE); break;
		case TK_LANGLE:    push(G, BC_JL); break;
		case TK_RANGLE:    push(G, BC_JG); break;
		case TK_LE:        push(G, BC_JLE); break;
		case TK_GE:        push(G, BC_JGE); break;
		default: shG_error(G, "unknown conditional operator");
	}
}

static void rvalue (struct sh_gen *G, enum sh_reg r) {
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
            rvalue(G, r);
            push_redef(G, r, BYTE);
            if (peek(G).type != TK_RBRACKET) {
                shG_error(G, "unclosed bracket");
            }
        } break;
        case TK_LBRACE: {
            consume(G);
            rvalue(G, r);
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

static void lvalue (struct sh_gen *G, enum sh_reg r) {
    switch (peek(G).type) {
        case TK_VAR: {
            push_mov(G, r, peek(G).value);
            consume(G);
        } break; 
        case TK_BYTE: {
            push_mov(G, r, peek(G).value);
            consume(G);
        } break;
        case TK_LBRACKET: {
            consume(G);
            lvalue(G, r);
            push_redef(G, r, BYTE);
            if (peek(G).type != TK_RBRACKET) {
                shG_error(G, "unclosed bracket");
            } 
			consume(G);
        } break;
        case TK_LBRACE: {
            consume(G);
            rvalue(G, r);
 			push_redef(G, r, VAR);
            if (peek(G).type != TK_RBRACE) {
                shG_error(G, "unclosed brace");
            }
			consume(G);
        } break;
		case TK_LIT: case TK_LANGLE: {
		   shG_error(G, "expected lvalue");
		} break;
        default: shG_error(G, "expected literal, got %s", sh_lex_to_string(peek(G).type));
    }
}

static void assignment (struct sh_gen *G, enum sh_val_t t) {
    lvalue(G, LREG);
    enum sh_tok_type op = consume(G).type;
    rvalue(G, RREG);

	push_op(G, t, op);

	consume(G);
}

static void command (struct sh_gen *G) {
	if (look(G).type == TK_EOF) shG_error(G, "expected literal");

	int type = consume(G).type;
    rvalue(G, LREG);

    switch (type) {
        case TK_WRITE: push(G, BC_WRITE); break;
        case TK_READ: push(G, BC_READ); break;
        case TK_EXIT: push(G, BC_EXIT); break;
        default: shG_error(G, "unknown command");
    }

    consume(G);
}

static void jump (struct sh_gen *G) {
	if (peek(G).type == TK_IF) {
		if (look(G).type == TK_EOF) shG_error(G, "expected a conditional operator");
		consume(G);

		if (look(G).type != TK_LABEL) shG_error(G, "expected a label");

		int cond_op = peek(G).type;
		consume(G);

		push_mov(G, LREG, peek(G).value);
		push_cond_op(G, cond_op);

		consume(G);
		
		return;
	} 

	consume(G);

	if (peek(G).type != TK_LABEL) shG_error(G, "expected a label");
	
	push_mov(G, LREG, peek(G).value);
	push(G, BC_JMP);

	consume(G);
}

static void label (struct sh_gen *G) {
	push(G, BC_SETLBL);
	push8(G, peek(G).value);

	consume(G);
}

static void skip_newline (struct sh_gen *G) {
    if (!is_newline(peek(G).type)) {
        shG_error(G, "expected newline");
    }

    consume(G);
}

static void skip_empty_stmts (struct sh_gen *G) {
    while (is_newline(peek(G).type)) skip_newline(G);
}

static void stmt (struct sh_gen *G) {
	// errorf("Line %zu (stmt) %s (%d)\n", G->line, sh_lex_to_string(peek(G).type), peek(G).type);

    skip_empty_stmts(G);

	// errorf("Line %zu (stmt after skip_empty_stmts) %s (%d)\n", G->line, sh_lex_to_string(peek(G).type), peek(G).type);

    if (is_assignment(peek(G).type)) {
        if (peek(G).type == TK_BYTE || peek(G).type == TK_LBRACKET) assignment(G, BYTE);
        else assignment(G, VAR);
    } else if (is_command(peek(G).type)) {
        command(G);
    } else if (is_jump(peek(G).type)) {
		jump(G);
	} else if (peek(G).type == TK_LABEL) {
		label(G);
	} else if (peek(G).type == TK_EOF) {
		return;
	} else {
        shG_error(G, "expected statement, got %s", sh_lex_to_string(peek(G).type));
    }
}

struct sh_bc sh_gen (struct sh_lexer *L, int do_print) {
    struct sh_gen G;
    init(&G, L, KiB(64));

    while (peek(&G).type != TK_EOF) {
        stmt(&G);
    }

    if (do_print) print(&G);

    sh_arena_remove(&L->toks);

    return G.bc;
}
