// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t ; echo $? | FileCheck %s
// CHECK: 42

int main() {
    return 42;
}
