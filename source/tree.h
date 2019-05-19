#ifndef TREE_H
#define TREE_H

#include <string>
#include <vector>

class TreeNode
{
private:
	int ID;
	std::string object_name;

	TreeNode *parent;

	std::vector<TreeNode *> children;

	int countNodesRec(TreeNode *root, int& count);

public:
	TreeNode();
	TreeNode(int iID,std::string iobject_name);

	void appendChild(TreeNode *child);
	void setParent(TreeNode *parent);

	void popBackChild();
	void removeChild(int pos);

	bool hasChildren();
	bool hasParent();

	TreeNode* getParent();
	TreeNode* getChild(int pos);

	TreeNode* DFS(TreeNode *head, std::string name);

	int childrenNumber();
	int grandChildrenNum();

	int getID();
	std::string getObjectName();
};


#endif