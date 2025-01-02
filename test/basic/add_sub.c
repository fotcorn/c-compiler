// RUN: %compiler %s > %t.s
// RUN: %gcc %t.s -o %t
// RUN: %t | FileCheck %s
int main() {
  // Basic definitions and assignments
  // CHECK: a: 10
  int a = 10;
  printf("a: %d\n", a);
  // CHECK: b: 5
  int b = 5;
  printf("b: %d\n", b);

  // Addition variants
  // CHECK: c: 15
  int c = a + b; // variable + variable
  printf("c: %d\n", c);
  // CHECK: d: 15
  int d = 10 + 5; // constant + constant
  printf("d: %d\n", d);
  // CHECK: e: 15
  int e = a + 5; // variable + constant
  printf("e: %d\n", e);
  // CHECK: f: 15
  int f = 10 + b; // constant + variable
  printf("f: %d\n", f);

  // Subtraction variants
  // CHECK: g: 5
  int g = a - b; // variable - variable
  printf("g: %d\n", g);
  // CHECK: h: 5
  int h = 10 - 5; // constant - constant
  printf("h: %d\n", h);
  // CHECK: i: 5
  int i = a - 5; // variable - constant
  printf("i: %d\n", i);
  // CHECK: j: 5
  int j = 10 - b; // constant - variable
  printf("j: %d\n", j);

  // Assignment variants with addition
  // CHECK: k: 15
  int k = 0;
  k = a + b; // variable + variable
  printf("k: %d\n", k);
  // CHECK: l: 15
  int l = 0;
  l = 10 + 5; // constant + constant
  printf("l: %d\n", l);
  // CHECK: m: 15
  int m = 0;
  m = a + 5; // variable + constant
  printf("m: %d\n", m);
  // CHECK: n: 15
  int n = 0;
  n = 10 + b; // constant + variable
  printf("n: %d\n", n);

  // Assignment variants with subtraction
  // CHECK: o: 5
  int o = 0;
  o = a - b; // variable - variable
  printf("o: %d\n", o);
  // CHECK: p: 5
  int p = 0;
  p = 10 - 5; // constant - constant
  printf("p: %d\n", p);
  // CHECK: q: 5
  int q = 0;
  q = a - 5; // variable - constant
  printf("q: %d\n", q);
  // CHECK: r: 5
  int r = 0;
  r = 10 - b; // constant - variable
  printf("r: %d\n", r);
  return 0;
}
