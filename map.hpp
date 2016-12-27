/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"
#include <iostream>
#include <queue>

namespace sjtu {
	const double alpha = 0.75;
template<class Key,class T,class Compare = std::less<Key> > 
class map 
{
public:
	typedef pair<const Key, T> value_type;
private:
	Compare compare;
	int cnt,used,reuse;
	struct node
	{
		value_type *storage;
		node *left, *right;
		int size, cover;
		bool exist;
		node(value_type* ss=NULL):storage(ss),left(NULL),right(NULL),size(1),cover(1),exist(true){}
		void pushup()
		{
			size = exist;
			cover = 1;
			if (left != NULL)
			{
				cover += left->cover;
				size += left->size;
			}
			if (right != NULL)
			{
				cover += right->cover;
				size += right->size;
			}
		}
		bool isBad()
		{
			return ((left != NULL && left->cover > cover*alpha + 5) || (right != NULL && right->cover > cover*alpha + 5));
		}
	};
	node *root;
	node **sort;
	void copyNode(const node* from, node*& to)
	{
		if (from != NULL)
		{
			to = new node;
			to->storage = new value_type(*(from->storage));
			to->size = from->size;
			to->cover = from->cover;
			to->exist = from->exist;
			copyNode(from->left, to->left);
			copyNode(from->right, to->right);
		}
		else to = NULL;
	}
	void makeEmpty()
	{
		if (root != NULL)
		{
			std::queue<node*> q;
			q.push(root);
			node *p;
			while (!q.empty())
			{
				p = q.front();
				q.pop();
				delete p->storage;
				if (p->left != NULL) q.push(p->left);
				if (p->right != NULL) q.push(p->right);
				delete p;
			}
			root = NULL;
		}
	}
	void travel(node*& p)
	{
		if (p == NULL) return;
		if (p->left != NULL) travel(p->left);
		if (p->exist) sort[cnt++] = p; else delete p->storage;
		if (p->right != NULL) travel(p->right);
		if (p->exist)
		{
			p->left = NULL;
			p->right = NULL;
			p->size = 1;
			p->cover = 1;
		} else delete p;
		p = NULL;
	}
	void Travel(node*& p)
	{
		cnt = 0;
		travel(p);
	}
	node* rebuild(int l, int r)
	{
		if (l>r) return NULL;
		if (l == r) return sort[l];
		int mid = (l + r) >> 1;
		node *tmp = sort[mid];
		tmp->left = rebuild(l, mid - 1);
		tmp->right = rebuild(mid + 1, r);
		tmp->pushup();
		return tmp;
	}
	void ReBuild(node*& p)
	{
		sort = new node*[p->size + 1];
		if (p == NULL) return;
		Travel(p);
		p = rebuild(0, cnt - 1);
		delete[] sort;
	}
public:
	class const_iterator;
	class iterator {
		friend const_iterator;
	private:
		node *data, *itsRoot;
	public:
		iterator(node* d1=NULL, node* r1 = NULL) {
			data = d1;
			itsRoot = r1;
		}
		iterator(const iterator &other) {
			data = other.data;
			itsRoot = other.itsRoot;
		}
		node* pointer() { return data; }
		node* findRoot() { return itsRoot; }
		iterator operator++(int) 
		{
			iterator tmp = *this;
			++(*this);
			return tmp;
		}
		iterator & operator++()
		{
			up();
			while (data!=NULL && !data->exist) up();
			return *this;
		}
		iterator& up()
		{
			Compare compare;
			if (data->right != NULL)
			{
				data = data->right;
				while (data->left != NULL) data = data->left;
				return *this;
			}
			else
			{
				node *q = itsRoot, *suc = NULL;
				while (q != NULL)
				{
					if (compare(data->storage->first, q->storage->first))
					{
						suc = q;
						q = q->left;
					}
					else if (compare(q->storage->first, data->storage->first))
						q = q->right;
					else
						break;
				}
				data = suc;
				return *this;
			}
		}
		iterator operator--(int) 
		{
			iterator tmp = *this;
			--(*this);
			return tmp;
		}
		iterator & operator--() 
		{
			down();
			while (data != NULL && !data->exist) down();
			return *this;
		}
		iterator& down()
		{
			if (data == NULL)
			{
				node *tmp = itsRoot;
				while (tmp->right != NULL) tmp = tmp->right;
				data = tmp;
				return *this;
			}
			Compare compare;
			if (data->left != NULL)
			{
				data = data->left;
				while (data->right != NULL) data = data->right;
				return *this;
			}
			else
			{
				node *q = itsRoot, *suc = NULL;
				while (q != NULL)
				{
					if (compare(q->storage->first, data->storage->first))
					{
						suc = q;
						q = q->right;
					}
					else if (compare(data->storage->first, q->storage->first))
						q = q->left;
					else
						break;
				}
				data = suc;
				return *this;
			}
		}
		value_type & operator*() const 
		{
			return *(data->storage);
		}
		bool operator==(const iterator &rhs) const 
		{
			if (itsRoot != rhs.itsRoot) return false;
			return (data == rhs.data);
		}
		bool operator==(const const_iterator &rhs) const 
		{
			if (itsRoot != rhs.itsRoot) return false;
			return (data == rhs.data);
		}
		bool operator!=(const iterator &rhs) const 
		{
			return (itsRoot!= rhs.itsRoot || data!=rhs.data);
		}
		bool operator!=(const const_iterator &rhs) const 
		{
			return (itsRoot != rhs.itsRoot || data != rhs.data);
		}
		/**
		 * for the support of it->first. 
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept 
		{
			return data->storage;
		}
	};
	class const_iterator {
		friend iterator;
		private:
			node *data, *itsRoot;
		public:
			const_iterator(node* d1 = NULL, node* r1 = NULL) {
				data = d1;
				itsRoot = r1;
			}
			const_iterator(const const_iterator &other) {
				data = other.data;
				itsRoot = other.itsRoot;
			}
			const_iterator(const iterator &other) {
				data = other.data;
				itsRoot = other.itsRoot;
			}
			node* pointer() { return data; }
			node* findRoot() { return itsRoot; }
			const_iterator operator++(int)
			{
				const_iterator tmp = *this;
				++(*this);
				return tmp;
			}
			const_iterator & operator++()
			{
				up();
				while (data != NULL && !data->exist) up();
				return *this;
			}
			const_iterator& up()
			{
				Compare compare;
				if (data->right != NULL)
				{
					data = data->right;
					while (data->left != NULL) data = data->left;
					return *this;
				}
				else
				{
					node *q = itsRoot, *suc = NULL;
					while (q != NULL)
					{
						if (compare(data->storage->first, q->storage->first))
						{
							suc = q;
							q = q->left;
						}
						else if (compare(q->storage->first, data->storage->first))
							q = q->right;
						else
							break;
					}
					data = suc;
					return *this;
				}
			}
			const_iterator operator--(int)
			{
				const_iterator tmp = *this;
				--(*this);
				return tmp;
			}
			const_iterator & operator--()
			{
				down();
				while (data != NULL && !data->exist) down();
				return *this;
			}
			const_iterator& down()
			{
				if (data == NULL)
				{
					node *tmp = itsRoot;
					while (tmp->right != NULL) tmp = tmp->right;
					data = tmp;
					return *this;
				}
				Compare compare;
				if (data->left != NULL)
				{
					data = data->left;
					while (data->right != NULL) data = data->right;
					return *this;
				}
				else
				{
					node *q = itsRoot, *suc = NULL;
					while (q != NULL)
					{
						if (compare(q->storage->first, data->storage->first))
						{
							suc = q;
							q = q->right;
						}
						else if (compare(data->storage->first, q->storage->first))
							q = q->left;
						else
							break;
					}
					data = suc;
					return *this;
				}
			}
			value_type & operator*() const
			{
				return *(data->storage);
			}
			bool operator==(const iterator &rhs) const
			{
				if (itsRoot != rhs.itsRoot) return false;
				return (data == rhs.data);
			}
			bool operator==(const const_iterator &rhs) const
			{
				if (itsRoot != rhs.itsRoot) return false;
				return (data == rhs.data);
			}
			bool operator!=(const iterator &rhs) const
			{
				return (itsRoot != rhs.itsRoot || data != rhs.data);
			}
			bool operator!=(const const_iterator &rhs) const
			{
				return (itsRoot != rhs.itsRoot || data != rhs.data);
			}
			value_type* operator->() const noexcept
			{
				return data->storage;
			}
	};
	map():root(NULL) {}
	map(const map &other) 
	{
		copyNode(other.root, root);
	}
	map & operator=(const map &other) 
	{
		if (this == &other) return *this;
		if (root != NULL) makeEmpty();
		copyNode(other.root, root);
		return *this;
	}
	~map() 
	{
		clear();
	}
	node** Insert(node*& p, const Key& k)
	{
		if (p == NULL)
		{
			T value;
			p = new node();
			p->storage = new value_type(k, value);
			return NULL;
		}
		node **res = (compare(k, p->storage->first)) ? (Insert(p->left, k)) : (Insert(p->right, k));
		++p->size;
		++p->cover;
		if (p->isBad()) res = &p;
		return res;
	}
	iterator add(node*& t, const Key& k)
	{
		node** tmp = Insert(t, k);
		if (tmp != NULL) ReBuild(*tmp);
		return find(k);
	}
	T & at(const Key &k) 
	{
		iterator it = find(k);
		if (it == end()) throw index_out_of_bound();
		return it.pointer()->storage->second;
	}
	const T & at(const Key &k) const 
	{
		const_iterator it = find(k);
		if (it == cend()) throw index_out_of_bound();
		return it.pointer()->storage->second;
	}
	T & operator[](const Key &k) 
	{
		iterator it = find(k);
		if (it == end())
		{
			it = add(root, k);
		}
		return it.pointer()->storage->second;
	}
	const T & operator[](const Key &k) const 
	{
		const_iterator it = find(k);
		if (it != cend()) return it.pointer()->storage->second;
		else throw runtime_error();
	}
	iterator begin() 
	{
		if (size() == 0) return iterator(NULL, root);
		node *tmp = root;
		while (tmp->left != NULL) tmp = tmp->left;
		iterator res = iterator(tmp, root);
		while (!res.pointer()->exist) res.up();
		return res;
	}
	const_iterator cbegin() const 
	{
		if (size() == 0) return const_iterator(NULL, root);
		node *tmp = root;
		while (tmp->left != NULL) tmp = tmp->left;
		const_iterator res = const_iterator(tmp, root);
		while (!res.pointer()->exist) res.up();
		return res;
	}
	iterator end() 
	{
		return iterator(NULL, root);
	}
	const_iterator cend() const 
	{
		return const_iterator(NULL, root);
	}
	bool empty() const { return (size() == 0); }
	size_t size() const 
	{ 
		if (root == NULL) return 0;
		else return root->size;
	}
	void clear() 
	{
		makeEmpty();
	}
	pair<iterator, bool> insert(const value_type &val) 
	{
		iterator tmp = find(val.first);
		if (tmp != end()) return pair<iterator, bool>(tmp, false);
		tmp = add(root, val.first);
		tmp.pointer()->storage->second = val.second;
		return pair<iterator, bool>(tmp, true);
	}
	void erase(iterator pos) 
	{
		if (pos == end() || pos.findRoot()!=root) throw invalid_iterator();
		else
		{
			node *tmp = root;
			while (tmp != NULL)
			{
				--tmp->size;
				if (tmp==pos.pointer() && tmp->exist)
				{
					tmp->exist = false;
					break;
				}
				else if (compare(pos.pointer()->storage->first, tmp->storage->first)) tmp = tmp->left;
				else tmp = tmp->right;
			}
			if (root->size + 10 < alpha*root->cover) ReBuild(root);
		}
	}
	size_t count(const Key &k)
	{
		if (find(k) != iterator(NULL,root)) return 1; 
		    else return 0;
	}
	iterator find(const Key &k) 
	{
		node *tmp = root;
		while (tmp != NULL) 
		{
			if (compare(tmp->storage->first, k))
			{
				tmp = tmp->right;
				//std::cout << "right\n";
			}
			else if (compare(k, tmp->storage->first))
			{
				tmp = tmp->left;
				//std::cout << "left\n";
			}
			else
			{
				if (!tmp->exist) tmp = NULL;
				break;
			}
		}
		return iterator(tmp, root);
	}
	const_iterator find(const Key &k) const 
	{
		node *tmp = root;
		while (tmp != NULL)
		{
			if (compare(tmp->storage->first, k))
				tmp = tmp->right;
			else if (compare(k, tmp->storage->first))
				tmp = tmp->left;
			else
			{
				if (!tmp->exist) tmp = NULL;
				break;
			}
		}
		return const_iterator(tmp, root);
	}
};

}

#endif
