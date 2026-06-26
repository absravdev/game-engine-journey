#include <iostream>

class HeapInt {
public:
    HeapInt(int v) {
        ptr_ = new int(v);
        std::cout << "acquire " << v << "\n";
    }

    ~HeapInt() {
        std::cout << "release " << *ptr_ << "\n";
        delete ptr_;
    }

    int value() const {
        return *ptr_;
    }

private:
    int* ptr_;
};

int main() {
    HeapInt a(1);
    {
        HeapInt b(2);
        HeapInt c(3);
    }
    HeapInt d(4);
}