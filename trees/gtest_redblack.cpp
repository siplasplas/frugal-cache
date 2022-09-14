//
// Created by Andrzej Borucki on 2022-09-14
//
#include "gtest/gtest.h"
#include "RedBlackTree.h"
#include "AvlTree.h"

using namespace std;

TEST(RBTree, test) {
    AvlTree avl;
    for (int i=0; i<1000; i++)
        avl.insertNode(i);
    RedBlackTree rbt;
    for (int i=0; i<1000; i++)
        rbt.insertNode(i);
}