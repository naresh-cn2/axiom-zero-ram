#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

int main() {
    clock_t start = clock();
    
    // 1. Open the file directly
    int fd = open("massive_data.csv", O_RDONLY);
    if (fd < 0) { perror("Open failed"); return 1; }

    // 2. Get the exact file size
    struct stat s;
    fstat(fd, &s);
    size_t size = s.st_size;

    // 3. THE WEAPON: Map the file directly into virtual memory
    char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) { perror("mmap failed"); return 1; }

    long rows = 0;
    
    // 4. Scan raw bytes at hardware speed
    for (size_t i = 0; i < size; i++) {
        if (data[i] == '\n') rows++;
    }

    // Clean up
    munmap(data, size);
    close(fd);
    clock_t end = clock();

    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("\nAxiom C-Engine Time: %f seconds.\n", time_spent);
    printf("Rows processed: %ld\n", rows - 1); // Subtract 1 for header
    
    return 0;
}
