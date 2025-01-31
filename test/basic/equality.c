// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t | FileCheck %s
int main() {
    int a = 5;
    int b = 5;
    int c = 3;

    // Test ==
    // CHECK: 1
    printf("%d\n", a == b);
    // CHECK: 0
    printf("%d\n", a == c);
    // CHECK: 1
    printf("%d\n", a == a);

    // Test !=
    // CHECK: 0
    printf("%d\n", a != b);
    // CHECK: 1
    printf("%d\n", a != c);
    // CHECK: 0
    printf("%d\n", a != a);

    return 0;
}
