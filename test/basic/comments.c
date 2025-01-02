// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t | FileCheck %s

// Single line comment test
/* Multi-line comment test
   spanning multiple lines
   with some text */

// CHECK: a: 42
// CHECK: b: 24

int main() {
    // Single line comment before variable
    int a = 42;
    printf("a: %d\n", a);

    /* Multi-line comment
       before variable
       declaration */
    int b = 24;
    printf("b: %d\n", b);

    return 0;
} 