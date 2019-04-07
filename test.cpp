#include "LSMTree.hpp"
#include <cassert>
#include <iostream>

#define RUN(TEST_NAME) \
    TEST_NAME(); \
    std::experimental::filesystem::remove_all("./storage"); \
    std::cout << "Done: " << #TEST_NAME << std::endl

void test_simple_insert()
{
    LSMTree tree("./storage");
    tree.insert(10);
    assert(tree.find(10));
    assert(!tree.find(11));
}

void test_big_data()
{
    LSMTree tree("./storage");
    for (size_t i = 0; i < 250; i++) {
        tree.insert(i);
    }

    tree.remove(23);

    assert(tree.find(24));
    assert(!tree.find(23));
}

void test_simple_5_insert()
{
    LSMTree tree("./storage");
    tree.insert(10);
    tree.insert(8);
    tree.insert(12);
    tree.insert(11);
    tree.insert(9);
    tree.insert(11);
    assert(tree.find(8));
    assert(tree.find(9));
    assert(tree.find(10));
    assert(tree.find(11));
    assert(tree.find(12));
    assert(!tree.find(7));
}

int main()
{
    RUN(test_simple_insert);
    RUN(test_simple_5_insert);
    RUN(test_big_data);
}
