
namespace ls {
namespace utils {

/*-------------------------------------
    B-Tree Constructor
-------------------------------------*/
template <typename key_t, typename data_t>
constexpr bTree<key_t, data_t>::bTree() :
    head{},
    numNodes{0}
{}

/*-------------------------------------
    B-Tree Copy Constructor
-------------------------------------*/
template <typename key_t, typename data_t>
bTree<key_t, data_t>::bTree(const bTree& bt) :
    head{ bt.head },
    numNodes{ bt.numNodes }
{}

/*-------------------------------------
    B-Tree Move Constructor
-------------------------------------*/
template <typename key_t, typename data_t>
bTree<key_t, data_t>::bTree(bTree&& bt) {
    head.subNodes = bt.head.subNodes;
    bt.head.subNodes = nullptr;
    
    numNodes = bt.numNodes;
    bt.numNodes = 0;
}

/*-------------------------------------
    B-Tree Copy Operator
-------------------------------------*/
template <typename key_t, typename data_t>
bTree<key_t, data_t>& bTree<key_t, data_t>::operator =(const bTree& bt) {
    head = bt.head;
    numNodes = bt.numNodes;
    return *this;
}

/*-------------------------------------
    B-Tree Move Operator
-------------------------------------*/
template <typename key_t, typename data_t>
bTree<key_t, data_t>& bTree<key_t, data_t>::operator =(bTree&& bt) {
    head.subNodes = bt.head.subNodes;
    bt.head.subNodes = nullptr;
    
    numNodes = bt.numNodes;
    bt.numNodes = 0;
    
    return *this;
}

/*-------------------------------------
    B-Tree Element iteration
-------------------------------------*/
template <typename key_t, typename data_t>
const bTreeNode<data_t>* bTree<key_t, data_t>::iterate(const key_t* k) const {
    
    unsigned bytePos = 0;
    const bTreeNode<data_t>* bNodeIter = &head;
    const utils::bitMask* byteIter = nullptr;
    
    while ((byteIter = ls::utils::getByte<key_t>(k, bytePos++))) {
        
        for (unsigned currBit = LS_BITS_PER_BYTE; currBit--;) {

            // check to see if a new bTreeNode needs to be made
            if (!bNodeIter->subNodes) {
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
    B-Tree Element iteration
-------------------------------------*/
template <typename key_t, typename data_t>
bTreeNode<data_t>* bTree<key_t, data_t>::iterate(const key_t* k, bool createNodes) {
    
    unsigned                bytePos     = 0;
    bTreeNode<data_t>*      bNodeIter   = &head;
    const utils::bitMask*   byteIter    = nullptr;
    
    while ((byteIter = ls::utils::getByte<key_t>(k, bytePos++))) {
        
        for (unsigned currBit = LS_BITS_PER_BYTE; currBit--;) {

            // check to see if a new bTreeNode needs to be made
            if (!bNodeIter->subNodes) {
                if (createNodes) {
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
    B-Tree Destructor
-------------------------------------*/
template <typename key_t, typename data_t>
void bTree<key_t, data_t>::clear() {
    delete head.data;
    delete [] head.subNodes;
    
    head.data = nullptr;
    head.subNodes = nullptr;
    numNodes = 0;
}

/*-------------------------------------
    B-Tree Node Array Subscript operator
-------------------------------------*/
template <typename key_t, typename data_t>
const data_t& bTree<key_t, data_t>::operator [](const key_t& k) const {
    const bTreeNode<data_t>* const iter = iterate(&k);
    
    if (!iter || !iter->data) {
        throw ls::utils::error_t::LS_ERROR;
    }
    
    return *iter->data;
}

/*-------------------------------------
    B-Tree Node Array Subscript operator
-------------------------------------*/
template <typename key_t, typename data_t>
data_t& bTree<key_t, data_t>::operator [](const key_t& k) {
    bTreeNode<data_t>* const iter = iterate(&k, true);
    
    if (!iter->data) {
        iter->data = new data_t{};
        ++numNodes;
    }
    
    return *iter->data;
}

/*-------------------------------------
    B-Tree Emplace
-------------------------------------*/
template <typename key_t, typename data_t>
void bTree<key_t, data_t>::emplace(const key_t& k, data_t&& d) {
    bTreeNode<data_t>* const iter = iterate(&k, true);
    
    if (!iter->data) {
        iter->data = new data_t{std::move(d)};
        ++numNodes;
    }
    else {
        *iter->data = std::move(d);
    }
}

/*-------------------------------------
    B-Tree Push
-------------------------------------*/
template <typename key_t, typename data_t>
void bTree<key_t, data_t>::push(const key_t& k, const data_t& d) {
    bTreeNode<data_t>* const iter = iterate(&k, true);
    
    if (!iter->data) {
        iter->data = new data_t{d};
        ++numNodes;
    }
    else {
        *iter->data = d;
    }
}

/*-------------------------------------
    B-Tree Pop
-------------------------------------*/
template <typename key_t, typename data_t>
void bTree<key_t, data_t>::pop(const key_t& k) {
    bTreeNode<data_t>* const iter = iterate(&k, false);

    if (!iter || !iter->data) {
        return;
    }

    delete iter->data;
    iter->data = nullptr;
    --numNodes;
}

/*-------------------------------------
    B-Tree Has Data
-------------------------------------*/
template <typename key_t, typename data_t>
bool bTree<key_t, data_t>::contains(const key_t& k) const {
    const bTreeNode<data_t>* const iter = iterate(&k);
    return iter && iter->data;
}

/*-------------------------------------
    B-Tree Push
-------------------------------------*/
template <typename key_t, typename data_t>
const data_t& bTree<key_t, data_t>::at(const key_t& k) const {
    const bTreeNode<data_t>* const iter = iterate(&k);
    
    if (!iter || !iter->data) {
        throw ls::utils::error_t::LS_ERROR;
    }
    
    return *iter->data;
}

/*-------------------------------------
    B-Tree Push
-------------------------------------*/
template <typename key_t, typename data_t>
data_t& bTree<key_t, data_t>::at(const key_t& k) {
    bTreeNode<data_t>* const iter = iterate(&k, false);
    
    if (!iter || !iter->data) {
        throw ls::utils::error_t::LS_ERROR;
    }
    
    return *iter->data;
}

} // end utils namespace
} // end ls namespace
