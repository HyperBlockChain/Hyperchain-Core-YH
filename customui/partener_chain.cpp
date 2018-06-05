/*copyright 2016-2018 hyperchain.net (Hyperchain)
/*
/*Distributed under the MIT software license, see the accompanying
/*file COPYING or https://opensource.org/licenses/MIT。
/*
/*Permission is hereby granted, free of charge, to any person obtaining a copy of this 
/*software and associated documentation files (the "Software"), to deal in the Software
/*without restriction, including without limitation the rights to use, copy, modify, merge,
/*publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
/*to whom the Software is furnished to do so, subject to the following conditions:
/*
/*The above copyright notice and this permission notice shall be included in all copies or
/*substantial portions of the Software.
/*
/*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
/*INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
/*PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
/*FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
/*OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/*DEALINGS IN THE SOFTWARE.
*/

#define NODE_W_H_SIZE 20
#define NODE_SPACEING 2
#include "partener_chain.h"

partener_chain::partener_chain(QWidget *parent) : QFrame(parent)
{

}

void partener_chain::setNodeInfo(int chainNum)
{
    initNodes(chainNum);
}

void partener_chain::setNodeInfo(VEC_T_NODEINFO &vec)
{

	clear();
    const int MAX_NODE_SIZE = 23;

    int size = vec.size();
    int startIndex = 0;
    if(size > MAX_NODE_SIZE){
        startIndex = size - MAX_NODE_SIZE;
    }

    for(int i = startIndex; i < size; i++){
        PartnerNode node;
        node.index = i;

		node.state = (_ePoeReqState)vec[i].uiNodeState;

        node.info  = QString::fromStdString(vec[i].strNodeIp);

        parentNodes_.append(node);
    }

}

void partener_chain::initNodes(int chainNum)
{

	clear();

    if(1 == chainNum){
        int count = 12;
        for(unsigned int i = 0; i < count; i++){
            PartnerNode node;
            node.index = i;
            node.state = CONFIRMED;
            parentNodes_.append(node);
        }

        PartnerNode node;
        node.index = count;

        node.state = DEFAULT_REGISREQ_STATE;
        parentNodes_.append(node);
    }else if(2 == chainNum){
        int count = 12;
        for(unsigned int i = 0; i < count; i++){
            PartnerNode node;
            node.index = i;
            node.state = RECV;
            parentNodes_.append(node);
        }

        PartnerNode node;
        node.index = count;
        node.state = CONFIRMING;
        parentNodes_.append(node);
    }else if(3 == chainNum){
        int count = 7;
        for(unsigned int i = 0; i < count; i++){
            PartnerNode node;
            node.index = i;
            node.state = RECV;
            parentNodes_.append(node);
        }

        PartnerNode node;
        node.index = count;
        node.state = CONFIRMING;
        parentNodes_.append(node);
    }

}

void partener_chain::showNodes()
{
    int h = height();
    int top = (h - NODE_W_H_SIZE) / 2;
    int left = NODE_SPACEING;

    for(auto item : parentNodes_){
        hc_label *label = new hc_label(this, item.state);
        label->setFixedSize(NODE_W_H_SIZE, NODE_W_H_SIZE);
		label->setGeometry(QRect(left, top, NODE_W_H_SIZE, NODE_W_H_SIZE));
		label->show();
		m_hcLabel_.append(label);

        left += (NODE_W_H_SIZE + NODE_SPACEING);
    }
}

void partener_chain::clear()
{
	if (parentNodes_.size() > 0 )
		parentNodes_.clear();

	for (auto item : m_hcLabel_)
	{
		delete item;
	}

	if ( m_hcLabel_.size() > 0)
		m_hcLabel_.clear();
}

void partener_chain::clearNodes()
{
	clear();
	update();
}