#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"

int main() {
    Skiplist<int, std::string> skipList(6);

    std::cout << "--- insert test ---" << std::endl;
    skipList.insert_element(1, "I");
    skipList.insert_element(3, "Love");
    skipList.insert_element(7, "Ling");
    skipList.insert_element(8, "Yoosen");
    skipList.insert_element(9, "Is");
    skipList.insert_element(19, "Niubility");
    skipList.insert_element(19, "!");

    std::cout << "skipList size:" << skipList.size() << std::endl;

    // 数据落盘操作
    std::cout << "--- dump file test ---" << std::endl;
    skipList.dump_file();

    // 查找操作
    std::cout << "--- search test ---" << std::endl;
    skipList.search_element(9);
    skipList.search_element(18);

    skipList.display_list();

    // 删除操作
    std::cout << "--- delete test ---" << std::endl;
    skipList.delete_element(3);
    skipList.delete_element(7);

    // 更新测试
    std::cout << "--- update test ---" << std::endl;
    skipList.update_element(3, "Like");
    skipList.update_element(10, "So Bad");
    skipList.update_element(10, "So Good", true);
    std::cout << "skipList size:" << skipList.size() << std::endl;

    std::cout << "skipList size:" << skipList.size() << std::endl;
    skipList.display_list();
}