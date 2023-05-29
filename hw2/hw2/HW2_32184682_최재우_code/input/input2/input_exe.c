#include <stdio.h>
int foo(int index);

int main() {
    int index = 4;
    int res = foo(index);
    printf("%d\n", res);
    return res;
}

int foo(int index) {
    if (index == 1)
        return 1;
    else
        return index + foo(index-1);
}
