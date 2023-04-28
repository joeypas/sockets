#ifndef FILE_TREE_HPP
#define FILE_TREE_HPP

#include <string>
#include <vector>
#include <stack>
#include <memory>
#include <optional>
#include <filesystem>
#include <functional>
#include <exception>
#include <cppcoro/recursive_generator.hpp>
#include <cppcoro/generator.hpp>

using namespace std;

namespace fs = std::filesystem;

/**
 * Represents a file in a directory structure
 * May have a parent node or children nodes
*/
struct FileNode : public enable_shared_from_this<FileNode> {
    shared_ptr<FileNode> parent;
    bool has_children;
    optional<vector<shared_ptr<FileNode>>> children;
    fs::path path;

    FileNode() : has_children(false), children(nullopt) {}
    FileNode(fs::path p, FileNode *parent = nullptr) : path(p) {
        if (parent != nullptr) this->parent = shared_ptr<FileNode>(parent);
        else this->parent = nullptr;


        if (fs::is_directory(path)) {
            has_children = true;
            vector<shared_ptr<FileNode>> vec;
            children = make_optional<vector<shared_ptr<FileNode>>>(vec);
            getChildren(this);
        } else if (!fs::is_empty(path)) {
            has_children = false;
            children = nullopt;
        }
    }

    // Finds children and constructs nodes from them
    void getChildren(FileNode *node) {
        for (auto const dir_entry : fs::directory_iterator(node->path)) {
            node->children->push_back(make_shared<FileNode>(dir_entry, node));
        }
    }

    // Getter for parent pointer
    shared_ptr<FileNode> getParent() {
        return parent->shared_from_this();
    }

};


/**
 * Represents a File tree with a root node
*/
class FileTree {
private:
    FileNode root;

public:
    FileTree(fs::path base_file) : root(base_file) {}

    cppcoro::generator<FileNode*> getNodes(FileNode& parent) {
        stack<FileNode*> stack_;
        stack_.push(&parent);
        while (!stack_.empty()) {
            FileNode* node = stack_.top();
            stack_.pop();

            if (!node->has_children) {
                co_yield node;
            } else {
                for (auto& child : *node->children) {
                    stack_.push(child.get());
                }
            }
        }
    }
    
    void fileAction(function<void(FileNode*)> action) {
        for (auto f : getNodes(root)) {
            action(f);
        }
    }

    /**
     * Getter function for pointer to root node
     * 
     * @return ptr for root node
     */
    FileNode* getRoot() {
        return new FileNode(root);
    }
};

#endif

