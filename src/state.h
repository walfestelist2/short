#ifndef STATE_H
#define STATE_H

#include <stdint.h>

struct sh_state {
    const uint8_t *bc;  /* bytecode */
    uint8_t *ip;        /* instruction pointer */

    sh_var lreg;        /* left register */
    sh_var rreg;        /* right register */
};

#endif /* STATE_H */
