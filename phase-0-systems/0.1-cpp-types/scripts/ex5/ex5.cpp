#include <iostream>
#include <cstddef>

struct IntBuffer 
{ 
    int* data; 
    size_t size; 
};
IntBuffer makeBuffer(size_t n)
{
    int* a = new int[n];
    
    for (size_t i = 0; i < n; i++)
    {
        a[i] = i * i;
    }
    IntBuffer mystruct; 
    mystruct.data = a;
    mystruct.size = n;
    return mystruct;
}
int sumBuffer(const IntBuffer& buf)
{
    int sum = 0;
    for (size_t i = 0; i < buf.size; i++)
    {
        sum = buf.data[i] + sum;
    }
    return sum;
}
void freeBuffer(IntBuffer& buf)
{
    delete[] buf.data;
    buf.data = nullptr;
    buf.size = 0;
}
int main()
{
    IntBuffer buf = makeBuffer(3);
    std::cout << buf.size << "\n";
    int sumBuf = sumBuffer(buf);
    std::cout << sumBuf;
    freeBuffer(buf);
}

