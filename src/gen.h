#ifndef GEN_H
#define GEN_H

#include <stdint.h>

#include "lexer.h"
#include "utils.h"

enum sh_opcode {
    BC_NOP,         /* BC - bytecode */

    BC_BMOVL,        /* moving a byte to the left register */
    BC_BMOVR,        /*               to the right         */
    BC_VMOVL,        /*          var                       */
    BC_VMOVR,        /*          var  to the right         */

    BC_BREDEFL,     /* redeference a byte in the left register */
    BC_BREDEFR,     /*                    in the right         */
    BC_VREDEFL,     /*               var                       */
    BC_VREDEFR,     /*               var  in the right         */

    BC_BASSIGN,     /* mov byte [lreg], rreg */
    BC_BPLUS,       /* add byte [rleg], rreg */
    BC_BMINUS,      /* sub byte [rleg], rreg */
    BC_BMULTIPLY,   /* mul byte [rleg], rreg */
    BC_BDIVIDE,     /* div byte [rleg], rreg */
    BC_BREM,        /* rem byte [rleg], rreg */
    BC_BXOR,        /* xor byte [rleg], rreg */
    BC_BAND,        /* and byte [rleg], rreg */
    BC_BOR,         /*  or byte [rleg], rreg */
    BC_BRSHIFT,     /* shr byte [rleg], rreg */
    BC_BLSHIFT,     /* shl byte [rleg], rreg */

    BC_VASSIGN,     /* mov var [lreg], rreg */
    BC_VPLUS,       /* add var [rleg], rreg */
    BC_VMINUS,      /* sub var [rleg], rreg */
    BC_VMULTIPLY,   /* mul var [rleg], rreg */
    BC_VDIVIDE,     /* div var [rleg], rreg */
    BC_VREM,        /* rem var [rleg], rreg */
    BC_VXOR,        /* xor var [rleg], rreg */
    BC_VAND,        /* and var [rleg], rreg */
    BC_VOR,         /*  or var [rleg], rreg */
    BC_VRSHIFT,     /* shr var [rleg], rreg */
    BC_VLSHIFT,     /* shl var [rleg], rreg */

    BC_WRITE,
    BC_READ,
    BC_EXIT
};

struct sh_bc /* byte code */ {
    uint8_t *bytes;
    sh_size count;
    sh_size capacity;
};

struct sh_gen {
    struct sh_bc bc;
    struct sh_tok *toks;    /* tokens list */
    sh_size toks_pc;        /* current token */
    sh_size line;           /* line we're parsing */
};

struct sh_bc sh_gen (struct sh_lexer *L);

#endif /* GEN_H */
