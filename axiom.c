#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

// Reverting to 8 to utilize the full silicon die
#define NUM_THREADS 8 

typedef struct {
    char *start;
    char *end;
    double *output_start;
    double threshold;
    double vol_trigger;
    long long rows_in_chunk;
    long long signals_found;
} AnalystData;

static inline double fast_atof(const char *p) {
    double res = 0.0;
    int neg = 0;
    if (*p == '-') { neg = 1; p++; }
    while (*p >= '0' && *p <= '9') {
        res = res * 10.0 + (*p - '0');
        p++;
    }
    if (*p == '.') {
        p++;
        double frac = 0.1;
        while (*p >= '0' && *p <= '9') {
            res += (*p - '0') * frac;
            frac *= 0.1;
            p++;
        }
    }
    return neg ? -res : res;
}

void* scout_worker(void* arg) {
    AnalystData *data = (AnalystData*)arg;
    char *ptr = data->start;
    if (ptr != data->start && *ptr != '\n') {
        while (ptr < data->end && *ptr != '\n') ptr++;
        ptr++;
    }
    char *count_ptr = ptr;
    long long count = 0;
    while (count_ptr < data->end) {
        if (*count_ptr == '\n') count++;
        count_ptr++;
    }
    data->rows_in_chunk = count;
    return (void*)ptr;
}

void* analyst_worker(void* arg) {
    AnalystData *data = (AnalystData*)arg;
    char *ptr = data->start;
    double *out = data->output_start;
    double prev_price = 0;
    long long found = 0;

    for (long long i = 0; i < data->rows_in_chunk; i++) {
        // PREFETCH: Look 512 bytes ahead to hide memory latency
        __builtin_prefetch(ptr + 512, 0, 3);
        
        double current = fast_atof(ptr);
        
        if (__builtin_expect(current > data->threshold, 0)) {
            double diff = fabs(current - prev_price);
            if (diff > data->vol_trigger) {
                out[found++] = current;
            }
        }

        prev_price = current;
        // Optimization: Fast-forward to newline
        while (ptr < data->end && *ptr != '\n') ptr++;
        ptr++;
    }
    data->signals_found = found;
    return NULL;
}

long long run_analyst_engine(const char *path, double *out, double T, double V) {
    int fd = open(path, O_RDONLY);
    struct stat sb;
    fstat(fd, &sb);
    
    // KERNEL HINT: Tell Linux we are reading sequentially to trigger aggressive readahead
    char *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    madvise(addr, sb.st_size, MADV_SEQUENTIAL | MADV_WILLNEED);

    pthread_t threads[NUM_THREADS];
    AnalystData t_data[NUM_THREADS];
    size_t chunk = sb.st_size / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        t_data[i].start = addr + (i * chunk);
        t_data[i].end = (i == NUM_THREADS - 1) ? addr + sb.st_size : t_data[i].start + chunk;
        pthread_create(&threads[i], NULL, scout_worker, &t_data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        void *corrected_start;
        pthread_join(threads[i], &corrected_start);
        t_data[i].start = (char*)corrected_start;
        t_data[i].output_start = out + (i * (10000000 / NUM_THREADS));
        t_data[i].threshold = T;
        t_data[i].vol_trigger = V;
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, analyst_worker, &t_data[i]);
    }

    long long total_signals = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        total_signals += t_data[i].signals_found;
    }

    munmap(addr, sb.st_size);
    close(fd);
    return total_signals;
}
