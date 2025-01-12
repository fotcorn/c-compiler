// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t | FileCheck %s
int main() {
    int a = 3;
    int b = 4;
    int c = a + b * 2;
    // CHECK: c: 11
    printf("c: %d\n", c);
    // CHECK: imm c: 11
    printf("imm c: %d\n", a + b * 2);

    int d = a * 2 + b;
    // CHECK: d: 10
    printf("d: %d\n", d);
    // CHECK: imm d: 10
    printf("imm d: %d\n", a * 2 + b);

    return 0;
}
