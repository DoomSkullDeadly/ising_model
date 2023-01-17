/*
 * Shitty "documentation":
 * The program will let the user choose parameters for the Ising model. There are no sanitary checks.
 * It will also allow the user to choose how to run the model, either a single run from specified parameters, or many
 * runs while changing a parameter with given increments in a range.
 * I chose to make the program incredibly memory efficient by using a 1D array for the lattice, consisting of char types,
 * which are only 1 byte in size. Furthermore, it then uses each bit in the bytes for individual elements in the lattice
 * and a maximum of 7 bits can be wasted for the entire lattice. 0 represents a negative spin and a 1 represents a positive.
 * In retrospect, it would have been better to simply use a char type for each element as it is probably computationally
 * taxing to access individual bits every time anything needs to be accessed, and hence the speed can be improved.
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#define mu_b 9.274E-21 // constants
//#define mu_b 1.
#define J 1.
#define k_b 1.380649E-23
//#define k_b 1.


typedef struct {  // all parameters that can be changed will be stored in the structure
    int size_x;
    int size_y;
    double energy;
    double energy2;
    double mag;
    int evolve_steps;
    int step;
    int delta_checks;
    double T;
    double B;
    int output;
    int randomise;
    int threads;
    int count;
    pthread_mutex_t setting;
    pthread_mutex_t counting;
    pthread_mutex_t energying;
    unsigned char *lattice;
} Model;


int get(Model*, int, int);

void set(Model*, int, int, int);

void print_arr(Model*);

double energy(Model*);

int nn(int, int);

double norm_mag(Model*);

void randomise(Model*);

void *evolve_loop(void*);

void evolve(Model*);

void output(Model*);

void video();

void from_file(Model*, char[]);

void M_as_T(Model*, double, double, double);

void M_as_B(Model*, double, double, double);

void set_evolve(Model*);


int main() {
    srand(time(NULL)); // random seed is set before anything happens, and only set once
    Model model = {10, 10, 0, 0, 0, 0, 0, 20, 1., 1., 0, 1, 1, 0};  // the model is initialised, apart from the lattice

    int running = 1;
    double lower, upper, increment;

    while (running) {  // the main menu loop
        printf("Model Parameters:\n");
        printf("Size (x*y)\tTemperature\tMagnetic Field\tEvolution Steps\tDelta Checks\n");
        printf("%ix%i\t\t%gK\t\t%gT\t\t%i\t\t%i\n\n", model.size_x, model.size_y, model.T, model.B, model.evolve_steps, model.delta_checks);
        printf("(1) Run Model\n(2) Edit Parameters\n(3) Magnetisation vs Temperature\n(4) Magnetisation vs Magnetic Field\n(5) Lattice From File\n(6) Print to Console\n(7) Create Video\n(0) Exit\n");
        int input;
        scanf("%i", &input);

        switch (input) { // switches between user inputs for the menu
            default:
                running = 0;
                break;

            case 1: // before running the model lattice is initialised, and randomised if necessary
                model.lattice = (unsigned char*) calloc((int) (model.size_x * model.size_y / sizeof(unsigned char)) + 1, sizeof(unsigned char));
                if (model.lattice == NULL) {
                    printf("Error occured!\n");
                    exit(0);
                }
                if (model.randomise) {
                    randomise(&model);
                }
                set_evolve(&model);
                break;

            case 2: // another menu to let user change any model parameter
                printf("Edit Parameters:\n");
                int edit = 1;
                while (edit) {
                    printf("(1) Size:\t%ix%i\n(2) Temperature:\t%gK\n(3) Magnetic Field:\t%gT\n(4) Evolution Steps:\t%i\n(5) Delta Checks:\t%i\n(6) Current Step:\t%i\n(7) Step-by-Step File Output:\t%i\n(8) Randomise:\t%i\n(9) Threads (use with caution):\t%i\n(0) Exit\n", model.size_x, model.size_y, model.T, model.B, model.evolve_steps, model.delta_checks, model.step, model.output, model.randomise, model.threads);
                    int param;
                    scanf("%i", &param);

                    switch (param) { // switch parameters to new user inputs
                        default:
                            edit = 0;
                            break;

                        case 1: // will reallocate memory for the model
                            printf("Enter new x y values:\n");
                            scanf("%i %i", &model.size_x, &model.size_y);
                            model.lattice = (unsigned char*) realloc(model.lattice, (int) (model.size_x * model.size_y / sizeof(unsigned char)) + 1);
                            if (model.lattice == NULL) {
                                printf("Error occured!\n");
                                exit(0);
                            }
                            break;

                        case 2:
                            printf("Enter new temperature:\n");
                            scanf("%lf", &model.T);
                            break;

                        case 3:
                            printf("Enter new magnetic field strength:\n");
                            scanf("%lf", &model.B);
                            break;

                        case 4:
                            printf("Enter steps to evolve (0 for automatic):\n");
                            scanf("%i", &model.evolve_steps);
                            break;

                        case 5:
                            printf("Enter number of delta checks (recommended 20):\n");
                            scanf("%i", &model.delta_checks);
                            break;

                        case 6:
                            printf("Manually set current step in evolution:\n");
                            scanf("%i", &model.step);
                            break;

                        case 7:
                            printf("Enable model evolution outputs (0/1):\n");
                            scanf("%i", &model.output);
                            break;

                        case 8:
                            printf("Model randomisation before running (0/1):\n");
                            scanf("%i", &model.randomise);
                            break;

                        case 9:
                            printf("Enter number of threads for program to use:\n");
                            scanf("%i", &model.threads);
                            break;
                    }
                }
                break;

            case 3: // Take user input for lower and upper bounds, and the increment, and calls function to do it
                printf("Enter lower bound, upper bound, increment of T:\n");
                scanf("%lf %lf %lf", &lower, &upper, &increment);
                M_as_T(&model, lower, upper, increment);
                break;

            case 4:
                printf("Enter lower bound, upper bound, increment of B:\n");
                scanf("%lf %lf %lf", &lower, &upper, &increment);
                M_as_B(&model, lower, upper, increment);
                break;

            case 5:
                printf("Enter file name to load from:\n");
                char name[32];
                scanf("%s", &name);
                FILE *file;
                file = fopen(name, "r");
                int x=0, y=0;
                char ch = getc(file);
                while (ch != EOF) { // loops through the file to get lattice size
                    if (ch == '\n') {
                        y += 1;
                    }
                    else {
                        x += 1;
                    }
                    ch = getc(file);
                }
                fclose(file);
                model.size_y = y;
                model.size_x = x / (y * 2); // lattice size is set to new from file and memory is reallocated
                model.lattice = (unsigned char*) realloc(model.lattice, (int) (model.size_x * model.size_y / sizeof(unsigned char)) + 1);
                if (model.lattice == NULL) {
                    printf("Error occured!\n");
                    exit(0);
                }
                from_file(&model, name); // finally, the file will be read again in the function to update lattice
                break;

            case 6:
                print_arr(&model);
                model.energy = energy(&model);
                model.mag = norm_mag(&model);
                printf("E = %g, M = %g\n", model.energy, model.mag);
                break;

            case 7:
                video(); // optional video if files have been output from running the model
                break;
        }
    }

    return 0;
}


int get(Model *model, int x, int y) {  // fml i hate this shit
    int byte = (int)(((y-1) * model->size_x + x-1) / 8); // finds the correct byte, as each array element is only 1 byte in size
    int bit_shift = 7 - (((y-1) * model->size_x + x-1) % 8); // finds appropriate bit shift to access the correct bit
    return (model->lattice[byte] & (1 << bit_shift)) ? 1 : 0; // will return a 1 or 0
}


void set(Model *model, int x, int y, int bit) {
    int byte = (int)(((y-1) * model->size_x + x-1) / 8);
    int bit_shift = 7 - (((y-1) * model->size_x + x-1) % 8);
    model->lattice[byte] ^= (-bit ^ model->lattice[byte]) & (1 << bit_shift); // performs XOR to keep original bit if not changed
}


void print_arr(Model *model) {
    for (int i = 0; i < model->size_y; i++) {  // double for loop not necessary but oh well
        for (int j = 0; j < model->size_x; j++) {
            printf("%c ", get(model, j+1, i+1) ? '+': '-');
        }
        printf("\n");
    }
}


double energy(Model *model) {
    double E = 0;
    for (int i = 0; i < model->size_x * model->size_y; i++) {

        int i_x = i % model->size_x + 1;
        int i_y = (int)(i / model->size_x) + 1;

        for (int dx = -1; dx < 2; dx++) {  // faster to test 3 cases in an O(n^2) rather than large N number in O(n)
            for (int dy = -1; dy < 2; dy++) {

                int j_x = ((model->size_x + i_x - 1 + dx) % model->size_x) + 1;  // ensures correct coordinate with periodic boundaries
                int j_y = ((model->size_y + i_y - 1 + dy) % model->size_y) + 1;

                E -= (J / 2) * nn(dx, dy) * (get(model, i_x, i_y) - .5) * (get(model, j_x, j_y) - .5);
            }
        }
        E -= mu_b * model->B * (get(model, i_x, i_y)-.5);
    }
    return E;
}


int nn(int dx, int dy) {  // really not that necessary as a function but looks cleaner ig
    return abs(dx) + abs(dy) == 1 ? 1 : 0;
}


double norm_mag(Model *model) {
    double M = 0;
    for (int i = 0; i < model->size_x * model->size_y; i++) {
        int i_x = i % model->size_x + 1;
        int i_y = (int)(i / model->size_x) + 1;
        M += 2 * (get(model, i_x, i_y)-.5) / (model->size_x * model->size_y);
    }
    return M;
}


void randomise(Model *model) { // since only individual bits are accessed, every byte can be randomised instead of setting bits
    int size = (int) (model->size_x * model->size_y / sizeof(unsigned char)) + 1;
    for (int i = 0; i < size; i++) {
        model->lattice[i] = (unsigned char)rand() % 256;
    }
}


void *evolve_loop(void *m) {
    Model *model = (Model *)m;
    while (model->count > 0) {

        pthread_mutex_lock(&model->counting);
        model->count -= 1;
        pthread_mutex_unlock(&model->counting);

//        printf("%i\n", model->count);
        int x = rand() % model->size_x + 1;
        int y = rand() % model->size_y + 1;
        int bit_flip = get(model, x, y);

        pthread_mutex_lock(&model->setting);
        set(model, x, y, bit_flip ? 0 : 1);

        double current_E = energy(model);
        double delta_E = current_E - model->energy;
        pthread_mutex_unlock(&model->setting);

        if (delta_E > 0 && (float) (rand() % 100000) / 100000 > exp(-delta_E / (k_b * model->T))) { // set back to original

            pthread_mutex_lock(&model->setting);
            set(model, x, y, bit_flip);
            pthread_mutex_unlock(&model->setting);

        }
        else {

            pthread_mutex_lock(&model->energying);
            model->energy = current_E;
            pthread_mutex_unlock(&model->energying);

        }
    }
    pthread_exit(NULL);
    return NULL;
}


void evolve(Model *model) {
    int running = 1;
    if (model->output) {
        output(model);
    }
    double d_E[model->delta_checks];
    while (running) {
        model->step++;
        pthread_t tid[model->threads];
        model->count = model->size_x * model->size_y;
        for (int t = 0; t < model->threads; t++) {
            pthread_create(&tid[t], NULL, evolve_loop, model);
        }
        for (int t = 0; t < model->threads; t++) {
            pthread_join(tid[t], NULL);
        }

        if (model->step == // if the model is set to run for a specified amount of steps it'll stop when those are reached
            model->evolve_steps && model->evolve_steps) {
            running = 0;
        }
        else if (!model->evolve_steps) {
            d_E[model->step % model->delta_checks] = model->energy; // sets current pos to current energy
            if (fabs(d_E[(model->step+1) % model->delta_checks] - model->energy) < fabs(model->energy * 0.001)) {
                running = 0; // if the change in energy over the last n steps is less than 0.1% of the current energy it will stop running the model. This seems to work reliably.
            }
        }
        if (model->output) {
            output(model);
        }
    }
    model->step = 0;
}


void output(Model *model) {
    FILE *file;
    char buf[12+(int)(model->step/10)]; // not necessary to do maths here, can just set to a reasonable number, but eh
    sprintf(buf, "output/%i.txt", model->step); // too lazy to check if directory exists and create one if not
    file = fopen(buf, "w");

    for (int i = 0; i < model->size_y; i++) {  // double for loop not necessary but oh well
        for (int j = 0; j < model->size_x; j++) {
            fprintf(file, "%i ", get(model, j+1, i+1));
        }
        fprintf(file, "\n");
    }
    fclose(file);
}


void video() {
    system("python ../video.py"); // may need to use different command depending on system
}


void from_file(Model *model, char name[32]) {
    FILE *file;
    file = fopen(name, "r");

    int bit;
    for (int i = 0; i < model->size_y; i++) {
        for (int j = 0; j < model->size_x; j++) {
            fscanf(file, "%i ", &bit);
            if (bit) {
                set(model, j+1, i+1, bit);
            }
        }
    }
    fclose(file);
}


void M_as_T(Model *model, double lower, double upper, double increment) {
    for (int num = 0; num < 1000; ++num) { // will do 1000 iterations for each step in the range
        printf("Run: %i\n", num+1);
        model->T = lower; // values reset
        model->step = 0;
        int steps = (int) ((upper - lower) / increment);
        double mags[steps];

        for (int i = 0; i <= steps; ++i) { // could also use i+=increment but too lazy to change
            model->T = lower + (i * increment);
            randomise(model);
            evolve(model);
            mags[i] = norm_mag(model);
        }

        FILE *file;
        char buf[10 + (int) ((num+1) / 10)];
        sprintf(buf, "mags/%i.txt", num+1); // again directory needs to be present
        file = fopen(buf, "w");
        fprintf(file, "T\tM\n");
        for (int i = 0; i <= steps; ++i) {
            fprintf(file, "%g\t%g\n", lower + (i * increment), mags[i]);
        }
        fclose(file);
    }
}


void M_as_B(Model *model, double lower, double upper, double increment) { // same as M_as_T but changing B
    for (int num = 100; num < 1000; ++num) {
        printf("Run: %i\n", num+1);
        model->B = lower;
        model->step = 0;
        int steps = (int) ((upper - lower) / increment);
        double mags[steps+1];

        for (int i = 0; i <= steps; ++i) {
            model->B = lower + (i * increment);
            randomise(model);
            evolve(model);
            mags[i] = norm_mag(model);
        }

        FILE *file;
        char buf[10 + (int) ((num+1) / 10)];
        sprintf(buf, "mags/%i.txt", num+1);
        file = fopen(buf, "w");
        fprintf(file, "B\tM\n");
        for (int i = 0; i <= steps; ++i) {
            fprintf(file, "%g\t%g\n", lower + (i * increment), mags[i]);
        }
        fclose(file);
    }
}


void set_evolve(Model *model) { // performs evolution once
    if (model->randomise) {
        randomise(model);
    }
    model->energy = energy(model);
    model->mag = norm_mag(model);
//    print_arr(model);
    printf("E = %g, M = %g\n", model->energy, model->mag);
    evolve(model);
//    print_arr(model);
    model->energy = energy(model);
    model->mag = norm_mag(model);
    printf("E = %g, M = %g\n", model->energy, model->mag);
}