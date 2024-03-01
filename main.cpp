#include <iostream>
#include "basics.h"

#include "RingBuffer.h"

template <typename T>
void print_buffer(RingBuffer<T>& r)
{
    for (int i = 0; i < r.size(); i++)
    {
        std::cout << r[i] << '\n';
    }
}

int main()
{
    //basic_main();
    //order_book_ex();

    RingBuffer<std::string> rb(3);
    rb.push_back("Tim");
    rb.push_back("Sarah");
    rb.push_back("Max");

    print_buffer(rb);

    rb.push_back("Wilson");
    rb.push_back("Jill");

    std::cout << '\n';
    print_buffer(rb);

    std::cout << '\n';

    rb.print_top();
    rb.print_top();
    rb.print_top();
    rb.print_top();
    rb.print_top();

    return 0;
}