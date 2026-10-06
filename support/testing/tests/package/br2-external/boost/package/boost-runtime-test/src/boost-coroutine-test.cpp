#define BOOST_COROUTINES_NO_DEPRECATION_WARNING
#include <boost/coroutine/asymmetric_coroutine.hpp>

#include <iostream>

typedef boost::coroutines::asymmetric_coroutine<int> coro_t;

int main()
{
    coro_t::pull_type fibonacci([](coro_t::push_type &sink) {
        int a = 0, b = 1;
        for (int i = 0; i < 10; i++) {
            sink(a);
            int next = a + b;
            a = b;
            b = next;
        }
    });

    int count = 0, last = 0;
    while (fibonacci) {
        last = fibonacci.get();
        count++;
        fibonacci();
    }

    if (count != 10 || last != 34)
        return 1;

    std::cout << "coroutine: fib(9)=" << last << std::endl;
    return 0;
}
