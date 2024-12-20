// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t ; echo $? | FileCheck %s
// CHECK: 20

int main() {
    int a = 10;
    int b = 5;
    int c = a + b * 2;
    return c;
} 
