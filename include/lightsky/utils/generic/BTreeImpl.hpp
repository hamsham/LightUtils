/*
 * File:   BTreeImpl.hpp
 * Author: hammy
 *
 * Created on Oct 16, 2022 at 2:42 PM
 */

#ifndef LS_UTILS_BTREE_IMPL_HPP
#define LS_UTILS_BTREE_IMPL_HPP

namespace ls
{
namespace utils
{



/*-----------------------------------------------------------------------------
 * B-Tree Node
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * B-Tree Node Destructor
 * ----------------------------------*/
template <typename data_t>
BTreeNode<data_t>::~BTreeNode() noexcept
{
    delete data;
    data = nullptr;

    delete[] subNodes;
    subNodes = nullptr;
}



/*-------------------------------------
 * B-Tree Node Constructor
 * ----------------------------------*/
template <typename data_t>
constexpr
BTreeNode<data_t>::BTreeNode() noexcept :
    data{nullptr},
    subNodes{nullptr}
{
}



/*-------------------------------------
 * B-Tree Node Copy Constructor
 * ----------------------------------*/
template <typename data_t>
BTreeNode<data_t>::BTreeNode(const BTreeNode& btn) noexcept :
    data{btn.data != nullptr ? new data_t(*btn.data) : nullptr},
    subNodes{nullptr}
{
    if (data != nullptr && btn.subNodes != nullptr)
    {
        subNodes = new BTreeNode[BNODE_MAX];
        subNodes[BNODE_LEFT] = btn.subNodes[BNODE_LEFT];
        subNodes[BNODE_RIGHT] = btn.subNodes[BNODE_RIGHT];
    }
}



/*-------------------------------------
 * B-Tree Node Move Constructor
 * ----------------------------------*/
template <typename data_t>
BTreeNode<data_t>::BTreeNode(BTreeNode&& btn) noexcept :
    data{btn.data},
    subNodes{btn.subNodes}
{
    btn.data = nullptr;
    btn.subNodes = nullptr;
}



/*-------------------------------------
 * B-Tree Node Copy Operator
 * ----------------------------------*/
template <typename data_t>
BTreeNode<data_t>& BTreeNode<data_t>::operator=(const BTreeNode& btn) noexcept
{
    if (this == &btn)
    {
        return *this;
    }

    delete data;
    if (btn.data)
    {
        data = new data_t;
        *data = *btn.data;
    }
    else
    {
        data = nullptr;
    }

    delete subNodes;
    if (btn.subNodes)
    {
        subNodes = new BTreeNode[BNODE_MAX];
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
BTreeNode<data_t>& BTreeNode<data_t>::operator=(BTreeNode&& btn) noexcept
{
    if (this != &btn)
    {
        data = btn.data;
        btn.data = nullptr;

        subNodes = btn.subNodes;
        btn.subNodes = nullptr;
    }
    
    return *this;
}



/*-----------------------------------------------------------------------------
 * B-Tree Member Functions
 * --------------------------------------------------------------------------*/
/*-------------------------------------
 * B-Tree Constructor
 * ----------------------------------*/
template <typename key_t, typename data_t>
BTree<key_t, data_t>::~BTree() noexcept
{
}



/*-------------------------------------
 * B-Tree Constructor
 * ----------------------------------*/
template <typename key_t, typename data_t>
constexpr BTree<key_t, data_t>::BTree() noexcept :
    head{},
    numNodes{0}
{
}



/*-------------------------------------
 * B-Tree Copy Constructor
 * ----------------------------------*/
template <typename key_t, typename data_t>
BTree<key_t, data_t>::BTree(const BTree& bt) noexcept :
    head{bt.head},
    numNodes{bt.numNodes}
{
}



/*-------------------------------------
 * B-Tree Move Constructor
 * ----------------------------------*/
template <typename key_t, typename data_t>
BTree<key_t, data_t>::BTree(BTree&& bt) noexcept
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
BTree <key_t, data_t>& BTree<key_t, data_t>::operator=(const BTree& bt) noexcept
{
    if (this != &bt)
    {
        head = bt.head;
        numNodes = bt.numNodes;
    }

    return *this;
}



/*-------------------------------------
 * B-Tree Move Operator
 * ----------------------------------*/
template <typename key_t, typename data_t>
BTree <key_t, data_t>& BTree<key_t, data_t>::operator=(BTree&& bt) noexcept
{
    if (this != &bt)
    {
        head.subNodes = bt.head.subNodes;
        bt.head.subNodes = nullptr;

        numNodes = bt.numNodes;
        bt.numNodes = 0;
    }

    return *this;
}



/*-------------------------------------
 * B-Tree Element iteration
 * ----------------------------------*/
template <typename key_t, typename data_t>
const BTreeNode<data_t>* BTree<key_t, data_t>::iterate(const key_t* k) const noexcept
{
    unsigned bytePos = 0;
    const BTreeNode<data_t>* bNodeIter = &head;
    const utils::BitMask* byteIter = nullptr;

    while ((byteIter = ls::utils::get_byte<key_t>(k, bytePos++)))
    {
        for (unsigned currBit = LS_BITS_PER_BYTE; currBit--;)
        {
            // check to see if a new BTreeNode needs to be made
            if (!bNodeIter->subNodes)
            {
                return nullptr;
            }

            // move to the next BTreeNode
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
BTreeNode<data_t>* BTree<key_t, data_t>::iterate(const key_t* k, bool createNodes) noexcept
{
    unsigned bytePos = 0;
    BTreeNode<data_t>* bNodeIter = &head;
    const utils::BitMask* byteIter = nullptr;

    while ((byteIter = ls::utils::get_byte<key_t>(k, bytePos++)))
    {
        for (unsigned currBit = LS_BITS_PER_BYTE; currBit--;)
        {
            // check to see if a new BTreeNode needs to be made
            if (!bNodeIter->subNodes)
            {
                if (createNodes)
                {
                    // create and initialize the upcoming sub BTreeNode
                    bNodeIter->subNodes = new BTreeNode<data_t>[BTreeNode<data_t>::BNODE_MAX];
                }
                else
                {
                    return nullptr;
                }
            }

            // move to the next BTreeNode
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
void BTree<key_t, data_t>::clear() noexcept
{
    delete head.data;
    delete[] head.subNodes;

    head.data = nullptr;
    head.subNodes = nullptr;
    numNodes = 0;
}



/*-------------------------------------
 * B-Tree Node Array Subscript operator
 * ----------------------------------*/
template <typename key_t, typename data_t>
const data_t& BTree<key_t, data_t>::operator[](const key_t& k) const noexcept
{
    const BTreeNode<data_t>* const iter = iterate(&k);

    LS_ASSERT(iter && iter->data);

    return *iter->data;
}



/*-------------------------------------
 * B-Tree Node Array Subscript operator
 * ----------------------------------*/
template <typename key_t, typename data_t>
data_t& BTree<key_t, data_t>::operator[](const key_t& k) noexcept
{
    BTreeNode<data_t>* const iter = iterate(&k, true);

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
void BTree<key_t, data_t>::emplace(const key_t& k, data_t&& d) noexcept
{
    BTreeNode<data_t>* const iter = iterate(&k, true);

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
void BTree<key_t, data_t>::push(const key_t& k, const data_t& d) noexcept
{
    BTreeNode<data_t>* const iter = iterate(&k, true);

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
void BTree<key_t, data_t>::pop(const key_t& k) noexcept
{
    BTreeNode<data_t>* const iter = iterate(&k, false);

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
bool BTree<key_t, data_t>::contains(const key_t& k) const noexcept
{
    const BTreeNode<data_t>* const iter = iterate(&k);
    return iter && iter->data;
}



/*-------------------------------------
 * B-Tree Push
 * ----------------------------------*/
template <typename key_t, typename data_t>
const data_t& BTree<key_t, data_t>::at(const key_t& k) const noexcept
{
    const BTreeNode<data_t>* const iter = iterate(&k);

    LS_ASSERT(iter && iter->data);

    return *iter->data;
}



/*-------------------------------------
 * B-Tree Size
 * ----------------------------------*/
template <typename key_t, typename data_t>
inline
unsigned BTree<key_t, data_t>::size() const noexcept
{
    return numNodes;
}



/*-------------------------------------
 * B-Tree Push
 * ----------------------------------*/
template <typename key_t, typename data_t>
data_t& BTree<key_t, data_t>::at(const key_t& k) noexcept
{
    BTreeNode<data_t>* const iter = iterate(&k, false);

    LS_ASSERT(iter && iter->data);

    return *iter->data;
}



} // end utils namespace
} // end ls namespace

#endif /* LS_UTILS_BTREE_IMPL_HPP */
