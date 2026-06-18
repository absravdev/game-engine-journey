#include <iostream>

int main()
{
    //2.1
    int x = 10;
    int* p = &x;
    //2.2
    std::cout << "x: " << x << "\n";
    std::cout << "&x: " << &x << "\n";
    std::cout << "p: " << p << "\n";
    std::cout << "&p: " << &p << "\n";
    std::cout << "*p: " << *p << "\n";
    //2.3
    *p = 20;
    std::cout << "x: " << x << "\n";
    //2.4
    std::cout << "size of p: " << sizeof(p) << "\n";
    std::cout << "size of *p: " << sizeof(*p) << "\n";
    //p is a pointer, so its value is a memory address; all addresses occupy 8 bytes on a 64-bit build,
    // regardless of what they point to. *p is the int it points to (here, 20), and an int occupies 4 bytes.