// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t | FileCheck %s


int func(int a, int b, int c, int d, int e, int f) {
    return a + b + c + d + e + f;
}

int add(int a, int b) {
    int result = a + b;
    return result;
}

int main() {
    int a = func(1, 2, 3, 4, 5, add(6, 7));
    // CHECK: a: 28
    printf("a: %d\n", a);
    return 0;
}
