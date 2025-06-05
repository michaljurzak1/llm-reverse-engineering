#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Node structure
typedef struct TreeNode {
    int data;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

// Tree structure
typedef struct {
    TreeNode* root;
    size_t size;
} BinaryTree;

// Function to create a new node
TreeNode* create_node(int data) {
    TreeNode* new_node = (TreeNode*)malloc(sizeof(TreeNode));
    if (!new_node) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    new_node->data = data;
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

// Function to initialize a binary tree
BinaryTree* create_tree() {
    BinaryTree* tree = (BinaryTree*)malloc(sizeof(BinaryTree));
    if (!tree) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    tree->root = NULL;
    tree->size = 0;
    return tree;
}

// Function to insert a node recursively
TreeNode* insert_recursive(TreeNode* node, int data) {
    if (node == NULL) {
        return create_node(data);
    }
    
    if (data < node->data) {
        node->left = insert_recursive(node->left, data);
    } else if (data > node->data) {
        node->right = insert_recursive(node->right, data);
    }
    
    return node;
}

// Function to insert a node
void insert(BinaryTree* tree, int data) {
    tree->root = insert_recursive(tree->root, data);
    tree->size++;
}

// Function to find minimum value node
TreeNode* find_min(TreeNode* node) {
    TreeNode* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

// Function to delete a node recursively
TreeNode* delete_recursive(TreeNode* node, int data) {
    if (node == NULL) {
        return node;
    }
    
    if (data < node->data) {
        node->left = delete_recursive(node->left, data);
    } else if (data > node->data) {
        node->right = delete_recursive(node->right, data);
    } else {
        // Node with only one child or no child
        if (node->left == NULL) {
            TreeNode* temp = node->right;
            free(node);
            return temp;
        } else if (node->right == NULL) {
            TreeNode* temp = node->left;
            free(node);
            return temp;
        }
        
        // Node with two children
        TreeNode* temp = find_min(node->right);
        node->data = temp->data;
        node->right = delete_recursive(node->right, temp->data);
    }
    return node;
}

// Function to delete a node
void delete(BinaryTree* tree, int data) {
    tree->root = delete_recursive(tree->root, data);
    tree->size--;
}

// Function to perform inorder traversal
void inorder_traversal(TreeNode* node) {
    if (node != NULL) {
        inorder_traversal(node->left);
        printf("%d ", node->data);
        inorder_traversal(node->right);
    }
}

// Function to perform preorder traversal
void preorder_traversal(TreeNode* node) {
    if (node != NULL) {
        printf("%d ", node->data);
        preorder_traversal(node->left);
        preorder_traversal(node->right);
    }
}

// Function to perform postorder traversal
void postorder_traversal(TreeNode* node) {
    if (node != NULL) {
        postorder_traversal(node->left);
        postorder_traversal(node->right);
        printf("%d ", node->data);
    }
}

// Function to search for a value
TreeNode* search(TreeNode* node, int data) {
    if (node == NULL || node->data == data) {
        return node;
    }
    
    if (data < node->data) {
        return search(node->left, data);
    }
    
    return search(node->right, data);
}

// Function to calculate tree height
int tree_height(TreeNode* node) {
    if (node == NULL) {
        return -1;
    }
    
    int left_height = tree_height(node->left);
    int right_height = tree_height(node->right);
    
    return (left_height > right_height ? left_height : right_height) + 1;
}

// Function to free the tree
void free_tree(TreeNode* node) {
    if (node != NULL) {
        free_tree(node->left);
        free_tree(node->right);
        free(node);
    }
}

int main() {
    BinaryTree* tree = create_tree();
    
    // Insert some values
    insert(tree, 50);
    insert(tree, 30);
    insert(tree, 70);
    insert(tree, 20);
    insert(tree, 40);
    insert(tree, 60);
    insert(tree, 80);
    
    printf("Tree traversals:\n");
    printf("Inorder: ");
    inorder_traversal(tree->root);
    printf("\n");
    
    printf("Preorder: ");
    preorder_traversal(tree->root);
    printf("\n");
    
    printf("Postorder: ");
    postorder_traversal(tree->root);
    printf("\n");
    
    printf("\nTree height: %d\n", tree_height(tree->root));
    
    printf("\nSearching for value 40: ");
    if (search(tree->root, 40)) {
        printf("Found\n");
    } else {
        printf("Not found\n");
    }
    
    printf("\nDeleting node with value 30:\n");
    delete(tree, 30);
    printf("Inorder after deletion: ");
    inorder_traversal(tree->root);
    printf("\n");
    
    // Clean up
    free_tree(tree->root);
    free(tree);
    
    return 0;
} 