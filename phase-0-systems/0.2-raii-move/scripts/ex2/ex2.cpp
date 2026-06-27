#include <iostream>
#include <memory>

int main()
{
    // unique_ptr<int>: single ownership
    auto p = std::make_unique<int>(42);
    std::cout << "p before move:\n";
    std::cout << "  address: " << p.get() << "\n";
    std::cout << "  value:   " << *p << "\n";

    // auto q = p;  // does NOT compile: unique_ptr copy ctor is = deleted (single ownership)
    auto q = std::move(p);  // transfer ownership (move ctor), not a copy

    std::cout << "after move:\n";
    if (p)
        std::cout << "  p still owns something\n";
    else
        std::cout << "  p is empty (moved-from, holds nullptr)\n";
    std::cout << "  q value: " << *q << "\n";  // q is the new owner; *q is safe

    // unique_ptr<int[]>: owns a heap array, frees with delete[]
    const std::size_t n = 5;
    auto arr = std::make_unique<int[]>(n);
    for (std::size_t i = 0; i < n; i++)
        arr[i] = i * i;
    for (std::size_t i = 0; i < n; i++)
        std::cout << arr[i] << "\n";

    // cost: unique_ptr<int> is the size of a raw pointer (zero overhead)
    std::cout << "sizeof(unique_ptr<int>): " << sizeof(std::unique_ptr<int>) << "\n";
    std::cout << "sizeof(int*):            " << sizeof(int*) << "\n";
}