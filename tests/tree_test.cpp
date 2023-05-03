#include "file_tree.hpp"
#include <iostream>

int main() {
    try {
        FileTree ft(fs::path("/Users/josephliotta/dev"));
    
        auto root = ft.getRoot();

        auto action = [root](FileNode* node) {
            if (node->path == root->path) {
                fs::path p(node->path);
                cout << static_cast<string>(p.filename()) << endl;
            } else {
                auto parent = node->getParent();
                string name;
                while (parent->path != root->path) {
                    name = static_cast<string>(parent->path.filename()) + "/" + name;
                    parent = parent->parent;
                }
                cout << name << static_cast<string>(node->path.filename()) << endl;
            }
        };

        ft.fileAction(action);
    }
    catch (const std::exception &e) {
        cerr << e.what() << endl;
    } 
    return 0;
}