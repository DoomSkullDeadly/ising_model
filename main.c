#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define mu_b 9.274E-21
#define J 1.
#define B 0


int get(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1], int, int);

void set(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1], int, int, int);

void print_arr(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1]);

double energy(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1]);

int nn(int, int);

double norm_mag(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1]);

void randomise(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1]);


typedef struct model {
    int size_x;
    int size_y;
    double energy;
    double mag;
    unsigned char lattice[];
} Model;


int main() {
    int size_x = 10;
    int size_y = 10;
    unsigned char *arr;
    arr = (unsigned char*) calloc(size_x * size_y / sizeof(unsigned char), size_x * size_y);
    randomise(size_x, size_y, arr);
    print_arr(size_x, size_y, arr);
    return 0;
}


int get(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1], int x, int y) {  // fml i hate this shit
    int byte = (int)(((y-1) * size_x + x-1) / 8);
    int bit_shift = 7 - (((y-1) * size_x + x-1) % 8);
    return (arr[byte] & (1 << bit_shift)) ? 1 : 0;
}


void set(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1], int x, int y, int bit) {
    int byte = (int)(((y-1) * size_x + x-1) / 8);
    int bit_shift = 7 - (((y-1) * size_x + x-1) % 8);
    arr[byte] ^= (-bit ^ arr[byte]) & (1 << bit_shift);
}


void print_arr(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1]) {
    for (int i = 0; i < size_y; i++) {
        for (int j = 0; j < size_x; j++) {
            printf("%i ", get(size_x, size_y, arr, j+1, i+1));
        }
        printf("\n");
    }
}


double energy(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1]) {
    double E = 0;
    for (int i = 0; i < size_x * size_y; i++) {

        int i_x = i % size_x + 1;
        int i_y = (int)(i / size_x) + 1;

        for (int dx = -1; dx < 2; dx++) {  // faster to test 3 cases in an O(n^2) rather than large N number in O(n)
            for (int dy = -1; dy < 2; dy++) {

                int j_x = ((size_x + i_x - 1 + dx) % size_x) + 1;  // ensures correct coordinate with periodic boundaries
                int j_y = ((size_y + i_y - 1 + dy) % size_y) + 1;

                E -= J / 2 * nn(dx, dy) * (get(size_x, size_y, arr, i_x, i_y) - .5) * (get(size_x, size_y, arr, j_x, j_y) - .5);
            }
        }
        E -= mu_b * B * (get(size_x, size_y, arr, i_x, i_y)-.5);
    }
    return E;
}


int nn(int dx, int dy) {
    return abs(dx) + abs(dy) == 1 ? 1 : 0;
}


double norm_mag(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1]) {
    double M = 0;
    for (int i = 0; i < size_x * size_y; i++) {
        int i_x = i % size_x + 1;
        int i_y = (int)(i / size_x) + 1;
        M += 2 * (get(size_x, size_y, arr, i_x, i_y)-.5) / (size_x * size_y);
    }
    return M;
}


void randomise(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1]) {
    srand(time(NULL));
    for (int i = 0; i < size_x * size_y; i++) {
        int x = i % 8 + 1;
        int y = (int)(i / 8) + 1;
        int bit = rand() % 2;
        if (bit) {
            set(size_x, size_y, arr, x, y, bit);
        }
    }
}