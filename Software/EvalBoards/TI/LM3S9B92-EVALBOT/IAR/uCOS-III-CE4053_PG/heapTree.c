#include <os.h>
#include <stdio.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *os_tick__c = "$Id: $";
#endif

void traverse(struct node* toInsert){
  if(toInsert == NULL){
     return;
  }
  
  traverse(toInsert->childLeft);
  printf("Tick Remain %d and height of %d\n", (toInsert)->tickToRun, (toInsert)->height);
  traverse(toInsert->childRight);
}

void updateHeight(struct node *toUpdate){
    if(toUpdate->childLeft != NULL && toUpdate->childRight != NULL){
        toUpdate->height = (toUpdate->childLeft->height>toUpdate->childRight->height) ? toUpdate->childLeft->height : toUpdate->childRight->height;
        toUpdate->height = (toUpdate->height) + 1;
    }
    else if(toUpdate->childLeft != NULL && toUpdate->childRight == NULL){
        toUpdate->height = (toUpdate->childLeft->height) + 1;
    }
    else if(toUpdate->childLeft == NULL && toUpdate->childRight != NULL){
        toUpdate->height = (toUpdate->childRight->height) + 1;
    }
    else
    {
        toUpdate->height = 1;
    }
}

struct node *rightRotate(struct node *root) { 
    struct node *left = root->childLeft; 
    struct node *right = left->childRight; 
  
    // Perform rotation 
    left->childRight = root; 
    root->childLeft = right; 
	
	//Update parent
	right->parent = root;
	left->parent = root->parent;
	root->parent = left;
  
    //Update heights for only leftChild of root which became root and root which became the rightChild of root's old leftChild
	//Check if left child of root or right child of root has a greater height and add 1 to it
        updateHeight(root);
	
        updateHeight(left);
	
        //left->height = (left->childLeft->height>left->childRight->height) ? left->childLeft->height : left->childRight->height;
	//left->height = (left->height) + 1;
	
	return left;
} 

struct node *leftRotate(struct node *root) { 
    struct node *right = root->childRight; 
    struct node *left = right->childLeft; 
  
	right->childLeft = root;
	root->childRight = left;
  
	//Update parent
	left->parent = root;
	right->parent = root->parent;
	root->parent = right;
	
	//Update heights for only rightChild of root which became root and root which became the leftChild of root's old rightChild
	//Check if left child of root or right child of root has a greater height and add 1 to it
	updateHeight(root);
	
        updateHeight(right);
	
	return right;
} 

//Insertion Node
struct node* insertNode(struct node* toInsert, struct node* currentNode){
	//Find location of insertion and insert
	if(currentNode == NULL){
		return toInsert;
	}
	
        //If overflow means tickToRun is less but is actually more
        if(toInsert->tickToRun < currentNode->tickToRun && currentNode->overflow != toInsert->overflow){
          currentNode->childRight = insertNode(toInsert, currentNode->childRight);
        }
	else if(toInsert->tickToRun < currentNode->tickToRun){
          currentNode->childLeft = insertNode(toInsert, currentNode->childLeft);
	}
	else if(toInsert->tickToRun > currentNode->tickToRun){
           currentNode->childRight = insertNode(toInsert, currentNode->childRight);
	}
	//If same, binary tree cannot have same key, so increase key by 1
	//TODO, update the tree if keys overlap
	else{
		//If sameKeyValue is True then means that tickRemain is not the true tickRemain
                //printf("Same key value\n");
                toInsert->tickToRun = toInsert->tickToRun+1;
                toInsert->sameKeyValue = 1;
                return currentNode;
	}
	
    //Update height of this ancestor node
        updateHeight(currentNode);
	//currentNode->height = (currentNode->childLeft->height>currentNode->childRight->height) ? currentNode->childLeft->height : currentNode->childRight->height;
	//currentNode->height = (currentNode->height) + 1;
	

	//Check if need to balance
	//Check right child height
	int rightChildHeight = 0;
	if(currentNode->childRight != NULL){
		rightChildHeight = currentNode->childRight->height;
	}
	
	//Check left child height
	int leftChildHeight = 0;
	if(currentNode->childLeft != NULL){
		leftChildHeight = currentNode->childLeft->height;
	}
	
	int balance = leftChildHeight - rightChildHeight;
        //printf("Balance %d\n", balance);
	int key = toInsert->tickToRun;
  
    // If absolute value is greater than 1 then node is unbalanced, then there are 4 cases  
  
    // Left Left Case  
        if (balance > 1 && key < currentNode->childLeft->tickToRun){
          //printf("I am in left left\n");
          currentNode = rightRotate(currentNode);
          return currentNode;  
        }
      
  
    // Right Right Case  
        if (balance < -1 && key > currentNode->childRight->tickToRun){
          //printf("I am in right right\n");
          return leftRotate(currentNode);  
        }
  
    // Left Right Case  
    if (balance > 1 && key > currentNode->childLeft->tickToRun)  
    {  
        //printf("I am in left right\n");
        currentNode->childLeft = leftRotate(currentNode->childLeft);  
        return rightRotate(currentNode);  
    }  
  
    // Right Left Case  
    if (balance < -1 && key < currentNode->childRight->tickToRun)  
    {  
        //printf("I am in right left\n");
        currentNode->childRight = rightRotate(currentNode->childRight);  
        return leftRotate(currentNode);  
    }  
  
    /* return the (unchanged) node pointer */
    return currentNode;  
}  

//Find avl tree node with the specific p_tcb address
struct node *findNodeWithTCB(struct node *base, OS_TCB *p_tcb){
  if(base==NULL){
    return NULL;
  }
  if(base->taskToRun == p_tcb){
    return base;
  }
  else{
    struct node* returnedLeft = findNodeWithTCB(base->childLeft, p_tcb);
    if(returnedLeft == NULL){
      struct node* returnedRight = findNodeWithTCB(base->childRight, p_tcb);
      return returnedRight;
    }
    else{
      return returnedLeft;
    }
  }
}

struct node *obtainMostLeft(struct node *base){
    while(base->childLeft != NULL){
        base = base->childLeft;
    }
    return base;
}

// Get Balance factor of node N  
int getBalance(struct node *N){  
    if (N == NULL)  
        return 0;  
    
    if(N->childLeft != NULL && N->childRight != NULL){
       return N->childLeft->height - N->childRight->height;
    }
    else if(N->childLeft == NULL && N->childRight != NULL){
      return N->childRight->height;
    }
    else if(N->childLeft != NULL && N->childRight == NULL){
      return N->childLeft->height;
    }
    else if(N->childLeft == NULL && N->childRight == NULL){
      return 0;
    }
    return 0;
}  

// Recursive function to delete a node  
// with given key from subtree with  
// given root. It returns root of the  
// modified subtree.  
struct node *deleteNode(struct node* root, int key){  
      
    // STEP 1: PERFORM STANDARD BST DELETE  
  if (root == NULL){
        return root;  
        }
  
    // If the key to be deleted is smaller  
    // than the root's key, then it lies 
    // in left subtree  
  if ( key < root->tickToRun ){
        root->childLeft = deleteNode(root->childLeft, key);  
  }
  
    // If the key to be deleted is greater  
    // than the root's key, then it lies  
    // in right subtree  
  else if( key > root->tickToRun )  {
        root->childRight = deleteNode(root->childRight, key);  
  }
  
    // if key is same as root's key, then  
    // This is the node to be deleted, assume it is always the most left so dont deal with child, at most have a right child
    else
    {  
        if(root->childRight != NULL){
          return root->childRight;
        }
      
        return NULL; 
    }  
  
    // If the tree had only one node 
    // then return  
    if (root == NULL)  
    return root;  

    // STEP 2: UPDATE HEIGHT OF THE CURRENT NODE  
    updateHeight(root);   

  
    // STEP 3: GET THE BALANCE FACTOR OF  
    // THIS NODE (to check whether this  
    // node became unbalanced)  
    //Check right child height
    int rightChildHeight = 0;
    if(root->childRight != NULL){
            rightChildHeight = root->childRight->height;
    }
    
    //Check left child height
    int leftChildHeight = 0;
    if(root->childLeft != NULL){
            leftChildHeight = root->childLeft->height;
    }
    int balance = leftChildHeight - rightChildHeight; 
    //printf("Balance %d\n", balance);
  
    // If this node becomes unbalanced,  
    // then there are 4 cases  
  
    // Left Left Case  
    if (balance > 1 && getBalance(root->childLeft) >= 0){  
        //printf("I am in left left\n");
        return rightRotate(root);  
    }
    // Left Right Case  
    if (balance > 1 && getBalance(root->childLeft) < 0)  
    {  
        //printf("I am in left right\n");
        root->childLeft = leftRotate(root->childLeft);  
        return rightRotate(root);  
    }  
    // Right Right Case  
    if (balance < -1 && getBalance(root->childRight) <= 0){
        //printf("I am in right right\n");
        return leftRotate(root);  
    }
    // Right Left Case  
    if (balance < -1 && getBalance(root->childRight) > 0)  
    {  
        //printf("I am in right left\n");
        root->childRight = rightRotate(root->childRight);  
        return leftRotate(root);  
    }  
  
    return root;  
}     
    

/*
struct node *addNode(struct node* root, struct node* parent, struct node* childLeft,  struct node* childRight, int  height, int tickRemain, int tickToRun, void * taskToRun, int sameKeyValue){
    struct node *toAdd = malloc(sizeof(struct node));
    toAdd->parent = parent;
    toAdd->childLeft = childLeft;
    toAdd->childRight = childRight;
    toAdd->height = height;
    toAdd->tickRemain = tickRemain;
    toAdd->tickToRun = tickToRun;
    toAdd->taskToRun = taskToRun;
    toAdd->sameKeyValue = sameKeyValue;
    root = insertNode(root, toAdd);
    traverse(root);
    printf("\n");
    return root;
}
*/

