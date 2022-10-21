/*
 * File:   BTree.h
 * Author: Miles Lacey
 *
 * Created on June 19, 2013, 12:25 AM
 */

#ifndef LS_UTILS_BTREE_H
#define LS_UTILS_BTREE_H

#include <utility> // std::move(...)

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Bits.h"



namespace ls
{
namespace utils
{



/**
 *  @brief BTree Node
 *  A node type used by the BTree class in order to store data.
 */
template <typename data_t>
class BTreeNode
{
  private:
    template <typename, typename>
    friend class BTree;

    /**
     *  @brief bnode_dir_t
     *  A simple enumeration to help determine where within a btree-node a child
     *  node should be placed.
     */
    enum : unsigned
    {
        BNODE_LEFT = 0,
        BNODE_RIGHT = 1,
        BNODE_MAX
    };

    /**
     *  @brief data
     *  A pointer to a dynamically-allocated object.
     */
    data_t* data;

    /**
     *  @brief subNodes
     *  A pointer to a dynamic array of child nodes.
     */
    BTreeNode* subNodes;

  public:
    /**
     *  @brief Destructor
     *  Frees all memory/resources used by *this.
     */
    ~BTreeNode() noexcept;

    /**
     *  @brief Constructor
     *  Creates a b-tree node with no data within.
     */
    constexpr BTreeNode() noexcept;



    /**
     *  @brief B-Tree Node Copu Operator
     *  Copies all data from the input parameter into *this.
     *
     *  @param btn
     *  A constant reference to a b-tree node.
     */
    BTreeNode(const BTreeNode& btn) noexcept;

    /**
     *  @brief B-Tree Node Move Operator
     *  Moves all data from the input parameter into *this.
     *
     *  @param btn
     *  An r-value reference to a b-tree node that will to go out of
     *  scope.
     */
    BTreeNode(BTreeNode&& btn) noexcept;

    /**
     *  @brief B-Tree Node Copu Operator
     *  Copies all data from the input parameter into *this.
     *
     *  @param btn
     *  A constant reference to a b-tree node.
     *
     *  @return a reference to *this.
     */
    BTreeNode& operator=(const BTreeNode& btn) noexcept;

    /**
     *  @brief B-Tree Node Move Operator
     *  Moves all data from the input parameter into *this.
     *
     *  @param btn
     *  An r-value reference to a b-tree node that will to go out of
     *  scope.
     *
     *  @return a reference to *this.
     */
    BTreeNode& operator=(BTreeNode&& btn) noexcept;
};



/*-----------------------------------------------------------------------------
 * B-Tree Class
 * --------------------------------------------------------------------------*/
/**
 *  @brief B-Tree
 *
 *  A simple tree container that allows for fast lookup of data.
 *
 *  @todo
 *  Add iterators, reduce usage of the new operator.
 */
template <typename key_t, typename data_t>
class BTree
{
  private:
    /**
     *  @brief The head tree which contains all child nodes and data.
     */
    BTreeNode<data_t> head = BTreeNode<data_t>{};

    /**
     *  @brief Number of child nodes, not including the head node.
     */
    unsigned numNodes = 0;

    /**
     *  @brief Iterates through the list of child nodes and returns
     * whichever  node is referenced by a key.
     *
     *  @param k
     *  A key which indicates the child node that should be iterated to.
     *
     *  @param createNodes
     *  A boolean flag to determine if a child node should be created if
     *  one does not exist at they key 'k.'
     *
     *  @return
     *  A pointer to a child node, referenced by 'k,' or NULL if one does
     *  not exist.
     */
    const BTreeNode<data_t>* iterate(const key_t* k) const noexcept;

    /**
     *  @brief Iterates through the list of child nodes and returns
     * whichever  node is referenced by a key.
     *
     *  @param k
     *  A key which indicates the child node that should be iterated to.
     *
     *  @param createNodes
     *  A boolean flag to determine if a child node should be created if
     *  one does not exist at they key 'k.'
     *
     *  @return
     *  A pointer to a child node, referenced by 'k,' or NULL if one does
     *  not exist.
     */
    BTreeNode<data_t>* iterate(const key_t* k, bool createNodes) noexcept;

  public:
    /**
     *  @brief destructor
     *
     *  Clears all data and resources used by *this.
     */
    ~BTree() noexcept;

    /**
     *  @brief constructor
     *
     *  Creates an empty tree with no child nodes.
     */
    constexpr BTree() noexcept;

    /**
     *  @brief Copy constructor.
     *
     *  @param tree
     *  A btree with data to be copied into *this.
     */
    BTree(const BTree& tree) noexcept;

    /**
     *  @brief Move constructor
     *
     *  Moves all data from the input parameter into *this without any
     *  copies.
     *
     *  @param tree
     *  An r-value reference to a temporary tree.
     */
    BTree(BTree&& tree) noexcept;

    /**
     *  @brief copy operator
     *
     *  @param tree
     *  A btree with data to be copied into *this.
     *
     *  @return a reference to *this.
     */
    BTree& operator=(const BTree& tree) noexcept;

    /**
     *  @brief move operator
     *
     *  Moves all data from the input parameter into *this without any
     *  copies.
     *
     *  @param tree
     *  An r-value reference to a temporary tree.
     *
     *  @return a reference to *this.
     */
    BTree& operator=(BTree&& tree) noexcept;

    /**
     *  @brief subscript operator (const)
     *
     *  Iterates through the tree of nodes and returns the data referenced
     *  by a key. This operator behaves just like an std::map, where if an
     *  object does not exist at the specified key, one will be created.
     *
     *  @param k
     *  A key used to reference a specific object in *this.
     *
     *  @return a reference to a specific piece of data referenced by 'k.'
     */
    const data_t& operator[](const key_t& k) const noexcept;

    /**
     *  @brief subscript operator
     *
     *  Iterates through the tree of nodes and returns the data referenced
     *  by a key. This operator behaves just like an std::map, where if an
     *  object does not exist at the specified key, one will be created.
     *
     *  @param k
     *  A key used to reference a specific object in *this.
     *
     *  @return a reference to a specific piece of data referenced by 'k.'
     */
    data_t& operator[](const key_t& k) noexcept;

    /**
     *  @brief Insert a piece of data into *this, referencing it by a key.
     *
     *  @param k
     *  The key that will be used to reference the inserted data.
     *
     *  @param d
     *  The data that will be inserted into *this.
     */
    void emplace(const key_t& k, data_t&& d) noexcept;

    /**
     *  @brief Insert a piece of data into *this, referencing it by a key.
     *
     *  @param k
     *  The key that will be used to reference the inserted data.
     *
     *  @param d
     *  The data that will be inserted into *this.
     */
    void push(const key_t& k, const data_t& d) noexcept;

    /**
     *  @brief Delete an object contained within *this.
     *
     *  @param k
     *  The key that will be used to reference the data to be deleted.
     */
    void pop(const key_t& k) noexcept;

    /**
     *  @brief Check to see if there is data within the tree, referenced by
     * a key.
     *
     *  @param k
     *  The key that will be used to check for the existence of data in
     *  *this.
     *
     *  @return TRUE if there is data referenced by the key 'k,' FALSE if
     *  not.
     */
    bool contains(const key_t& k) const noexcept;

    /**
     *  @brief Get a reference to the data that's referenced by a key.
     *
     *  @param k
     *  The key that will be used to check for the existence of data in
     *  *this.
     *
     *  @return a reference to the data referenced by 'k.'
     */
    const data_t& at(const key_t& k) const noexcept;

    /**
     *  @brief Get a reference to the data that's referenced by a key.
     *
     *  @param k
     *  The key that will be used to check for the existence of data in
     *  *this.
     *
     *  @return a reference to the data referenced by 'k.'
     */
    data_t& at(const key_t& k) noexcept;

    /**
     *  @brief Get the number of nodes contained within *this.
     *
     *  @return the number of objects inserted into *this.
     */
    unsigned size() const noexcept;

    /**
     *  @brief Frees all objects and dynamic memory from *this.
     */
    void clear() noexcept;
};



} // end utils namespace
} // end ls namespace

#include "lightsky/utils/generic/BTreeImpl.hpp"

#endif  /* LS_UTILS_BTREE_H */
