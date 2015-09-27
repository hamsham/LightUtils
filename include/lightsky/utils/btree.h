/*
 * File:   bTree.h
 * Author: Miles Lacey
 *
 * Created on June 19, 2013, 12:25 AM
 */

#ifndef __LS_UTILS_BTREE__
#define __LS_UTILS_BTREE__

#include <utility> // std::move(...)

#include "lightsky/utils/bits.h"

namespace ls {
namespace utils {

/**
 *  @brief bnode_dir_t
 *  A simple enumeration to help determine where within a btree-node a child
 *  node should be placed.
 */
enum bnode_dir_t : unsigned
{
    BNODE_LEFT   = 0,
    BNODE_RIGHT  = 1,
    BNODE_MAX
};

/**
 *  @brief BTree Node
 *  A node type used by the BTree class in order to store data.
 */
template <typename data_t>
class bTreeNode
{
    private:
        template <typename, typename>
        friend
        class bTree;

        /**
         *  @brief data
         *  A pointer to a dynamically-allocated object.
         */
        data_t* data = nullptr;

        /**
         *  @brief subNodes
         *  A pointer to a dynamic array of child nodes.
         */
        bTreeNode* subNodes = nullptr;

    public:
        /**
         *  @brief Constructor
         *  Creates a b-tree node with no data within.
         */
        constexpr
        bTreeNode();

        /**
         *  @brief B-Tree Node Copu Operator
         *  Copies all data from the input parameter into *this.
         *
         *  @param btn
         *  A constant reference to a b-tree node.
         */
        bTreeNode(const bTreeNode& btn);

        /**
         *  @brief B-Tree Node Move Operator
         *  Moves all data from the input parameter into *this.
         *
         *  @param btn
         *  An r-value reference to a b-tree node that will to go out of
         *  scope.
         */
        bTreeNode(bTreeNode&& btn);

        /**
         *  @brief Destructor
         *  Frees all memory/resources used by *this.
         */
        ~bTreeNode();

        /**
         *  @brief B-Tree Node Copu Operator
         *  Copies all data from the input parameter into *this.
         *
         *  @param btn
         *  A constant reference to a b-tree node.
         *
         *  @return a reference to *this.
         */
        bTreeNode& operator=(const bTreeNode& btn);

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
        bTreeNode& operator=(bTreeNode&& btn);
};

/*-------------------------------------
 * B-Tree Node Constructor
 * ----------------------------------*/
template <typename data_t>
constexpr
bTreeNode<data_t>::bTreeNode()
{}

/*-------------------------------------
 * B-Tree Node Copy Constructor
 * ----------------------------------*/
template <typename data_t>
bTreeNode<data_t>::bTreeNode(const bTreeNode& btn)
:
    data{
        btn.data != nullptr
        ?
            new data_t(*btn.data)
        :
            nullptr
    },
    subNodes{nullptr}
{
    if (btn.subNodes != nullptr)
    {
        subNodes = new bTreeNode[BNODE_MAX];
        subNodes[BNODE_LEFT] = btn.subNodes[BNODE_LEFT];
        subNodes[BNODE_RIGHT] = btn.subNodes[BNODE_RIGHT];
    }
}

/*-------------------------------------
 * B-Tree Node Move Constructor
 * ----------------------------------*/
template <typename data_t>
bTreeNode<data_t>::bTreeNode(bTreeNode&& btn)
:
    data{btn.data},
    subNodes{btn.subNodes}
{
    btn.data = nullptr;
    btn.subNodes = nullptr;
}

/*-------------------------------------
 * B-Tree Node Destructor
 * ----------------------------------*/
template <typename data_t>
bTreeNode<data_t>::~bTreeNode()
{
    delete data;
    data = nullptr;

    delete [] subNodes;
    subNodes = nullptr;
}

/*-------------------------------------
 * B-Tree Node Copy Operator
 * ----------------------------------*/
template <typename data_t>
bTreeNode<data_t>& bTreeNode<data_t>::operator=(const bTreeNode& btn)
{
    delete data;
    if (btn.data)
    {
        data = new data_t;
        *data = *btn.data;
    }
    else {
        data = nullptr;
    }

    delete subNodes;
    if (btn.subNodes)
    {
        subNodes = new bTreeNode[BNODE_MAX];
        subNodes[BNODE_LEFT] = btn.subNodes[BNODE_LEFT];
        subNodes[BNODE_RIGHT] = btn.subNodes[BNODE_RIGHT];
    }
    else
    {
        subNodes = nullptr;
    }
    return *this;
}

/*-------------------------------------
 * B-Tree Node Move Operator
 * ----------------------------------*/
template <typename data_t>
bTreeNode<data_t>& bTreeNode<data_t>::operator=(bTreeNode&& btn)
{
    data = btn.data;
    btn.data = nullptr;

    subNodes = btn.subNodes;
    btn.subNodes = nullptr;
}

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
class bTree
{
    private:
        /**
         *  @brief The head tree which contains all child nodes and data.
         */
        bTreeNode<data_t> head = bTreeNode<data_t>{};

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
        const bTreeNode<data_t>* iterate(const key_t* k) const;

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
        bTreeNode<data_t>* iterate(const key_t* k, bool createNodes);

    public:
        /**
         *  @brief constructor
         *
         *  Creates an empty tree with no child nodes.
         */
        constexpr
        bTree();

        /**
         *  @brief Copy constructor.
         *
         *  @param tree
         *  A btree with data to be copied into *this.
         */
        bTree(const bTree& tree);

        /**
         *  @brief Move constructor
         *
         *  Moves all data from the input parameter into *this without any
         *  copies.
         *
         *  @param tree
         *  An r-value reference to a temporary tree.
         */
        bTree(bTree&& tree);

        /**
         *  @brief destructor
         *
         *  Clears all data and resources used by *this.
         */
        ~bTree() {}

        /**
         *  @brief copy operator
         *
         *  @param tree
         *  A btree with data to be copied into *this.
         *
         *  @return a reference to *this.
         */
        bTree& operator=(const bTree& tree);

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
        bTree& operator=(bTree&& tree);

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
        const data_t& operator [] (const key_t& k) const;

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
        data_t& operator [] (const key_t& k);

        /**
         *  @brief Insert a piece of data into *this, referencing it by a key.
         *
         *  @param k
         *  The key that will be used to reference the inserted data.
         *
         *  @param d
         *  The data that will be inserted into *this.
         */
        void emplace(const key_t& k, data_t&& d);

        /**
         *  @brief Insert a piece of data into *this, referencing it by a key.
         *
         *  @param k
         *  The key that will be used to reference the inserted data.
         *
         *  @param d
         *  The data that will be inserted into *this.
         */
        void push(const key_t& k, const data_t& d);

        /**
         *  @brief Delete an object contained within *this.
         *
         *  @param k
         *  The key that will be used to reference the data to be deleted.
         */
        void pop(const key_t& k);

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
        bool contains(const key_t& k) const;

        /**
         *  @brief Get a reference to the data that's referenced by a key.
         *
         *  @param k
         *  The key that will be used to check for the existence of data in
         *  *this.
         *
         *  @return a reference to the data referenced by 'k.'
         */
        const data_t& at(const key_t& k) const;

        /**
         *  @brief Get a reference to the data that's referenced by a key.
         *
         *  @param k
         *  The key that will be used to check for the existence of data in
         *  *this.
         *
         *  @return a reference to the data referenced by 'k.'
         */
        data_t& at(const key_t& k);

        /**
         *  @brief Get the number of nodes contained within *this.
         *
         *  @return the number of objects inserted into *this.
         */
        inline
        unsigned size() const;

        /**
         *  @brief Frees all objects and dynamic memory from *this.
         */
        void clear();
};

/*-----------------------------------------------------------------------------
 * B-Tree Member Functions
 * --------------------------------------------------------------------------*/
/*-------------------------------------
 * B-Tree Constructor
 * ----------------------------------*/
template <typename key_t, typename data_t>
constexpr bTree<key_t, data_t>::bTree()
:
    head{},
    numNodes{0}
{}

/*-------------------------------------
 * B-Tree Copy Constructor
 * ----------------------------------*/
template <typename key_t, typename data_t>
bTree<key_t, data_t>::bTree(const bTree& bt)
:
    head{bt.head},
    numNodes{bt.numNodes}
{}

/*-------------------------------------
 * B-Tree Move Constructor
 * ----------------------------------*/
template <typename key_t, typename data_t>
bTree<key_t, data_t>::bTree(bTree&& bt)
{
    head.subNodes = bt.head.subNodes;
    bt.head.subNodes = nullptr;

    numNodes = bt.numNodes;
    bt.numNodes = 0;
}

/*-------------------------------------
 * B-Tree Copy Operator
 * ----------------------------------*/
template <typename key_t, typename data_t>
bTree<key_t, data_t>& bTree<key_t, data_t>::operator =(const bTree& bt)
{
    head = bt.head;
    numNodes = bt.numNodes;
    return *this;
}

/*-------------------------------------
 * B-Tree Move Operator
 * ----------------------------------*/
template <typename key_t, typename data_t>
bTree<key_t, data_t>& bTree<key_t, data_t>::operator =(bTree&& bt)
{
    head.subNodes = bt.head.subNodes;
    bt.head.subNodes = nullptr;

    numNodes = bt.numNodes;
    bt.numNodes = 0;

    return *this;
}

/*-------------------------------------
 * B-Tree Element iteration
 * ----------------------------------*/
template <typename key_t, typename data_t>
const bTreeNode<data_t>* bTree<key_t, data_t>::iterate(const key_t* k) const
{
    unsigned bytePos = 0;
    const bTreeNode<data_t>* bNodeIter = &head;
    const utils::bitMask* byteIter = nullptr;

    while ((byteIter = ls::utils::getByte<key_t>(k, bytePos++)))
    {
        for (unsigned currBit = LS_BITS_PER_BYTE; currBit--;)
        {
            // check to see if a new bTreeNode needs to be made
            if (!bNodeIter->subNodes)
            {
                return nullptr;
            }

            // move to the next bTreeNode
            const int dir = byteIter->get(currBit);
            bNodeIter = &(bNodeIter->subNodes[dir]);
        }
    }

    return bNodeIter;
}

/*-------------------------------------
 * B-Tree Element iteration
 * ----------------------------------*/
template <typename key_t, typename data_t>
bTreeNode<data_t>* bTree<key_t, data_t>::iterate(const key_t* k, bool createNodes)
{
    unsigned                bytePos     = 0;
    bTreeNode<data_t>*      bNodeIter   = &head;
    const utils::bitMask*   byteIter    = nullptr;

    while ((byteIter = ls::utils::getByte<key_t>(k, bytePos++)))
    {
        for (unsigned currBit = LS_BITS_PER_BYTE; currBit--;)
        {
            // check to see if a new bTreeNode needs to be made
            if (!bNodeIter->subNodes)
            {
                if (createNodes)
                {
                    // create and initialize the upcoming sub bTreeNode
                    bNodeIter->subNodes = new bTreeNode<data_t>[BNODE_MAX];
                }
                else {
                    return nullptr;
                }
            }

            // move to the next bTreeNode
            const int dir = byteIter->get(currBit);
            bNodeIter = &(bNodeIter->subNodes[dir]);
        }
    }

    return bNodeIter;
}

/*-------------------------------------
 * B-Tree Destructor
 * ----------------------------------*/
template <typename key_t, typename data_t>
void bTree<key_t, data_t>::clear()
{
    delete head.data;
    delete [] head.subNodes;

    head.data = nullptr;
    head.subNodes = nullptr;
    numNodes = 0;
}

/*-------------------------------------
 * B-Tree Node Array Subscript operator
 * ----------------------------------*/
template <typename key_t, typename data_t>
const data_t& bTree<key_t, data_t>::operator [](const key_t& k) const
{
    const bTreeNode<data_t>* const iter = iterate(&k);

    if (!iter || !iter->data)
    {
        throw ls::utils::error_t::LS_ERROR;
    }

    return *iter->data;
}

/*-------------------------------------
 * B-Tree Node Array Subscript operator
 * ----------------------------------*/
template <typename key_t, typename data_t>
data_t& bTree<key_t, data_t>::operator [](const key_t& k)
{
    bTreeNode<data_t>* const iter = iterate(&k, true);

    if (!iter->data)
    {
        iter->data = new data_t{};
        ++numNodes;
    }

    return *iter->data;
}

/*-------------------------------------
 * B-Tree Emplace
 * ----------------------------------*/
template <typename key_t, typename data_t>
void bTree<key_t, data_t>::emplace(const key_t& k, data_t&& d)
{
    bTreeNode<data_t>* const iter = iterate(&k, true);

    if (!iter->data)
    {
        iter->data = new data_t{std::move(d)};
        ++numNodes;
    }
    else
    {
        *iter->data = std::move(d);
    }
}

/*-------------------------------------
 * B-Tree Push
 * ----------------------------------*/
template <typename key_t, typename data_t>
void bTree<key_t, data_t>::push(const key_t& k, const data_t& d)
{
    bTreeNode<data_t>* const iter = iterate(&k, true);

    if (!iter->data)
    {
        iter->data = new data_t{d};
        ++numNodes;
    }
    else
    {
        *iter->data = d;
    }
}

/*-------------------------------------
 * B-Tree Pop
 * ----------------------------------*/
template <typename key_t, typename data_t>
void bTree<key_t, data_t>::pop(const key_t& k)
{
    bTreeNode<data_t>* const iter = iterate(&k, false);

    if (!iter || !iter->data)
    {
        return;
    }

    delete iter->data;

    iter->data = nullptr;

    --numNodes;
}

/*-------------------------------------
 * B-Tree Has Data
 * ----------------------------------*/
template <typename key_t, typename data_t>
bool bTree<key_t, data_t>::contains(const key_t& k) const
{
    const bTreeNode<data_t>* const iter = iterate(&k);
    return iter && iter->data;
}

/*-------------------------------------
 * B-Tree Push
 * ----------------------------------*/
template <typename key_t, typename data_t>
const data_t& bTree<key_t, data_t>::at(const key_t& k) const
{
    const bTreeNode<data_t>* const iter = iterate(&k);

    if (!iter || !iter->data) {
        throw ls::utils::error_t::LS_ERROR;
    }

    return *iter->data;
}

/*-------------------------------------
 * B-Tree Size
 * ----------------------------------*/
template <typename key_t, typename data_t>
inline
unsigned bTree<key_t, data_t>::size() const
{
    return numNodes;
}

/*-------------------------------------
 * B-Tree Push
 * ----------------------------------*/
template <typename key_t, typename data_t>
data_t& bTree<key_t, data_t>::at(const key_t& k)
{
    bTreeNode<data_t>* const iter = iterate(&k, false);

    if (!iter || !iter->data)
    {
        throw ls::utils::error_t::LS_ERROR;
    }

    return *iter->data;
}

} // end utils namespace
} // end ls namespace

#endif  /* __LS_UTILS_BTREE__ */
