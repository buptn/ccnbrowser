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

#ifndef CCNTRIETREE_H_
#define CCNTRIETREE_H_

#include<vector>
#include<string>
#include<iostream>
using namespace std;

#define nullptr 0





typedef  struct ccncharbuf{
    size_t length;
    size_t limit;
    unsigned char *buf;
}  ContentObj;

class CCNFileNode{
public:
	CCNFileNode();
	CCNFileNode(size_t n);
   ~CCNFileNode();
   ContentObj* getCob();
   size_t getLength();
   unsigned char* getBuf();
   void setCob(unsigned char* content);
   void setCob(ContentObj * cob);

private:
   ContentObj *cob;

};

const int size = 128;
const int CAN_DELETE = 1;
const int CANNOT_DELETE = 0;
const int PATH_NO_EXIST = -1;
const int ADD_FAILED = -1;
const int ADD_SUCCEED = 0;


class Vertex{
public:
	Vertex();
	Vertex(CCNFileNode *filenode);
	~Vertex();
	int getNumOfChild();
	void setNumOfChild(int numOfChild);
	CCNFileNode * getCCNFileNode();
	void setFileNode(CCNFileNode * filenode);

	bool getIsExist();
	void setExist(bool  isExist);
	Vertex** getEdges(int c);

	char getNodeChar();
	void setNodeChar(char nodechar);


private:
	int numOfChild;
	bool  isExist;
	CCNFileNode*  filenode;

	char nodechar;
	Vertex* edges[size];
};

class CCNTrieTree{
public:
	CCNTrieTree();
	//CCNTrieTree(int jumpDistance);
	~CCNTrieTree();
	void checkTree();

	Vertex* getRoot();
	void setRoot(Vertex *root);

	void deletePath(string path);
	int deletePath(Vertex *vertex, string path);

	int addPath(string path);
	int addPath(string path, CCNFileNode *filenode);
	int  addPath(Vertex *vertex, string path, CCNFileNode *filenode);

	Vertex* findPath(string path);
	Vertex* findPath(string path, Vertex *vertex);

	void print(string path);
	void print();
	vector< string>  listAllPaths();
	void depthFirstSearch(vector<string> paths, Vertex *edge, string pathName);



private:
//	const int DEFAULT_DISTANCE = 2;
//	const int PATH_EXISTED = 2;
//	const int PATH_CREATE_SUCCEED = 1;
//	const int PATH_CREATE_FAIL = 0;
    //const int CAN_DELETE = 1;
    //const int CANNOT_DELETE = 0;
    //const int PATH_NO_EXIST = -1;
    //const int ADD_FAILED = -1;
    //const int ADD_SUCCEED = 0;
	Vertex *root;
};



#endif /* CCNTRIETREE_H_ */
