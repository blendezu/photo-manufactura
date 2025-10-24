#include "ui_main.h"
#include <iostream>

int main() {
    std::cout << "Testing UI Component independently\n";
    ui::initialize();
    ui::render();
    return 0;
}