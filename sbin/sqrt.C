#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc > 1) printf("%.20f\n", sqrt(atof(argv[1])));
}
