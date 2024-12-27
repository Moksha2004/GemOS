#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Unable to execute\n");
        return 1;
    }

    char *lastarg = argv[argc - 1];
    unsigned long n = atof(lastarg);
    n = n * n;

    if (argc == 2) {
        printf("%lu\n", n);
        return 0;
    } else if (argc > 2) {
        char nstr[15];
        sprintf(nstr, "%lu", n);
        argv[argc - 1] = nstr;

        execv(argv[1], argv + 1);

        // If execv fails, it will not return, so only this block will be executed in case of failure
        perror("execv");
        return 1;
    }

    return 0;
}

