#ifndef TREENODE_H
#define TREENODE_H

#include <QList>
#include <QString>

template <typename T>
class TreeNode
{
public:
	TreeNode() {}
	TreeNode(const QString &name) : _name(name) {}

	const QString &name() const {return _name;}
	const QList<TreeNode> &childs() const {return _childs;}
	const QList<T> &items() const {return _items;}

	void addItem(T node) {_items.append(node);}
	void addChild(const TreeNode<T> &child) {_childs.append(child);}

	bool isEmpty() const {return _childs.isEmpty() && _items.isEmpty();}

private:
	QString _name;
	QList<TreeNode> _childs;
	QList<T> _items;
};

#endif // TREENODE_H
