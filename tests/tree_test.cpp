#include "file_tree.hpp"
#include <iostream>
#include <cppcoro/sync_wait.hpp>

int main() {
    try {
        FileTree ft(fs::path("../"));
    
        fs::path rootp = ft.getRootPath();

        auto action = [rootp](FileNode* node) {
            if (node->path == rootp) {
                fs::path p(node->path);
                cout << static_cast<string>(p.filename()) << endl;
            } else {
                FileNode* parent = node->getParent();
                string name;
                while (parent->getParent() != nullptr) {
                    name = static_cast<string>(parent->path.filename()) + "/" + name;
                    parent = parent->getParent();
                }
                cout << name << static_cast<string>(node->path.filename()) << endl;

            }
        };

        cppcoro::sync_wait(ft.fileAction(action));
    }
    catch (const std::exception &e) {
        cerr << e.what() << endl;
    } 
    return 0;
}