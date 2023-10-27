#include <stdio.h>

void function1() {
    printf("This is function 1.\n");
}

void function2() {
    printf("This is function 2.\n");
}

int main() {
    void (*funcPtr)();  // Declare a function pointer

    int choice;
    scanf("%d", &choice);

    if (choice == 1) {
        funcPtr = &function1;  // Assign the address of function1 to funcPtr
    } else if (choice == 2) {
        funcPtr = &function2;  // Assign the address of function2 to funcPtr
    } else {
        printf("Invalid choice.\n");
        return 1;
    }

    (*funcPtr)();  // Call the function through the function pointer

    return 0;
}