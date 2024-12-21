// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t | FileCheck %s
// CHECK: a: 10
// CHECK: b: 5
// CHECK: c: 15
// CHECK: d: 3
// CHECK: d: 7

int main() {
    int a = 10;
    printf("a: %d\n", a);
    int b = 5;
    printf("b: %d\n", b);
    int c = a + b;
    printf("c: %d\n", c);
    int d = 3;
    printf("d: %d\n", d);
    d = a - 3;
    printf("d: %d\n", d);
    return 0;
} 
