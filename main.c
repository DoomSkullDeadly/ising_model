#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define mu_b 9.274E-21
#define J 1.
#define B 1.2
#define T 0.69
#define k_b 1.380649E-23
//#define k_b 1.


typedef struct model {
    int size_x;
    int size_y;
    double energy;
    double mag;
    int evolve_steps;
    int step;
    unsigned char *lattice;
} Model;


int get(Model, int, int);

void set(Model, int, int, int);

void print_arr(Model);

double energy(Model);

int nn(int, int);

double norm_mag(Model);

void randomise(Model);

void evolve(Model);

void output(Model);

void video();


int main() {
    Model model = {50, 50};
    model.lattice = (unsigned char*) calloc((int) (model.size_x * model.size_y / sizeof(unsigned char)) + 1, sizeof(unsigned char));
    if (model.lattice == NULL) {
        printf("Error occured!\n");
        exit(0);
    }
    model.evolve_steps = 100;
    randomise(model);
    model.energy = energy(model);
    model.mag = norm_mag(model);
    //print_arr(model);
    printf("E = %g, M = %g\n", model.energy, model.mag);
    evolve(model);
    //print_arr(model);
    model.energy = energy(model);
    model.mag = norm_mag(model);
    printf("E = %g, M = %g\n", model.energy, model.mag);
    video();
    return 0;
}


int get(Model model, int x, int y) {  // fml i hate this shit
    int byte = (int)(((y-1) * model.size_x + x-1) / 8);
    int bit_shift = 7 - (((y-1) * model.size_x + x-1) % 8);
    return (model.lattice[byte] & (1 << bit_shift)) ? 1 : 0;
}


void set(Model model, int x, int y, int bit) {
    int byte = (int)(((y-1) * model.size_x + x-1) / 8);
    int bit_shift = 7 - (((y-1) * model.size_x + x-1) % 8);
    model.lattice[byte] ^= (-bit ^ model.lattice[byte]) & (1 << bit_shift);
}


void print_arr(Model model) {
    for (int i = 0; i < model.size_y; i++) {  // double for loop not necessary but oh well
        for (int j = 0; j < model.size_x; j++) {
            printf("%i ", get(model, j+1, i+1));
        }
        printf("\n");
    }
}


double energy(Model model) {
    double E = 0;
    for (int i = 0; i < model.size_x * model.size_y; i++) {

        int i_x = i % model.size_x + 1;
        int i_y = (int)(i / model.size_x) + 1;

        for (int dx = -1; dx < 2; dx++) {  // faster to test 3 cases in an O(n^2) rather than large N number in O(n)
            for (int dy = -1; dy < 2; dy++) {

                int j_x = ((model.size_x + i_x - 1 + dx) % model.size_x) + 1;  // ensures correct coordinate with periodic boundaries
                int j_y = ((model.size_y + i_y - 1 + dy) % model.size_y) + 1;

                E -= (J / 2) * nn(dx, dy) * (get(model, i_x, i_y) - .5) * (get(model, j_x, j_y) - .5);
            }
        }
        E -= mu_b * B * (get(model, i_x, i_y)-.5);
    }
    return E;
}


int nn(int dx, int dy) {  // really not that necessary as a function but looks cleaner ig
    return abs(dx) + abs(dy) == 1 ? 1 : 0;
}


double norm_mag(Model model) {
    double M = 0;
    for (int i = 0; i < model.size_x * model.size_y; i++) {
        int i_x = i % model.size_x + 1;
        int i_y = (int)(i / model.size_x) + 1;
        M += 2 * (get(model, i_x, i_y)-.5) / (model.size_x * model.size_y);
    }
    return M;
}


void randomise(Model model) {
    printf("Randomising...\n");
    srand(time(NULL));
    int size = (int) (model.size_x * model.size_y / sizeof(unsigned char)) + 1;
    for (int i = 0; i < size; i++) {
        model.lattice[i] = (char)rand() % 128;
    }
    printf("Randomised!\n");
}


void evolve(Model model) {
    int running = 1;
    srand(time(NULL));
    output(model);
    while (running) {
        model.step++;
        for (int i = 0; i < model.size_x * model.size_y; i++) {
            int x = rand() % model.size_x + 1;
            int y = rand() % model.size_y + 1;
            int bit_flip = get(model, x, y);
            set(model, x, y, bit_flip ? 0 : 1);

            double current_E = energy(model);
            double delta_E = current_E - model.energy;

            if (delta_E > 0 &&
                (float) (rand() % 100000) / 100000 > exp(-delta_E / (k_b * T))) { // set back to original
                set(model, x, y, bit_flip);
            } else {
                model.energy = current_E;
            }
        }

        if (model.step ==
            model.evolve_steps) {  // evolve with varying steps and see where it converges, turn this into a for loop lmao
            running = 0;
        }

        output(model);
    }
    printf("Steps taken: %i\n", model.step);
}


void output(Model model) {
    FILE *file;
    char buf[12+(int)(model.step/10)];
    sprintf(buf, "output/%i.txt", model.step);
    file = fopen(buf, "w");

    for (int i = 0; i < model.size_y; i++) {  // double for loop not necessary but oh well
        for (int j = 0; j < model.size_x; j++) {
            fprintf(file, "%i ", get(model, j+1, i+1));
        }
        fprintf(file, "\n");
    }
    fclose(file);
}


void video() {
    system("C:/Users/Student/AppData/Local/Programs/Python/Python39/python.exe ../video.py");
}