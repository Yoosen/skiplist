#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"

int main() {
    Skiplist<int, std::string> skipList(6);

    skipList.insert_element(1, "I");
    skipList.insert_element(3, "Love");
    skipList.insert_element(7, "Ling");
    skipList.insert_element(8, "Yoosen");
    skipList.insert_element(9, "Is");
    skipList.insert_element(19, "Niubility");
    skipList.insert_element(19, "!");

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.dump_file();

    skipList.search_element(9);
    skipList.search_element(18);

    skipList.display_list();

    skipList.delete_element(3);
    skipList.delete_element(7);

    std::cout << "skipList size:" << skipList.size() << std::endl;
    skipList.display_list();
}