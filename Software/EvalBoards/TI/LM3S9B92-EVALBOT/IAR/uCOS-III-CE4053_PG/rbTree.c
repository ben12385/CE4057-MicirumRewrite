#include <os.h>
#include <stdio.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *os_tick__c = "$Id: $";
#endif

#define RED 1
#define BLACK 0

void traverseRB(struct rbNode *currentNode, int level){
  if(currentNode == NULL)
    return;
  
  traverseRB(currentNode->left, level+1);
  printf("Key value %d and blackWhite %d and level %d\n", currentNode->key, currentNode->color, level);
  traverseRB(currentNode->right, level+1);
}

struct rbNode* obtainMostLeftrbNode(struct rbNode *root){
  if(root == NULL){
    return NULL;
  }
  struct rbNode *temp = root;
  while(temp->left != NULL){
    temp = temp->left;
  }
  return temp;
}

void rotateRight(struct rbNode *root, struct rbNode *pt){ 
  if(pt == NULL || pt == root){
    return;
  }
  
    struct rbNode *pt_left = pt->left; 
  
    pt->left = pt_left->right; 
  
    if (pt->left != NULL) 
        pt->left->parent = pt; 
  
    pt_left->parent = pt->parent; 
  
    if (pt->parent == NULL) 
        root = pt_left; 
  
    else if (pt == pt->parent->left) 
        pt->parent->left = pt_left; 
  
    else
        pt->parent->right = pt_left; 
  
    pt_left->right = pt; 
    pt->parent = pt_left; 
} 

void rotateLeft(struct rbNode *root, struct rbNode *pt){ 
    if(pt == NULL || pt == root){
    return;
  }
  
    struct rbNode *pt_right = pt->right; 
  
    pt->right = pt_right->left; 
  
    if (pt->right != NULL) 
        pt->right->parent = pt; 
  
    pt_right->parent = pt->parent; 
  
    if (pt->parent == NULL) 
        root = pt_right; 
  
    else if (pt == pt->parent->left) 
        pt->parent->left = pt_right; 
  
    else
        pt->parent->right = pt_right; 
  
    pt_right->left = pt; 
    pt->parent = pt_right; 
} 

void swap(struct rbNode *node1, struct rbNode *node2){
  int temp = node1->color;
  node1->color = node2->color;
  node2->color = temp;
}

struct rbNode* rbInsertNode(struct rbNode* currentNode, struct rbNode* toInsert){
	//Find location of insertion and insert
	if(currentNode == NULL){
                toInsert->color = RED;
		return toInsert;
	}
	

	if(toInsert->key < currentNode->key){
          currentNode->left = rbInsertNode(currentNode->left,toInsert);
	  currentNode->left->parent = currentNode;
          return currentNode;
	}
	else if(toInsert->key > currentNode->key){
          currentNode->right = rbInsertNode(currentNode->right, toInsert);
	  currentNode->right->parent = currentNode;
          return currentNode;
	}
	//If same, binary tree cannot have same key, so increase key by 1
	//TODO, update the tree if keys overlap
	else{
		//If sameKeyValue is True then means that tickRemain is not the true tickRemain
                //printf("Same key value\n");
                toInsert->key = toInsert->key+1;
                toInsert->sameKeyValue = 1;
                return currentNode;
	}
}

void fixViolation(struct rbNode *root, struct rbNode *pt){ 
    struct rbNode *parent_pt = NULL; 
    struct rbNode *grand_parent_pt = NULL; 
  
    while ((pt != root) && (pt->color != BLACK) && 
           (pt->parent->color == RED)) 
    { 
  
        parent_pt = pt->parent; 
        grand_parent_pt = pt->parent->parent; 
  
        /*  Case : A 
            Parent of pt is left child of Grand-parent of pt */
        if (parent_pt == grand_parent_pt->left) 
        { 
  
            struct rbNode *uncle_pt = grand_parent_pt->right; 
  
            /* Case : 1 
               The uncle of pt is also red 
               Only Recoloring required */
            if (uncle_pt != NULL && uncle_pt->color == RED) 
            { 
                grand_parent_pt->color = RED; 
                parent_pt->color = BLACK; 
                uncle_pt->color = BLACK; 
                pt = grand_parent_pt; 
            } 
  
            else
            { 
                /* Case : 2 
                   pt is right child of its parent 
                   Left-rotation required */
                if (pt == parent_pt->right) 
                { 
                    rotateLeft(root, parent_pt); 
                    pt = parent_pt; 
                    parent_pt = pt->parent; 
                } 
  
                /* Case : 3 
                   pt is left child of its parent 
                   Right-rotation required */
                rotateRight(root, grand_parent_pt); 
                swap(parent_pt, grand_parent_pt); 
                pt = parent_pt; 
            } 
        } 
  
        /* Case : B 
           Parent of pt is right child of Grand-parent of pt */
        else
        { 
            struct rbNode *uncle_pt = grand_parent_pt->left; 
  
            /*  Case : 1 
                The uncle of pt is also red 
                Only Recoloring required */
            if ((uncle_pt != NULL) && (uncle_pt->color == RED)) 
            { 
                grand_parent_pt->color = RED; 
                parent_pt->color = BLACK; 
                uncle_pt->color = BLACK; 
                pt = grand_parent_pt; 
            } 
            else
            { 
                /* Case : 2 
                   pt is left child of its parent 
                   Right-rotation required */
                if (pt == parent_pt->left) 
                { 
                    rotateRight(root, parent_pt); 
                    pt = parent_pt; 
                    parent_pt = pt->parent; 
                } 
  
                /* Case : 3 
                   pt is right child of its parent 
                   Left-rotation required */
                rotateLeft(root, grand_parent_pt); 
                swap(parent_pt, grand_parent_pt); 
                pt = parent_pt; 
            } 
        } 
    } 
    root->color = BLACK; 
    root->parent = NULL;
} 

//Exits out if value already in rbTree
struct rbNode* insertNodeToRBTree(struct rbNode* root, struct rbNode* toInsert){
        int temp = toInsert->key;  
	root = rbInsertNode(root, toInsert);
        
        //Check if insert was successful
        if(temp != toInsert->key){
            temp = toInsert->key;
            root = rbInsertNode(root, toInsert);
        }
        
	fixViolation(root, toInsert);
        return root;
}

//------------------------------------------------------------------------//
void swapPlaces(struct rbNode* u, struct rbNode *v){
  struct rbNode* leftU = u->left;
  struct rbNode* rightU = u->right;
  struct rbNode* parentU = u->parent;
  struct rbNode* leftV = v->left;
  struct rbNode* rightV = v->right;
  struct rbNode* parentV = v->parent;
  
  if(leftU == v)
    v->left = u;
  else
    v->left = leftU;
    
  if(rightU == v)
    v->right = u;
  else
    v->right = rightU;

  if(leftV == u)
    u->left = v;
  else
    u->left = leftV;
    
  if(rightV == u)
    u->right = v;
  else
    u->right = rightV;
    
  if(parentU == v){
    v->parent = u;
  }
  else{
    v->parent = parentU;
   if(parentU->left == u){
    parentU->left = v;
    }
    else{
      parentU->right = v;
    }
  }
    
  if(parentV == u){
    u->parent = v;
  }
  else{
    u->parent = parentV;
    if(parentV->left == v){
    parentV->left = u;
    }
    else{
      parentV->right = u;
    }
  }
  
  if(u->left != NULL)
    u->left->parent = u;
  if(u->right != NULL)
    u->right->parent = u;
  if(v->left != NULL)
    v->left->parent = v;
  if(v->right != NULL)
    v->right->parent = v;
  
  
}

struct rbNode* standardBSTDelete(struct rbNode* toDelete){
  struct rbNode* toReplace;
  //if have no child then is leaf so do nothing
  if(toDelete->left == NULL && toDelete->right == NULL){
    //Do nothing
    toReplace = NULL;
  }
  //if only have left child, obtain more right node in left subtree
  else if(toDelete->right == NULL && toDelete->left != NULL){
    toReplace = toDelete->left;
    while(toReplace->right != NULL){
      toReplace = toReplace->right;
    }
    //Swap places
    swapPlaces(toDelete,toReplace);
  }
  //if only have right child or have 2 children, obtain more left node in right subtree
  else{
    toReplace = toDelete->right;
    while(toReplace->left != NULL){
      toReplace = toReplace->left;
    }
    //Swap places
    swapPlaces(toDelete,toReplace);
  }
  
  return toReplace;
}

struct rbNode* deleteNodeInRBTree(struct rbNode* root, struct rbNode* toDelete){
  if(toDelete == NULL || root == NULL){
    return root;
  }
  
  struct rbNode* replacement = standardBSTDelete(toDelete);
  
  //if replacement is null means leaf node so double black and delete
  if(replacement == NULL){
    if(root == toDelete){
      return NULL;
    }
    else{
      toDelete->color = BLACK;
      fixDoubleBlack(toDelete, root);
      if(toDelete->parent->left == toDelete){
        toDelete->parent->left = NULL;
      }
      else{
        toDelete->parent->right = NULL;
      }
      
    }
  }
  //if either replacement or toDelete is red
  else if((replacement->color == RED && toDelete->color == BLACK) || (toDelete->color == RED && replacement->color == BLACK)){
    replacement->color = BLACK;
    
    if(toDelete->parent->left == toDelete){
        toDelete->parent->left = NULL;
      }
      else{
        toDelete->parent->right = NULL;
      }
  }
  //if both are black
  else if(replacement->color == BLACK && toDelete->color == BLACK){
      if(toDelete->parent->left == toDelete){
        toDelete->parent->left = NULL;
      }
      else{
        toDelete->parent->right = NULL;
      }
    
    fixDoubleBlack(replacement, root);
  }
  if(root == toDelete){
    return replacement;
  }
  return root;
}

struct rbNode* findSibling(struct rbNode* toCheck) { 
    // sibling null if no parent 
    if (toCheck->parent == NULL) 
      return NULL; 
  
    if (toCheck==toCheck->parent->left) 
      return toCheck->parent->right; 
  
    return toCheck->parent->left; 
} 

int hasRedChild(struct rbNode* toCheck) { 
    return (toCheck->left != NULL && toCheck->left->color == RED) || (toCheck->right != NULL && toCheck->right->color == RED); 
} 

void fixDoubleBlack(struct rbNode* x, struct rbNode* root) { 
    if (x == root) 
      // Reached root 
      return; 
  
    struct rbNode* sibling = findSibling(x); 
    struct rbNode* parent = x->parent; 
    if (sibling == NULL){ 
      // No sibiling, double black pushed up 
      fixDoubleBlack(parent, root); 
    } 
    else{ 
      if (sibling->color == RED){ 
        // Sibling red 
        parent->color = RED; 
        sibling->color = BLACK; 
        if (sibling==sibling->parent->left){ 
          // left case 
          rotateRight(root, parent); 
        } 
        else { 
          // right case 
          rotateLeft(root, parent); 
        } 
        fixDoubleBlack(x, root); 
      }
      else{ 
        // Sibling black 
        if(hasRedChild(sibling)){ 
          // at least 1 red children 
          if(sibling->left != NULL && sibling->left->color == RED){ 
            if(sibling==sibling->parent->left){ 
              // left left 
              sibling->left->color = sibling->color; 
              sibling->color = parent->color; 
              rotateRight(root, parent); 
            } else { 
              // right left 
              sibling->left->color = parent->color; 
              rotateRight(root, sibling);
              rotateLeft(root, parent); 
            } 
          }
          else{ 
            if (sibling==sibling->parent->left){ 
              // left right 
              sibling->right->color = parent->color; 
              rotateLeft(root, sibling);
              rotateRight(root, parent);
            } else { 
              // right right 
              sibling->right->color = sibling->color; 
              sibling->color = parent->color; 
              rotateLeft(root, parent);
            } 
          } 
          parent->color = BLACK; 
        } else { 
          // 2 black children 
          sibling->color = RED; 
          if (parent->color == BLACK) 
            fixDoubleBlack(parent, root); 
          else
            parent->color = BLACK; 
        } 
      } 
    } 
} 