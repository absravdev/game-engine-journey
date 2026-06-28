#include <iostream>
#include <cstddef>

class Resource {
public:
    explicit Resource(size_t n)
    {
        data_ = new int[n];
        size_ = n;
        for (size_t i = 0; i < n; i++)
        {
            data_[i] = static_cast<int>(i);
        }
        std::cout << "ctor n= " << n << "\n";
    }

    ~Resource()
    {
        if (data_ == nullptr)
        {
            std::cout << "dtor (data_ null? yes)" << "\n";
        }
        else
        {
            std::cout << "dtor (data_ null? no)" << "\n";
        }
        delete[] data_;
    }

    Resource(const Resource& other)
    {
        size_ = other.size_;
        data_ = new int[other.size_];
        for (size_t i = 0; i < other.size_; i++)
        {
            data_[i] = other.data_[i];
        }
        std::cout << "COPY" << "\n";
    }
    Resource& operator=(const Resource& other)
    {
        delete[] data_;               
        size_ = other.size_;       
        data_ = new int[other.size_];
        for (size_t i = 0; i < other.size_; i++)
        {
            data_[i] = other.data_[i];
        }
        std::cout << "COPY=" << "\n";
        return *this;
    }
    Resource(Resource&& other) noexcept
    {
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
        std::cout << "MOVE" << "\n";
    }
    Resource& operator=(Resource&& other) noexcept
    {
        if (this != &other)
        {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        std::cout << "MOVE=" << "\n";
        return *this;
    }

private:
    int* data_;
    size_t size_;
};

int main()
{
    Resource a(5);
    Resource b = a;
    b = a;
    Resource c = std::move(a);
    Resource d(3);
    d = std::move(b);  
}