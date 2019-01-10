#include "timer.hpp"
#include <iostream>
#include "env.hpp"

double f()
{
    utils::timer t1("empty");
    return t1.stop();
}

int main(int argn, char** argv)
{
    double t{0};
    for (int i = 0; i < 1000; i++) {
        t += f();
    }
    std::cout << "timer overhead: " << t / 1000 << " sec.\n";

    for (int i = 0; i < 10; i++) {
        auto v = utils::get_env<bool>("HOME1");
        if (v) {
            std::cout << *v << "\n";
        }
    }

}
