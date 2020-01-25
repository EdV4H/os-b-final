#include <curses.h>

int main (void) {
    char msg[] = "AAA BBB";
    char str1[4], str2[4];
    sscanf(msg, "%s", str1);
    sscanf(msg, "%s", str2);
    printf("%s\n%s\n", str1, str2);
    return 0;
}