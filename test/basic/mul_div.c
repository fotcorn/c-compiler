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

  // Multiplication variants
  // CHECK: c: 50
  int c = a * b; // variable * variable
  printf("c: %d\n", c);
  // CHECK: d: 50
  int d = 10 * 5; // constant * constant
  printf("d: %d\n", d);
  // CHECK: e: 50
  int e = a * 5; // variable * constant
  printf("e: %d\n", e);
  // CHECK: f: 50
  int f = 10 * b; // constant * variable
  printf("f: %d\n", f);

  // Division variants
  // CHECK: g: 2
  int g = a / b; // variable / variable
  printf("g: %d\n", g);
  // CHECK: h: 2
  int h = 10 / 5; // constant / constant
  printf("h: %d\n", h);
  // CHECK: i: 2
  int i = a / 5; // variable / constant
  printf("i: %d\n", i);
  // CHECK: j: 2
  int j = 10 / b; // constant / variable
  printf("j: %d\n", j);

  // Assignment variants with multiplication
  // CHECK: k: 50
  int k = 0;
  k = a * b; // variable * variable
  printf("k: %d\n", k);
  // CHECK: l: 50
  int l = 0;
  l = 10 * 5; // constant * constant
  printf("l: %d\n", l);
  // CHECK: m: 50
  int m = 0;
  m = a * 5; // variable * constant
  printf("m: %d\n", m);
  // CHECK: n: 50
  int n = 0;
  n = 10 * b; // constant * variable
  printf("n: %d\n", n);

  // Assignment variants with division
  // CHECK: o: 2
  int o = 0;
  o = a / b; // variable / variable
  printf("o: %d\n", o);
  // CHECK: p: 2
  int p = 0;
  p = 10 / 5; // constant / constant
  printf("p: %d\n", p);
  // CHECK: q: 2
  int q = 0;
  q = a / 5; // variable / constant
  printf("q: %d\n", q);
  // CHECK: r: 2
  int r = 0;
  r = 10 / b; // constant / variable
  printf("r: %d\n", r);
  return 0;
}
