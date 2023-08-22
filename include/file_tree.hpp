#ifndef FILE_TREE_HPP
#define FILE_TREE_HPP

#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <filesystem>
#include <functional>
#include <exception>
#include <thread>
#include <cppcoro/recursive_generator.hpp>
#include <cppcoro/generator.hpp>
#include <cppcoro/task.hpp>

using namespace std;

namespace fs = std::filesystem;

/**
 * Represents a file in a directory structure
 * May have a parent node or children nodes
*/
struct FileNode {
    FileNode* parent;
    bool has_children;
    vector<unique_ptr<FileNode>> children;
    fs::path path;

    FileNode() {
        parent = nullptr;
        has_children = false;
    }
    explicit FileNode(fs::path p) : parent(nullptr), path(p) {
        if (fs::is_directory(path)) {
            has_children = true;
            thread t(getChildren, std::move(this));
            t.join();
        } else if (!fs::is_empty(path)) {
            has_children = false;
        }
    }
    FileNode(fs::path p, FileNode* parent) : parent(parent), path(p) {
        if (fs::is_directory(path)) {
            has_children = true;
            thread t(getChildren, this);
            t.join();
        } else if (!fs::is_empty(path)) {
            has_children = false;
        }
    }

    // Finds children and constructs nodes from them
    static void getChildren(FileNode* node) {
        for (auto const& dir_entry : fs::directory_iterator(node->path)) {
            node->children.push_back(make_unique<FileNode>(dir_entry, node));
        }
    }

    // Getter for parent pointer
    FileNode* getParent() {
        FileNode* p = parent;
        return p;
    }

    fs::path getParentPath() const {
        return parent->path;
    }

};


/**
 * Represents a File tree with a root node
*/
class FileTree {
private:
    unique_ptr<FileNode> root;

    /**
     * Non-blocking stackbased file node generator
     * using coroutines(very fast when spawing lots of tasks)
     * to provide an iterator like pointer after each co_yeild is called.
     * Private function so we can properly handle iterators.
    */
    cppcoro::generator<FileNode*> getNodes() const {
        stack<FileNode*> stack_;
        stack_.push(root.get());
        while (!stack_.empty()) {
            auto node = stack_.top();
            stack_.pop();

            if (!node->has_children) {
                co_yield node;
            } else {
                for (auto const &child : node->children) {
                    stack_.push(child.get());
                }
            }
        }
    }

public:
    explicit FileTree(fs::path base_file) {
        root = make_unique<FileNode>(base_file);
    }


    /**
     * Uses iterator generator getNodes() to multitask retrival
     * of nodes and preformes tasks as soon as a pointer is produced
     * 
     * @param action void func(FileNode*) takes pointer and performs action with it
    */
    cppcoro::task<> fileAction(function<void(FileNode*)> action) const {
        for (auto const &f : getNodes()) {
            action(f);
        }
        co_return;
    }

    /**
     * Getter function for reference to root node
     * 
     * @return FileNode* root
     */
    fs::path getRootPath() const {
        return root->path;
    }

};

#endif

