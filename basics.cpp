#include <iostream>

enum TradeState
{
    Opener = 0x01,
    Modification = 0x02,
    Cleared = 0x10,
    Closing = 0x100,
};

void branch()
{
    u_int64_t x = 0x0;
    x |= Opener;

    if (x & (Opener | Cleared | Closing))
    {
        std::cout << "Optimized\n";
    }

    u_int32_t s2 = 0;
    s2 |= Opener;
    s2 |= Cleared;
    //s2 |= Closing //Will invalidate boolean below

    if ((s2 & (Opener | Cleared)) && !(s2 & Closing))
    {
        std::cout << "Cleared and opened\n";
    }
}

//Move constructor refresh
class MyClass
{
private:
    int* data;
public:
    // Move constructor
    MyClass(MyClass&& other) noexcept : data(other.data)
    {
        other.data = nullptr; // Ensure the 'moved-from' object is in a safe state
    }
};

//This is pointer aliasing, __restrict is a compiler optimization because it tells it that
//These two pointers do not intersect at all it can optimize at compile time with that
void updateDatabase(int* __restrict a, const int* __restrict b, int size)
{
    for (int i = 0; i < size; ++i) {
        a[i] = b[i] + 5;
    }
}

void basic_main()
{
    branch();
}

//Order Book example
#include <vector>

struct Order
{
    int id;
    double price;
};

class InheritanceOrderBook : public std::vector<Order> {};

class CompositionOrderBook
{
private:
    std::vector<Order> orders_;
public:
    [[nodiscard]] size_t size() const noexcept
    {
        return orders_.size();
    }
};

//Runtime Polymorphism
class RuntimePolyBook
{
public:
    virtual void PlaceOrder()
    {
        std::cout << "PlaceOrder\n";
    }
};

class SpecificRuntime : public RuntimePolyBook
{
public:
    void PlaceOrder() override
    {
        std::cout << "SpecificPlaceeOrder\n";
    }
};

//CRTP
template<typename actual_type>
class CRTPOrderBook
{
public:
    void place_order()
    {
        static_cast<actual_type*>(this)->actualPlaceOrder();
    }

    void actualPlaceOrder()
    {
        std::cout << "actual place order\n";
    }
};

class SpecificCRTPOrderBook : public CRTPOrderBook<SpecificCRTPOrderBook>
{
public:
    void actualPlaceOrder()
    {
        std::cout << "Specific Order Actually Placed\n";
    }
};

//CRTP Abstract Implementation
template <typename Derived>
class Base
{
public:
    void interface()
    {
        // ...
        static_cast<Derived*>(this)->implementation();
        // ...
    }

    // A default implementation that can be overridden
    void implementation()
    {
        // Default behavior (if any)
    }
};

class Derived : public Base<Derived>
{
public:
    void implementation()
    {
        // Derived-specific behavior
    }
};


void order_book_ex()
{
    InheritanceOrderBook i_book;
    CompositionOrderBook c_book;
    std::cout << i_book.size() << " " << c_book.size();

    RuntimePolyBook* runtime_ex = new SpecificRuntime();
    runtime_ex->PlaceOrder();

    CRTPOrderBook<SpecificCRTPOrderBook> crtp_ex;
    crtp_ex.place_order();
    delete runtime_ex;
}