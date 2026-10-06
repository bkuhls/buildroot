#include <boost/python.hpp>

#include <string>

static std::string greet(const std::string &name)
{
    return "hello " + name;
}

struct counter {
    int value = 0;
    void add(int n)
    {
        value += n;
    }
};

BOOST_PYTHON_MODULE(boost_python_test)
{
    namespace bp = boost::python;

    bp::def("greet", greet);
    bp::class_<counter>("Counter")
        .def("add", &counter::add)
        .def_readonly("value", &counter::value);
}
