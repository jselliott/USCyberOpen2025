#include <string.h>

// dynamically resolved at runtime
int (*my_strcmp)(const char *, const char *);

int check_flag(const char *arg) {
    return my_strcmp(arg, "SVUSCG{1_sh0uldnt_h4v3_tru5t3d_b1t_t0rr3nt}") == 0;
}
