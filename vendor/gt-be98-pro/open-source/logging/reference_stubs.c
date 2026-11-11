#include <stdlib.h>
#include <string.h>
char *nvram_get(const char *key) {
    if (!strcmp(key,"asuslog_path")) return getenv("TEST_LOG_BASE");
    return NULL;
}
int cprintf(const char *format, ...) { (void)format;return 0; }
