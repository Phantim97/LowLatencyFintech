#include <iostream>

#include "basics.h"
#include "RingBuffer.h"
#include "mem_pool.h"

template <typename T>
void print_buffer(RingBuffer<T>& r)
{
    for (int i = 0; i < r.size(); i++)
    {
        std::cout << r[i] << '\n';
    }
}

struct MyData
{
	int d[3];
};

void mempool_ex()
{
	MemPool<double> prim_pool(50);
	MemPool<MyData> struct_pool(50);

	//Allocate in Memory Pools
	for (size_t i = 0; i < 50; i++)
	{
		double *p_ret = prim_pool.allocate(i);
		MyData *s_ret = struct_pool.allocate(
				MyData {static_cast<int>(i), static_cast<int>(i + 1), static_cast<int>(i + 2)});

		std::cout << "Prim element:" << *p_ret << "Allocated at:" << p_ret << '\n';
		std::cout << "Struct Elem:" << s_ret->d[0] << ", " << s_ret->d[1] << ", " << s_ret->d[2]
			<< " allocated at: " << s_ret << '\n';

		if (i % 5 == 0)
		{
			std::cout << "Deallocating prim elem: " << *p_ret << " from: " << p_ret << '\n';
			std::cout << "Deallocating struct elem: " <<   s_ret->d[0] << ", " << s_ret->d[1] << ", " << s_ret->d[2]
				<< " from:" << s_ret << '\n';

			prim_pool.deallocate(p_ret);
			struct_pool.deallocate(s_ret);
		}
	}
}

void ring_buffer_ex()
{
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
}

int main()
{
    //basic_main();
    //order_book_ex();

	mempool_ex();
    return 0;
}