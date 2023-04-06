#pragma once

#include <stdint.h>
#include <stdio.h>
#include <iostream>

#include <set>
#include <utility>

#include "common.h"

template <typename TKey, typename TData>
class AVLTree
{
public:
    typedef TKey KeyType;
    typedef TData DataType;

private:
    struct Node
    {
        Node *Parent;
        Node *Left;
        Node *Right;

        int8_t Balance;

        KeyType Key;
        DataType Data;

        /* constructor */ Node(void)
            : Parent(nullptr)
            , Left(nullptr)
            , Right(nullptr)
            , Balance(0)
            , Key()
            , Data()
        {
        }
    };

    Node m_root;
    Node *m_min;
    size_t m_count;

    inline Node *First(void) const { return m_root.Left; }

    Node *RotateLeft(Node *n)
    {
        Node *tmp = n->Right;

        n->Right = tmp->Left;

        if (n->Right)
            n->Right->Parent = n;

        tmp->Parent = n->Parent;

        if (n == n->Parent->Left)
            n->Parent->Left = tmp;
        else
            n->Parent->Right = tmp;

        tmp->Left = n;
        n->Parent = tmp;

        return tmp;
    }

    Node *RotateRight(Node *n)
    {
        Node *tmp = n->Left;

        n->Left = tmp->Right;

        if (n->Left)
            n->Left->Parent = n;

        tmp->Parent = n->Parent;

        if (n == n->Parent->Left)
            n->Parent->Left = tmp;
        else
            n->Parent->Right = tmp;

        tmp->Right = n;
        n->Parent = tmp;

        return tmp;
    }

    void FixInsertLeftImbalance(Node *node)
    {
        if (node->Left->Balance == node->Balance)
        {
            node = RotateRight(node);
            node->Balance = node->Right->Balance = 0;
        }
        else
        {
            int oldBalance = node->Left->Right->Balance;

            RotateLeft(node->Left);
            node = RotateRight(node);

            node->Balance = 0;

            switch (oldBalance)
            {
            case -1: // Right heavy
                node->Left->Balance = 0;
                node->Right->Balance = 1;
                break;

            case 0: // We now have perfect balance
                node->Left->Balance = node->Right->Balance = 0;
                break;
                
            case 1: // Left heavy
                node->Left->Balance = -1;
                node->Right->Balance = 0;
                break;
            }
        }
    }

    void FixInsertRightImbalance(Node *node)
    {
        if (node->Right->Balance == node->Balance)
        {
            node = RotateLeft(node);
            node->Balance = node->Left->Balance = 0;
        }
        else
        {
            int oldBalance = node->Right->Left->Balance;

            RotateRight(node->Right);
            node = RotateLeft(node);

            node->Balance = 0;

            switch (oldBalance)
            {
            case -1: // Right heavy
                node->Left->Balance = 0;
                node->Right->Balance = 1;
                break;

            case 0: // We now have perfect balance
                node->Left->Balance = node->Right->Balance = 0;
                break;

            case 1: // Left heavy
                node->Left->Balance = -1;
                node->Right->Balance = 0;
                break;
            }
        }
    }

    Node *FixDeleteLeftImbalance(Node *node)
    {
        switch (node->Left->Balance)
        {
        case -1:
            node = RotateRight(node);
            node->Balance = node->Right->Balance = 0;
            break;

        case 0:
            node = RotateRight(node);
            node->Balance = 1;
            node->Right->Balance = -1;
            break;

        case 1:
            int oldBalance = node->Left->Right->Balance;

            RotateLeft(node->Left);
            node = RotateRight(node);

            node->Balance = 0;

            switch (oldBalance)
            {
            case -1:
                node->Left->Balance = 0;
                node->Right->Balance = 1;
                break;

            case 0:
                node->Left->Balance = node->Right->Balance = 0;
                break;
                
            case 1:
                node->Left->Balance = -1;
                node->Right->Balance = 0;
                break;
            }
            break;
        }
        
        return node;
    }

    Node *FixDeleteRightImbalance(Node *node)
    {
        switch (node->Right->Balance)
        {
        case 1:
            node = RotateLeft(node);
            node->Balance = node->Left->Balance = 0;
            break;

        case 0:
            node = RotateLeft(node);
            node->Balance = -1;
            node->Left->Balance = 1;
            break;

        case -1:
            int oldBalance = node->Right->Left->Balance;

            RotateRight(node->Right);
            node = RotateLeft(node);

            node->Balance = 0;

            switch (oldBalance)
            {
            case -1:
                node->Left->Balance = 0;
                node->Right->Balance = 1;
                break;

            case 0:
                node->Left->Balance = node->Right->Balance = 0;
                break;

            case 1:
                node->Left->Balance = -1;
                node->Right->Balance = 0;
                break;
            }

            break;
        }

        return node;
    }

    Node *FindNode(const KeyType &value)
    {
        Node *rval = First();

        while (rval)
        {
            auto cmp = value <=> rval->Key;

            if (cmp == 0)
                break;

            rval = (cmp < 0) ? rval->Left : rval->Right;
        }

        return rval;
    }

    Node *Successor(Node *node)
    {
        Node *n = node->Right;

        if (n)
        {
            // Move down until we find what we need.
            for (; n->Left; n = n->Left)
                ;
        }
        else
        {
            // Move up until we find what we need.
            for (n = node->Parent; node == n->Right; node = n, n = n->Parent)
                ;

            if (n == &m_root)
                n = nullptr;
        }

        return n;
    }

    bool CheckNode(std::set<KeyType> &seenItems, Node *node)
    {
        if (!node)
            return true;

        if (seenItems.contains(node->Data))
        {
            printf("Node check failed, duplicate of");
            std::cout << node->Data << "\r\n";
            return false;
        }

        bool rval = true;

        seenItems.insert(node->Data);

        rval &= CheckNode(seenItems, node->Left);
        rval &= CheckNode(seenItems, node->Right);

        return rval;
    }

    void PrintNode(Node *node, int depth, const char *label)
    {
        if (!node)
            return;

        PrintNode(node->Right, depth + 1, "R");
        printf("%*s", 2 * depth, "");
        if (label)
            printf("%s: ", label);

        // I know, boo, live with it, it's for testing.
        std::cout << node->Data;

        printf(" (%s%d)\n", (node->Balance >= 0) ? "+" : "", node->Balance);
        PrintNode(node->Left, depth + 1, "L");
    }

    int Compare(const KeyType &lhs, const KeyType &rhs)
    {
        // Seriously, why couldn't they have just returned an int!?
        auto stupid_effing_cpp_standards = lhs <=> rhs;
        return stupid_effing_cpp_standards < 0 ? -1 : (stupid_effing_cpp_standards > 0 ? 1 : 0);
    }

    int CheckHeight(Node *node)
    {
        int lh, rh, cmp;

        if (!node)
            return 0;

        lh = CheckHeight(node->Left);

        if (lh < 0)
            return lh;

        rh = CheckHeight(node->Right);

        if (rh < 0)
            return rh;

        cmp = rh - lh;

        if (cmp < -1 || cmp > 1 || cmp != node->Balance)
            return -1;

        return 1 + ((lh > rh) ? lh : rh);
    }

public:
    /* constructor */ AVLTree(void)
        : m_root()
        , m_min(nullptr)
        , m_count(0)
    {
    }

    virtual ~AVLTree(void)
    {
    }

    size_t Count(void) const { return m_count; }

    void Insert(const KeyType &key, const DataType &data)
    {
        Node *current = First();
        Node *parent = &m_root;
        int cmp = 0;

        while (current != nullptr)
        {
            cmp = Compare(key, current->Key);

            if (cmp == 0)
            {
                // Duplicate entry
                current->Data = data;
                return;
            }

            parent = current;
            current = cmp < 0 ? current->Left : current->Right;
        }

        current = new Node();
        current->Parent = parent;
        current->Key = key;
        current->Data = data;

        if ((parent == &m_root) || (cmp < 0))
            parent->Left = current;
        else
            parent->Right = current;

        if (!m_min || current->Key < m_min->Key)
            m_min = current;

        while (current != First())
        {
            if (current == parent->Left)
            {
                // Height of left subtree has changed
                if (parent->Balance == 1)
                {
                    // Left subtree now balances parent
                    parent->Balance = 0;
                    break;
                }
                else if (parent->Balance == 0)
                {
                    // Height of parent increases, continue processing
                    parent->Balance = -1;
                }
                else if (parent->Balance == -1)
                {
                    // Balance factor is now -2, rebalance!
                    FixInsertLeftImbalance(parent);
                    break;
                }
            }
            else
            {
                if (parent->Balance == -1)
                {
                    // Right subtree now balances parent
                    parent->Balance = 0;
                    break;
                }
                else if (parent->Balance == 0)
                {
                    // Height of parent increases, continue processing
                    parent->Balance = 1;
                }
                else if (parent->Balance == 1)
                {
                    // Height of parent is now 2, rebalance!
                    FixInsertRightImbalance(parent);
                    break;
                }
            }

            // Walk up the chain
            current = parent;
            parent = current->Parent;
        } // while (current != First())

        ++m_count;
    }

    DataType Find(const KeyType &key)
    {
        Node *node = FindNode(key);
        return node ? node->Data : DataType();
    }

    bool Remove(const KeyType &key)
    {
        Node *node = FindNode(key);

        if (!node)
            return false; // Not in tree, nothing to do.

        Node *current, *parent;
        Node *target;

        if (!node->Left || !node->Right)
        {
            target = node;

            if (target == m_min)
                m_min = Successor(target);
        }
        else
        {
            // Node right isn't null, thus move down.
            target = Successor(node);

            // Data swapped
            std::swap(node->Key, target->Key);
            std::swap(node->Data, target->Data);
        }

        current = target;
        parent = current->Parent;

        while (current != First())
        {
            if (current == parent->Left)
            {
                if (parent->Balance == -1)
                {
                    parent->Balance = 0;
                }
                else if (parent->Balance == 0)
                {
                    parent->Balance = 1;
                    break; // Height unchanged, exit loop.
                }
                else if (parent->Balance == 1)
                {
                    // Balance became 2, rebalance!

                    parent = FixDeleteRightImbalance(parent);

                    if (parent->Balance == -1)
                        break; // Height unchanged, exit loop.
                }
            }
            else
            {
                if (parent->Balance == 1)
                {
                    parent->Balance = 0;
                }
                else if (parent->Balance == 0)
                {
                    parent->Balance = -1;
                    break; // Height unchanged, exit loop.
                }
                else if (parent->Balance == -1)
                {
                    // Balance became -2, rebalance!

                    parent = FixDeleteLeftImbalance(parent);

                    if (parent->Balance == 1)
                        break; // Height unchanged, exit loop.
                }

            }

            // Walk up chain
            current = parent;
            parent = current->Parent;
        } // while (current != First())

        Node *child = target->Left ? target->Left : target->Right; // Child may be NULL

        if (child)
            child->Parent = target->Parent;

        if (target == target->Parent->Left)
            target->Parent->Left = child;
        else
            target->Parent->Right = child;

        delete target;

        --m_count;

        return true;
    }

    DataType Min(void) const { return m_min ? m_min->Data : DataType(); }

    int GetHeight() { return CheckHeight(First()); }

    void CheckTree()
    {
        std::set<KeyType> seenItems;

        CheckNode(seenItems, First());
    }

    void Print(void)
    {
        printf("--\r\n");
        PrintNode(First(), 0, "T");
        printf("\nheight = %d\r\n", GetHeight());
    }
};

typedef AVLTree<int, int> IntTree;
