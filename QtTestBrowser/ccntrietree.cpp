/*
 * Copyright (C) 2014 lv peng (lvpp9527@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ccntrietree.h"

#include<iostream>
#include<stdlib.h>

CCNTrieTree::CCNTrieTree(){
	root = new Vertex();
	//JumpDistance = DEFAULT_DISTANCE;
}

//CCNTrieTree::CCNTrieTree(int jumpDistance){
//	root = new Vertex();
//	this->JumpDistance = jumpDistance;
//}

CCNTrieTree::~CCNTrieTree(){

}

void CCNTrieTree::checkTree(){
	if (root == nullptr)
		root = new Vertex();
}

Vertex* CCNTrieTree::getRoot(){
	return root;
}

void CCNTrieTree::setRoot(Vertex *root){
	this->root = root;
}

void CCNTrieTree::deletePath(string path){
	checkTree();
	Vertex *vertex = findPath(path);
	if (vertex == nullptr)
		return;
	else
		deletePath(root, path);
}

int CCNTrieTree::deletePath(Vertex *vertex, string path){
	if (path.size() == 0){
		if (vertex->getIsExist()){
			cout << "pahtExist" << endl;
			if (vertex->getNumOfChild() == 0){
				vertex->setFileNode(nullptr);
				vertex->setExist(false);
				return CAN_DELETE;
			}
			//vertex->setExist(false);
			return CANNOT_DELETE;
		}
	}
	char c = path.at(0);
	Vertex** tmp = vertex->getEdges((int)c);

	//System.out.println("c"+c);
	if ((*tmp) == nullptr){
		return PATH_NO_EXIST;
	}
	int type = deletePath((*tmp), path.substr(1));
	if (type == CAN_DELETE){
		(*tmp) = nullptr;
		vertex->setNumOfChild(vertex->getNumOfChild() - 1);
		if (vertex->getNumOfChild() == 0){
			vertex->setFileNode(nullptr);
			vertex->setExist(false);
			return CAN_DELETE;
		}
	}
	if (type == PATH_NO_EXIST){
		return PATH_NO_EXIST;
	}
	return CANNOT_DELETE;


}
int CCNTrieTree::addPath(string path){
	checkTree();
	return addPath(root, path, nullptr);
}



int CCNTrieTree::addPath(string path, CCNFileNode *filenode){
	checkTree();
	return addPath(root, path, filenode);
}

int  CCNTrieTree::addPath(Vertex *vertex, string path, CCNFileNode *filenode){
	//System.out.println(path);
	if (path.size() == 0){
		if (vertex->getIsExist()){
			//System.out.println("pathExist");
			//filenode.print();
			if (filenode != nullptr){
				vertex->setFileNode(filenode);
				return  ADD_SUCCEED;
			}
			cout << "filenode_point is null !" << endl;
			return  ADD_FAILED;
			//return nullptr;
		}
		vertex->setExist(true);
		if (filenode != nullptr)
		{
			vertex->setFileNode(filenode);
			return  ADD_SUCCEED;
		}
		else
		{
			//vertex->setVertexJumpList(new CCNJumpList());
			cout << "filenode_point is null !" << endl;
			return ADD_FAILED;
			//return nullptr;
		}
	}
	char c = path.at(0);
	Vertex** tmp = vertex->getEdges((int)c);
	//System.out.println("c"+c);
	if ((*tmp) == nullptr){
		(*tmp) = new Vertex();
		vertex->setNumOfChild(vertex->getNumOfChild() + 1);
		(*tmp)->setNodeChar(c);
	}
	return addPath((*tmp), path.substr(1), filenode);
}

Vertex* CCNTrieTree::findPath(string path){
	checkTree();
	return findPath(path, root);
}

Vertex* CCNTrieTree::findPath(string path, Vertex *vertex){
	if (path.size() == 0){
		if (vertex->getIsExist() == true)
			return vertex;
		else
		{
			cout<<"not found!"<<endl;
			return nullptr;
		}
	}
	char c = path.at(0);
	Vertex** tmp = vertex->getEdges((int)c);
	if ((*tmp) != nullptr){
		return findPath(path.substr(1), (*tmp));
	}
	else{
		return nullptr;
	}

}




Vertex::Vertex(){
	isExist = false;
	numOfChild = 0;
	nodechar = 0;
	filenode = nullptr;
	for (int i = 0; i<size; i++){
		edges[i] = nullptr;
	}
}

Vertex::Vertex(CCNFileNode *node){
	isExist = false;
	numOfChild = 0;
	nodechar = 0;
	this->filenode = node;
	for (int i = 0; i<size; i++){
		edges[i] = nullptr;
	}

}
Vertex::~Vertex(){

}

int Vertex::getNumOfChild(){
	return  numOfChild;
}

void Vertex::setNumOfChild(int numOfChild){
	this->numOfChild = numOfChild;
}

CCNFileNode* Vertex::getCCNFileNode(){
	return filenode;
}

void Vertex::setFileNode(CCNFileNode* filenode){

	this->filenode = filenode;
}

bool  Vertex::getIsExist(){
	return isExist;
}

void Vertex::setExist(bool  isExist){
	this->isExist = isExist;
}

Vertex** Vertex::getEdges(int c){
	return &edges[c];
}


char Vertex::getNodeChar(){
	return this->nodechar;
}

void Vertex::setNodeChar(char nodechar){
	this->nodechar = nodechar;

}



CCNFileNode::CCNFileNode() {
	cob = (ContentObj*) calloc(1, sizeof(*cob));
}

CCNFileNode::CCNFileNode(size_t n) {
	cob = (ContentObj*) malloc(sizeof(*cob));

	cob->length = 0;
	cob->limit = n;
	if (n == 0) {
		cob->buf = NULL;
	}
	cob->buf = (unsigned char*) malloc(n);
	if (cob->buf == NULL) {
		free(cob);
		cob = NULL;
	}

}

CCNFileNode::~CCNFileNode() {
	if (cob != NULL) {

		free(cob);
	}
}

ContentObj* CCNFileNode::getCob() {
	return cob;

}

unsigned char* CCNFileNode::getBuf(){
	return cob->buf;
}

size_t CCNFileNode::getLength(){
	return cob->limit;
}

void CCNFileNode::setCob(unsigned char* content) {
	//cob->buf =(unsigned char*) calloc(1, sizeof(*content));
	cob->buf = content;

}

void CCNFileNode::setCob(ContentObj * cob) {
	this->cob = cob;

}




