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

#define NUM_THREADS 4

typedef struct {
    char *start;
    char *end;
    double *output_start;
    double threshold;
    double vol_trigger;
    long long rows_in_chunk;  // Pass 1 result
    long long signals_found;  // Pass 2 result
} AnalystData;

// --- PASS 1: The Scout (Finding Boundaries and Total Rows) ---
void* scout_worker(void* arg) {
    AnalystData *data = (AnalystData*)arg;
    char *ptr = data->start;
    
    // Boundary alignment: Skip to the next newline if not the absolute start
    if (ptr != NULL && *ptr != '\n' && ptr != (char*)0) { 
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
    return (void*)ptr; // Return corrected start position
}

// --- PASS 2: The Analyst (Filtering for Signals) ---
void* analyst_worker(void* arg) {
    AnalystData *data = (AnalystData*)arg;
    char *ptr = data->start;
    double *out = data->output_start;
    double prev_price = 0;
    long long found = 0;

    for (long long i = 0; i < data->rows_in_chunk; i++) {
        double current = atof(ptr);
        
        // THE FILTER: Threshold + Volatility logic
        double diff = (prev_price == 0) ? 0 : fabs(current - prev_price);
        
        if (current > data->threshold && diff > data->vol_trigger) {
            out[found] = current;
            found++;
        }

        prev_price = current;
        while (ptr < data->end && *ptr != '\n') ptr++;
        ptr++;
    }
    data->signals_found = found;
    return NULL;
}

// --- THE ENGINE: Coordinating the Hydra's Brain ---
long long run_analyst_engine(const char *path, double *out, double T, double V) {
    int fd = open(path, O_RDONLY);
    struct stat sb;
    fstat(fd, &sb);
    char *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    pthread_t threads[NUM_THREADS];
    AnalystData t_data[NUM_THREADS];
    size_t chunk = sb.st_size / NUM_THREADS;

    // Phase 1: Scout territory
    for (int i = 0; i < NUM_THREADS; i++) {
        t_data[i].start = addr + (i * chunk);
        t_data[i].end = (i == NUM_THREADS - 1) ? addr + sb.st_size : t_data[i].start + chunk;
        pthread_create(&threads[i], NULL, scout_worker, &t_data[i]);
    }

    long long total_raw_rows = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        void *corrected_start;
        pthread_join(threads[i], &corrected_start);
        t_data[i].start = (char*)corrected_start;
        // We give each thread the same buffer size to be safe, or manage precisely
        t_data[i].output_start = out + (i * (10000001 / NUM_THREADS)); 
        t_data[i].threshold = T;
        t_data[i].vol_trigger = V;
    }

    // Phase 2: Execute Analyst logic
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

/* --- THE HANDSHAKE --- */
static PyObject* method_extract(PyObject* self, PyObject* args) {
    const char* filepath;
    Py_buffer view;
    double threshold, volatility;

    // Now accepting: string (s), buffer (w*), and two doubles (dd)
    if (!PyArg_ParseTuple(args, "sw*dd", &filepath, &view, &threshold, &volatility)) {
        return NULL;
    }

    double *buffer = (double *)view.buf;
    long long result;

    Py_BEGIN_ALLOW_THREADS
    result = run_analyst_engine(filepath, buffer, threshold, volatility);
    Py_END_ALLOW_THREADS

    PyBuffer_Release(&view);
    return PyLong_FromLongLong(result);
}

static PyMethodDef AxiomMethods[] = {
    {"extract_prices", method_extract, METH_VARARGS, "Filtered multi-threaded extraction."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef axiommodule = { PyModuleDef_HEAD_INIT, "libaxiom", NULL, -1, AxiomMethods };
PyMODINIT_FUNC PyInit_libaxiom(void) { return PyModule_Create(&axiommodule); }
