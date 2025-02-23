/* RUN: %compiler %s > %t.s
   RUN: %gcc %t.s -o %t
   RUN: %t | FileCheck %s
*/

int main() {
    int x = 6;
    if (x == 6) {
        int y = 10;
        // CHECK: if: y = 10
        printf("if: y = %d\n", y);
    }


    // CHECK: while: z = 6
    // CHECK: while: z = 4
    // CHECK: while: z = 2
    while (x != 0) {
        int z = x;
        printf("while: z = %d\n", z);
        x = x - 2;
    }

    return 0;
}

