#include <boost/fiber/all.hpp>

#include <iostream>

int main()
{
    boost::fibers::buffered_channel<int> channel(8);
    int sum = 0;

    boost::fibers::fiber producer([&channel] {
        for (int i = 1; i <= 5; i++)
            channel.push(i);
        channel.close();
    });
    boost::fibers::fiber consumer([&channel, &sum] {
        int value;
        while (channel.pop(value) == boost::fibers::channel_op_status::success)
            sum += value;
    });

    producer.join();
    consumer.join();

    if (sum != 15)
        return 1;

    std::cout << "fiber: sum=" << sum << std::endl;
    return 0;
}
