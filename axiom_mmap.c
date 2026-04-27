#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[]) {
    // 1. INPUT VALIDATION: Ensure a file was actually provided
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename.csv>\n", argv[0]);
        return 1;
    }

    const char *filepath = argv[1];
    int fd = open(filepath, O_RDONLY);
    
    // 2. PERMISSION CHECK: Can we actually read this file?
    if (fd == -1) {
        perror("Error opening file (Permissions or File Not Found)");
        return 1;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("Error getting file stats");
        close(fd);
        return 1;
    }

    // 3. THE "ZERO-BYTE" DEFENSE: mmap fails on empty files
    if (sb.st_size == 0) {
        fprintf(stderr, "Error: File is empty. Nothing to process.\n");
        close(fd);
        return 1;
    }

    // 4. SECURE MAPPING
    char *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap failed (Out of Virtual Address Space)");
        close(fd);
        return 1;
    }

    printf("Axiom Core: Processing %lld bytes...\n", (long long)sb.st_size);

    clock_t start = clock();
    long long rows = 0;

    // 5. DATA SCANNING: Sequential access trigger
    for (size_t i = 0; i < sb.st_size; i++) {
        if (addr[i] == '\n') {
            rows++;
        }
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("\n--- Axiom Hardened Results ---\n");
    printf("Total Rows Identified: %lld\n", rows - 1); // Subtracting 1 for header
    printf("Processing Time: %.4f seconds\n", time_spent);
    printf("------------------------------\n");

    // 6. GRACEFUL EXIT
    munmap(addr, sb.st_size);
    close(fd);

    return 0;
}
