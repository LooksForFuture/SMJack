#include <stdio.h>
#include <stdlib.h>

#include "i_main.h"
#include "r_main.h"
#include "mix_main.h"
#include "ui_main.h"
#include "p_main.h"

void cleanup(void) {
    p_shutdown();
    mix_shutdown();
    r_shutdown();
    i_shutdown();
}

int main(int argc, char** argv) {
    atexit(cleanup);

    //init all systems
    i_init();
    r_init();
    mix_init();
    ui_init();
    p_init();

    p_mainloop();

    return 0;
}
