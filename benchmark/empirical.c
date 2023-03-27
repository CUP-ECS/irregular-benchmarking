#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

int empirical_dist(float min, float max, int binCount) {
    float binSize = (max - min)/binCount;

    int bins[binCount+1];

    int index = 0;
    for (float i = min; i <= max; i += binSize) {
        bins[index] = i;
        index += 1;
    }



    return 0;
}

int main(int argc, char *argv[]) {
    empirical_dist(0, 100, 10);
}



