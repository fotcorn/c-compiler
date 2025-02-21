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

    // CHECK: a == b
    if (a == b) {
        printf("a == b\n");
    } else {
        printf("a != b\n");
    }

    // CHECK: a != c
    if (a == c) {
        printf("a == c\n");
    } else {
        printf("a != c\n");
    }

    // CHECK: a == a
    if (a == a) {
        printf("a == a\n");
    } else {
        printf("a != a\n");
    }

    // CHECK: a == b and b != c
    if (a == b) {
        if (b == c) {
            printf("a == b and b == c\n");
        } else {
            printf("a == b and b != c\n");
        }
    } else {
        printf("a != b\n");
    }

    // CHECK: a == b
    if (a == b) {
        printf("a == b\n");
    } else if (b == c) {
        printf("b == c\n");
    } else {
        printf("a == b and b != c\n");
    }

    // CHECK: a == b
    if (b == c) {
        printf("b == c\n");
    } else if (a == b) {
        printf("a == b\n");
    } else {
        printf("a == b and b != c\n");
    }

    // CHECK: a != c and b != c
    if (a == c) {
        printf("a == c\n");
    } else if (b == c) {
        printf("b == c\n");
    } else {
        printf("a != c and b != c\n");
    }

    return 0;
}
