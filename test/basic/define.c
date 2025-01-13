// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t | FileCheck %s
#define CONSTANT 3

int main() {
    int a = CONSTANT;
    
    // CHECK: a: 3
    printf("a: %d\n", a);
    
    // CHECK: constant: 3
    printf("constant: %d\n", CONSTANT);
    
    return 0;
}
