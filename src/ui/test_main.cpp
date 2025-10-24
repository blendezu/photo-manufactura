#include <iostream>

#include "ui_main.h"

int main() {
    std::cout << "Testing UI Component independently\n";
    ui::initialize();
    ui::render();
    return 0;
}