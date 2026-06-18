#include <iostream>

// a and b are COPIES of the caller's variables.
// The swap works inside the function, but the copies are discarded on return,
// so main's variables are never affected.
void swapByValue(int a, int b)
{
    int temp = a;
    a = b;
    b = temp;
}

// a and b hold the ADDRESSES of the caller's variables.
// Dereferencing (*a, *b) reaches the original ints, so the swap reaches main.
// (The pointers themselves are passed by value; only what they point to is touched.)
void swapByPointer(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

// a and b are ALIASES for the caller's variables, so the swap reaches main directly.
void swapByReference(int& a, int& b)
{
    int temp = a;
    a = b;
    b = temp;
}

int main()
{
    int a = 1;
    int b = 2;
    std::cout << "start a: " << a << "  b: " << b << "\n";

    swapByValue(a, b);
    std::cout << "after value a: " << a << "  b: " << b << "\n";

    a = 1; b = 2; // reset, so each function is tested from the same starting point
    swapByPointer(&a, &b);
    std::cout << "after pointer a: " << a << "  b: " << b << "\n";

    a = 1; b = 2; // reset
    swapByReference(a, b);
    std::cout << "after ref a: " << a << "  b: " << b << "\n";
}