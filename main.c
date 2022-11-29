#include <stdio.h>


int get(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1], int, int);

void set(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1], int, int, int);

void print_arr(int size_x, int size_y, unsigned char arr[(int)((size_x * size_y - 1) / 8)+1]);

struct model {
    int size_x;
    int size_y;
    double energy;
    double mag;
    unsigned char lattice[];
};


// basically a 1 bit int
int main() {

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
//    for (int i = 0; i < size_y * 8; i++) {
//        if (i%8 == 0) {
//            printf("\n");
//        }
//        printf("%i ", (arr[(int)(i/8)] & (1 << (7-(i%8))) ? 1 : 0));
//    }
//    printf("\n");
//
//    for (int i = 0; i < size_y * size_x; i++) {
//        if (i%size_x == 0) {
//            printf("\n");
//        }
//        printf("%i ", (arr[(int)(i/8)] & (1 << (7-(i%8))) ? 1 : 0));
//    }
//    printf("\n");
//    printf("\n");

    for (int i = 0; i < size_y; i++) {
        for (int j = 0; j < size_x; j++) {
            printf("%i ", get(size_x, size_y, arr, j+1, i+1));
        }
        printf("\n");
    }
}
