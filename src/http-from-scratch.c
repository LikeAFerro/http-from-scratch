#include "assets.h"
#include <stdio.h>

int main() {
    http_status_t status = http_server();
    if (status != OK) {
        fprintf(stderr, "Error: %d\n", status);
        return 1;
    }
    return 0;
}
