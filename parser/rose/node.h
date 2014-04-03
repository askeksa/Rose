/* This file was generated by SableCC (http://www.sablecc.org/). */

#ifndef __rose__node_hh__
#define __rose__node_hh__

#include <string>
#include <list>
#include <vector>

namespace rose {

class Analysis;
class Node;
class Token;
class _GenericNode;
template <class T> class List;

struct _TypeInfo {
  const char *name;
  const _TypeInfo *parent;
  int type_id;

  int token_index;
  bool token_is_mutable;

  int prod_elem_count;
  int *prod_elem_is_list;

  enum {
    is_token,
    is_prod,
    is_other
  } type;

  typedef void (Analysis::* apply_t)(Node);
  apply_t apply_ptr;

private:
  friend class Node;
  friend class _GenericNode;
  bool is (const _TypeInfo *info) const;
  void apply (Analysis& analysis, Node& node) const;
};

struct _NodeData;

class _GenericNode {
private:
  _GenericNode ();
  ~_GenericNode ();

  // fields for alternatives
  std::vector<_NodeData> nodes;

  // fields for Token
  std::string text;
  int line;
  int pos;

  // typeinfo
  const _TypeInfo *type_info;

  // parent
  _GenericNode *parent;

  int ref_count;
  friend class Node;
  friend class Token;
  friend class Lexer;
  friend class Parser;

  static _GenericNode* initToken (const _TypeInfo *type_info, int line, int pos, const std::string& text);
  static _GenericNode* initProd (const _TypeInfo *type_info, void **args);

  Node getChildNode (int id);
  void setChildNode (int id, Node node);

  List<Node>& getChildList (int id);

  void removeChild (Node node);
  void replaceChild (Node old_child, Node new_child);

  void replaceBy (Node node);

  std::string toString() const;

  Node clone () const;
};

class Node
{
public:
  inline Node () : obj(0) { }
  inline Node (const Node& node) : obj(node.obj) { if (obj) obj->ref_count++; }
  inline Node& operator= (const Node& node)
  {
    if (obj && !--obj->ref_count) delete obj;
    obj = node.obj;
    if (obj) obj->ref_count++;
    return *this;
  }
  inline ~Node () { if ( obj && !--obj->ref_count) delete obj; }

  inline operator bool () const { return obj != 0; }

  template<class T>
  inline T& unsafe_cast () { return reinterpret_cast<T&>(*this); }

  template<class T>
  inline bool is() { return ptr()->type_info->is(&T::type_info); }

  template<class T>
  inline T cast () { if ( is<T>() ) return unsafe_cast<T>(); else return T(); }

  inline Node parent() { return ptr()->parent; }

  static const _TypeInfo type_info;

  inline std::string toString() const { return ptr()->toString(); }

  inline void replaceBy (Node node) { return ptr()->replaceBy(node); }

  inline void apply (Analysis& analysis) { ptr()->type_info->apply (analysis, *this); }

  inline const char * type_name() const { return ptr()->type_info->name; }
  inline int type_id() const { return ptr()->type_info->type_id; }

  inline Node clone () const { return ptr()->clone(); }

  friend inline bool operator== (const Node& l, const Node& r) { return l.obj == r.obj; }
  friend inline bool operator!= (const Node& l, const Node& r) { return l.obj != r.obj; }

protected:
  inline Node (_GenericNode *obj) : obj(obj) { if (obj) obj->ref_count++; }

  inline _GenericNode *ptr () { return obj; }
  inline const _GenericNode *ptr () const { return obj; }

  static inline _GenericNode *initProd (const _TypeInfo *type_info)
  { return _GenericNode::initProd(type_info, (void **)0); }
  static inline _GenericNode *initProd (const _TypeInfo *type_info, void **args)
  { return _GenericNode::initProd(type_info, args); }

  inline Node getChildNode (int id) { return ptr()->getChildNode(id); }
  inline void setChildNode (int id, Node node) { ptr()->setChildNode(id, node); }
  inline List<Node>& getChildList (int id) { return ptr()->getChildList(id); }

  template <class T>
  static inline std::list<Node>& getListGuts (List<T>& list) { return list.list; }

private:
  mutable _GenericNode *obj;

  inline void parent (_GenericNode *p) { ptr()->parent = p; }
  inline void removeChild (Node node) { ptr()->removeChild(node); }
  inline void replaceChild (Node old_child, Node new_child) { ptr()->replaceChild(old_child, new_child); }

  friend class _GenericNode;
  friend class _List_helper;
  friend class Parser;
};

class Exception {
public:
  inline Exception (const std::string& msg) : msg(msg) { }
  inline const std::string& getMessage () const { return msg; }
  virtual ~Exception ();

private:
  std::string msg;
};

}

#endif // ! __rose__node_hh__
