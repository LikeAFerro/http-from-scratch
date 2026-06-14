#include "assets.h"
#include <stdio.h>

// TODO:
// - Default page (index.html)
// - Content-Type lookup table
// - Browser tests (e.g., curl, Firefox, Chrome)
// - Break-it challenge

int main() {
    http_status_t status = http_server();
    if (status != OK) {
        fprintf(stderr, "Error: %d\n", status);
        return 1;
    }
    return 0;
}
