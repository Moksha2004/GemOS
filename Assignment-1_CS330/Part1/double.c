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
    n = 2 * n;

    if (argc == 2) {
        printf("%lu\n", n);
        return 0;
    } else if (argc > 2) {
        char nstr[15];
        sprintf(nstr, "%lu", n);
        argv[argc - 1] = nstr;
        execv(argv[1], argv + 1);
       // perror("execv");
        return 1;
    }

    return 0;
}

