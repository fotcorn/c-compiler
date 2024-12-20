// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t ; echo $? | FileCheck %s
// CHECK: 15

int main() {
    int a = 10;
    int b = 5;
    int c = a + b;
    return c;
} 
