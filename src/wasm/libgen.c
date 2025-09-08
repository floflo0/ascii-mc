#include <libgen.h>
#include <string.h>

char *__xpg_basename(char *path) {
    if (path == NULL) return NULL;

    char *basename_path = &path[strlen(path)];
    while (basename_path != path && *basename_path != '/') --basename_path;
    return basename_path;
}
