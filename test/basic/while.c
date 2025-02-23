/* RUN: %compiler %s > %t.s
   RUN: %gcc %t.s -o %t
   RUN: %t | FileCheck %s
*/

int main() {
    int i = 0;
    while(i != 3) {
        // CHECK: Loop: 0
        // CHECK: Loop: 1
        // CHECK: Loop: 2 
        if (i == 0) {
            printf("Loop: 0\n");
        } else if (i == 1) {
            printf("Loop: 1\n");
        } else if (i == 2) {
            printf("Loop: 2\n");
        }
        i = i + 1;
    }

    int a = 0;
    int b = 1;
    int count = 0;
    int temp = 0;
    
    // CHECK: Fibonacci: 0
    // CHECK: Fibonacci: 1
    // CHECK: Fibonacci: 1
    // CHECK: Fibonacci: 2
    // CHECK: Fibonacci: 3
    // CHECK: Fibonacci: 5
    while (count != 6) {
        printf("Fibonacci: %d\n", a);
        temp = a;
        a = b;
        b = temp + b;
        count = count + 1;
    }

    return 0;
}

