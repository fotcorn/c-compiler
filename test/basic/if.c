// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t | FileCheck %s
int main() {
    int a = 5;
    int b = 5;
    int c = 3;

    // CHECK: a == b
    if (a == b) {
        printf("a == b\n");
    }

    if (a == c) {
        printf("a == c\n");
    }

    // CHECK: a == a
    if (a == a) {
        printf("a == a\n");
    }

    return 0;
}
