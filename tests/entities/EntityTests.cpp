#include "EntityTests.h"

namespace tests {

void runEntityTests() {
    EntityTests::runAllTests();
}

} // namespace tests

int main() {
    try {
        tests::runEntityTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
