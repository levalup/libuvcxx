#include <iostream>
#include "uvcxx.h"

int main() {
    uv::loop loop; // = uv::default_loop();

    std::cout << "Default loop." << std::endl;
    loop.run();

    return 0;
}