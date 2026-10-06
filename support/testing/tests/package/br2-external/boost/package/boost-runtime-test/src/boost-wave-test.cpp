#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>

#include <iostream>
#include <string>

typedef boost::wave::cpplexer::lex_token<> token_type;
typedef boost::wave::cpplexer::lex_iterator<token_type> lex_iterator_type;
typedef boost::wave::context<std::string::iterator, lex_iterator_type> context_type;

int main()
{
    std::string input =
        "#define GREETING(x) hello x\n"
        "#if 1 + 1 == 2\n"
        "GREETING(buildroot)\n"
        "#else\n"
        "wrong\n"
        "#endif\n";

    context_type ctx(input.begin(), input.end(), "test.cpp");
    std::string output;
    for (context_type::iterator_type it = ctx.begin(); it != ctx.end(); ++it)
        output += (*it).get_value().c_str();

    if (output.find("hello buildroot") == std::string::npos ||
        output.find("wrong") != std::string::npos)
        return 1;

    std::cout << "wave: hello buildroot" << std::endl;
    return 0;
}
