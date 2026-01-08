#include <stdio.h>
#include <stdlib.h>
#include "simulator/cli.h"

int main(void) {
    printf("Memory Simulator \nType 'help' for commands\n");
    cli_run();
    return 0;
}
