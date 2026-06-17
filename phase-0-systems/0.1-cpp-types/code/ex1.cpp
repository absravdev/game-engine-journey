#include <iostream>

// Exercise 1.3: pass-by-value demo. Defined above main so that main can see it.
void addOne(int n) {
    n += 1;  // modifies the local copy only; never the caller's variable
}

int main() {
    // Exercise 1.1: sizes of fundamental types, pointer types, and a struct.
    std::cout << "size of int: " << sizeof(int) << "\n";
    std::cout << "size of char: " << sizeof(char) << "\n";
    std::cout << "size of bool: " << sizeof(bool) << "\n";
    std::cout << "size of double: " << sizeof(double) << "\n";
    std::cout << "size of int*: " << sizeof(int*) << "\n";
    std::cout << "size of double*: " << sizeof(double*) << "\n";

    struct MyStruct {
        int x;
        char c;
    };
    std::cout << "size of struct: " << sizeof(MyStruct) << "\n";

    // Exercise 1.2 observations are written in README.md.

    // Exercise 1.3: value semantics.
    int a = 5;
    std::cout << "before: " << a << "\n";
    addOne(a);
    std::cout << "after: " << a << "\n";
    // 'a' does not change: inside addOne, n is not 'a' itself but a copy of it.
}