#include <iostream>

// Allocates an int on the heap and returns its address; the caller owns it and must delete it.
int* makeOnHeap(int a)
{
    return new int(a);
}

int main()
{
    // heap object: outlives the function, so we must free it ourselves
    int a = 1;
    int* result = makeOnHeap(a);
    std::cout << "result (value): " << *result << "\n";
    delete result;
    std::cout << "result before nullptr: " << result << "\n";
    result = nullptr;
    std::cout << "result after nullptr:  " << result << "\n";

    // makeOnStack is broken (never called):
    // int* makeOnStack(int v) { int local = v; return &local; }
    // local dies when the function returns, so it returns the address of a dead
    // variable -> a dangling pointer (UB to dereference).

    // leak (on purpose): the only pointer to the heap int gets overwritten
    int c = 6;
    int* b = new int(5);
    std::cout << "b (heap):  " << b << "\n";
    b = &c;
    std::cout << "b (now &c): " << b << "\n";

    // fix: delete while we still hold the address, before reusing the pointer
    int e = 6;
    int* d = new int(5);
    std::cout << "d (heap):  " << d << "\n";
    delete d;
    d = &e;
    std::cout << "d (now &e): " << d << "\n";
}