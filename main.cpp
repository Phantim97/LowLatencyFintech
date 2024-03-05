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

#include "lock_free_q.h"
#include "thread_utils.h"
#include <thread>

void LFQ_Consume(LFQueue<MyData>* lfq)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);

	while (lfq->size())
	{
		const MyData* d = lfq->get_next_to_read();
		if (d != nullptr)
		{
			lfq->update_read_idx();

			std::cout << "Consume function read: " << d->d[0] << ", " << d->d[1] << ", " << d->d[2] << " | LFQ SZ: "
			          << lfq->size() << '\n';
		}
		std::this_thread::sleep_for(1s);
	}

	std::cout << "Exiting Consume Function\n";
}

void LFQ_test()
{
	using namespace std::chrono_literals;

	LFQueue<MyData> lfq(20);
	std::thread* ct = launch_thread(-1, "", LFQ_Consume, &lfq);
	//std::thread* ct2 = launch_thread(-1, "", LFQ_Consume, &lfq);

	for (size_t i = 0; i < 50; i++)
	{
		const MyData d{static_cast<int>(i), static_cast<int>(i*10), static_cast<int>(i * 100)};
		*(lfq.get_next_write_loc()) = d;
		lfq.update_write_idx();

		std::cout << "Main constructed elem: " << d.d[0] << ", " << d.d[1] << ", " << d.d[2] << " | LFQ SZ: " << lfq.size() << '\n';
		std::this_thread::sleep_for(1s);
	}

	ct->join();
	//ct2->join();

	std::cout << "Main exiting";
}

int main()
{
    //basic_main();
    //order_book_ex();

	//mempool_ex();
	LFQ_test();
    return 0;
}