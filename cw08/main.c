#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAGIC_NUMBER "P2"
#define WHITESPACES " \f\n\r\t\v"
#define COMMENT_INDICATOR '#'
#define COMMENT_RELACER ' '
#define OUTPUT_FILE_MAX_LINE_LEN 70

#define P(x) printf("%s: %d\n", #x, x);

#define skipcomment_getline(line, len, in_file)             \
    {                                                       \
        do {                                                \
            getline(&line, &len, in_file);                  \
        } while (len != 0 && line[0] != COMMENT_INDICATOR); \
    }

typedef struct {
    size_t width;
    size_t height;
    uint8_t max_color;
    uint8_t* pixels;
} ascii_pgm;

ascii_pgm in_bitmap;
ascii_pgm out_bitmap;

typedef struct {
    size_t from;
    size_t to;
} thread_args;

long get_timestamp() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * 1000000 + spec.tv_nsec / 1000;
}

void* convert_image_thread_numbers(void* args) {
    long t1 = get_timestamp();
    thread_args* params = args;
    for (size_t i = 0; i < in_bitmap.width * in_bitmap.height; ++i) {
        if (in_bitmap.pixels[i] >= params->from &&
            in_bitmap.pixels[i] < params->to) {
            out_bitmap.pixels[i] = in_bitmap.max_color - in_bitmap.pixels[i];
        }
    }
    long t2 = get_timestamp();
    long* t = malloc(sizeof(long));
    *t = t2 - t1;
    return t;
}

void* convert_image_thread_block(void* args) {
    long t1 = get_timestamp();
    thread_args* params = args;
    for (size_t i = 0; i < out_bitmap.height; ++i) {
        for (size_t j = params->from; j < params->to; ++j) {
            out_bitmap.pixels[i * out_bitmap.width + j] =
                in_bitmap.max_color -
                in_bitmap.pixels[i * out_bitmap.width + j];
        }
    }
    long t2 = get_timestamp();
    long* t = malloc(sizeof(long));
    *t = t2 - t1;
    return t;
}

int main(int argc, char* argv[]) {
    if (argc < 4 + 1) {
        perror("Expected 4 arguments!\n");
        exit(1);
    }
    int thread_count = strtol(argv[1], NULL, 10);
    bool block;
    if (strcmp("numbers", argv[2]) == 0) {
        block = false;
    } else if (strcmp("block", argv[2]) == 0) {
        block = true;
    } else {
        perror("Expected 'numbers' or 'block' as second arg!\n");
        exit(1);
    }
    printf(
        "\n=====================\nRunning test with '%s' mode using %d "
        "threads\n=====================\n",
        argv[2], thread_count);
    FILE* in_file = fopen(argv[3], "r");
    assert(in_file != NULL);
    FILE* out_file = fopen(argv[4], "w");

    // Read file
    fseek(in_file, 0, SEEK_END);
    size_t file_size = ftell(in_file) + 1;
    char* in_text = malloc(file_size);
    rewind(in_file);
    fread(in_text, file_size, sizeof(*in_text), in_file);
    char* textptr = in_text;
    // Replace comments in text with a whitespace
    while (*textptr != '\0') {
        if (*textptr == COMMENT_INDICATOR) {
            do {
                *textptr = COMMENT_RELACER;
                textptr++;
            } while (*textptr != '\0' && *textptr != '\n');
        } else {
            textptr++;
        }
    }
    textptr = strtok(in_text, WHITESPACES);
    if (strcmp(MAGIC_NUMBER, textptr) != 0) {
        perror("Invalid file format!\n");
        exit(1);
    }
    // Actual bitmap reading
    textptr = strtok(NULL, WHITESPACES);
    in_bitmap.width = out_bitmap.width = strtol(textptr, NULL, 10);
    textptr = strtok(NULL, WHITESPACES);
    in_bitmap.height = out_bitmap.height = strtol(textptr, NULL, 10);
    textptr = strtok(NULL, WHITESPACES);
    in_bitmap.max_color = out_bitmap.max_color = strtol(textptr, NULL, 10);
    // P(in_bitmap.max_color);
    size_t s = in_bitmap.width * in_bitmap.height * sizeof(*in_bitmap.pixels);
    in_bitmap.pixels = malloc(s);
    out_bitmap.pixels = malloc(s);
    size_t current_pos = 0;
    while ((textptr = strtok(NULL, WHITESPACES)) != NULL) {
        in_bitmap.pixels[current_pos++] = strtol(textptr, NULL, 10);
    }
    if (current_pos != in_bitmap.width * in_bitmap.height) {
        perror("File is corrupted!\n");
        exit(1);
    }

    // Multithreaded image processing
    long t1 = get_timestamp();
    pthread_t threads[thread_count];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    thread_args args[thread_count];

    if (block) {
        for (int i = 0; i < thread_count; ++i) {
            args[i].from = i * in_bitmap.width / thread_count;
            args[i].to = ((i + 1) * in_bitmap.width) / thread_count;
            pthread_create(&threads[i], &attr, convert_image_thread_block,
                           &args[i]);
        }
    } else {
        for (int i = 0; i < thread_count; ++i) {
            args[i].from = i * in_bitmap.max_color / thread_count;
            args[i].to = ((i + 1) * in_bitmap.max_color) / thread_count;
            pthread_create(&threads[i], &attr, convert_image_thread_numbers,
                           &args[i]);
        }
    }

    // Waiting for threads
    long* retval;
    for (int i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], (void**)&retval);
        printf("Thread %d execution time: %ld\n", i, *retval);
        free(retval);
    }
    printf("Total execution time: %ld\n", get_timestamp() - t1);

    // Saving image to file
    fprintf(out_file, "%s\n%ld %ld\n%d\n", MAGIC_NUMBER, out_bitmap.width,
            out_bitmap.height, out_bitmap.max_color);
    int char_in_line = 0;
    for (size_t i = 0; i < out_bitmap.width * out_bitmap.height; ++i) {
        if (char_in_line > OUTPUT_FILE_MAX_LINE_LEN) {
            fprintf(out_file, "\n");
            char_in_line = 0;
        }
        fprintf(out_file, "%d ", out_bitmap.pixels[i]);
        char_in_line += 3 + 1;
    }

    return 0;
}
