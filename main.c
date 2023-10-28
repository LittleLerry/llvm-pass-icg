#include <stdio.h>

void f1() {
    printf("f1\n");
}

void f2() {
    printf("f2\n");
}

void f3(){
    printf("f3\n");
}

int main() {
    void (*fp)();
    int var;
    scanf("%d", &var);

    if (var == 1) {
        fp = &f1;
    } else if (var == 2) {
        fp = &f2;
    } else {
        fp = &f3;
    }
    
    (*fp)();

    return 0;
}
