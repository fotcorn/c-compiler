// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t | FileCheck %s
int add(int a, int b) {
    int result = a + b;
    return result;
}

int main() {
    int a = add(1, 2);
    // CHECK: a: 3
    printf("a: %d\n", a);
    return 0;
}
