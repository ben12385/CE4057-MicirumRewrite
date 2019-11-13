#include <stdio.h>
#include <os.h>

int debug = 0;

void traverseHeap(OS_TCB* rootNode){
  if(rootNode == NULL)
      return;
  
  traverseHeap(rootNode->leftChildHeap);
  printf("Deadline of %d with number of child %d\n", rootNode->deadline, rootNode->numberOfChild);
  traverseHeap(rootNode->rightChildHeap);
  return;
}

void updateNodeNumber(OS_TCB* toUpdate){
    if(toUpdate->leftChildHeap != NULL && toUpdate->rightChildHeap != NULL){
        toUpdate->numberOfChild = toUpdate->leftChildHeap->numberOfChild + toUpdate->rightChildHeap->numberOfChild + 2;
    }
    else if(toUpdate->leftChildHeap != NULL && toUpdate->rightChildHeap == NULL){
        toUpdate->numberOfChild = (toUpdate->leftChildHeap->numberOfChild) + 1;
    }
    else if(toUpdate->leftChildHeap == NULL && toUpdate->rightChildHeap != NULL){
        toUpdate->numberOfChild = (toUpdate->rightChildHeap->numberOfChild) + 1;
    }
    else{
        toUpdate->numberOfChild = 0;
    }
}

OS_TCB* swapLeftWithRoot(OS_TCB* rootNode){
    OS_TCB* leftChildPointer = rootNode->leftChildHeap;
    OS_TCB* rightChildPointer = rootNode->rightChildHeap;        
    OS_TCB* leftChildleftChildPointer = leftChildPointer->leftChildHeap;
    OS_TCB* leftChildrightChildPointer = leftChildPointer->rightChildHeap;
    
    //Set root node to the left child pointers
    rootNode->rightChildHeap = leftChildrightChildPointer;
    rootNode->leftChildHeap = leftChildleftChildPointer;
    
    //Set leftChild node to the root child pointers
    leftChildPointer->rightChildHeap = rightChildPointer;
    leftChildPointer->leftChildHeap = rootNode;
    
    updateNodeNumber(rootNode);
    updateNodeNumber(leftChildPointer);
      
    //The new root
    return leftChildPointer;
}

OS_TCB* swapRightWithRoot(OS_TCB* rootNode){
    OS_TCB* rightChildPointer = rootNode->rightChildHeap;
    OS_TCB* leftChildPointer  = rootNode->leftChildHeap;        
    OS_TCB* rightChildleftChildPointer = rightChildPointer->leftChildHeap;
    OS_TCB* rightChildrightChildPointer = rightChildPointer->rightChildHeap;
    
    //Set root node to the left child pointers
    rootNode->rightChildHeap = rightChildrightChildPointer;
    rootNode->leftChildHeap = rightChildleftChildPointer;
    
    //Set leftChild node to the root child pointers
    rightChildPointer->leftChildHeap = leftChildPointer;
    rightChildPointer->rightChildHeap = rootNode;
    
    updateNodeNumber(rootNode);
    updateNodeNumber(rightChildPointer);
      
    //The new root
    return rightChildPointer;
}

//Insertion Node
OS_TCB* insertMinHeap(OS_TCB* toInsert, OS_TCB* rootNode){
    //Insert into most bottom
    if(rootNode == NULL){
            updateNodeNumber(toInsert);
            return toInsert;
    }
    if(rootNode->rightChildHeap != NULL && rootNode->leftChildHeap != NULL){
        if(rootNode->rightChildHeap->numberOfChild > rootNode->leftChildHeap->numberOfChild){
            rootNode->leftChildHeap = insertMinHeap(toInsert, rootNode->leftChildHeap);
            if(rootNode->leftChildHeap->deadline < rootNode->deadline){
                rootNode = swapLeftWithRoot(rootNode);
            }
        }
        else{
            rootNode->rightChildHeap = insertMinHeap(toInsert, rootNode->rightChildHeap);
            if(rootNode->rightChildHeap->deadline < rootNode->deadline){
                rootNode = swapRightWithRoot(rootNode);
            }
        }
    }
    else if(rootNode->rightChildHeap == NULL && rootNode->leftChildHeap != NULL){
        rootNode->rightChildHeap = insertMinHeap(toInsert, rootNode->rightChildHeap);
        if(rootNode->rightChildHeap->deadline < rootNode->deadline){
            rootNode = swapRightWithRoot(rootNode);
        }
    }
    else if(rootNode->rightChildHeap != NULL && rootNode->leftChildHeap == NULL){
        rootNode->leftChildHeap = insertMinHeap(toInsert, rootNode->leftChildHeap);
        if(rootNode->leftChildHeap->deadline < rootNode->deadline){
            rootNode = swapLeftWithRoot(rootNode);
        }
    }
    //Both null
    else{
        rootNode->leftChildHeap = insertMinHeap(toInsert, rootNode->leftChildHeap);
        if(rootNode->leftChildHeap->deadline < rootNode->deadline){
            rootNode = swapLeftWithRoot(rootNode);
        }
    }
    updateNodeNumber(rootNode);
    return rootNode;  
}  

OS_TCB* findNode(OS_TCB* rootNode, int key){
    if(rootNode->deadline == key){
        return rootNode;
    }
    
    OS_TCB* foundPointer = NULL;
    if(rootNode->leftChildHeap != NULL && key > rootNode->leftChildHeap->deadline){
        foundPointer = findNode(rootNode->leftChildHeap, key);
    }
    if(rootNode->rightChildHeap != NULL && key > rootNode->rightChildHeap->deadline && foundPointer == NULL){
        foundPointer = findNode(rootNode->rightChildHeap, key);
    }
    return foundPointer;
}

//Assume deletion is only root node for now
OS_TCB* deleteHeapNode(OS_TCB* root, int key){  
    //Find node
    OS_TCB* foundPointer = findNode(root, key);
    
    //Node does not exist in heap
    if(foundPointer == NULL){
      return root;
    }
    
    //Move foundPointer to the bottom
    root = removeMostBottom(foundPointer);
    
    return root;
}

OS_TCB* heapify(OS_TCB* rootNode){
	//Check if not smallest
	if(rootNode->rightChildHeap != NULL && rootNode->leftChildHeap != NULL){
		if(rootNode->rightChildHeap->deadline < rootNode->deadline || rootNode->leftChildHeap->deadline < rootNode->deadline){
			//Swap with left child because smallest  
			if(rootNode->rightChildHeap->deadline > rootNode->leftChildHeap->deadline){
				rootNode = swapLeftWithRoot(rootNode);
				rootNode->leftChildHeap = heapify(rootNode->leftChildHeap);
			}
			else{
				rootNode = swapRightWithRoot(rootNode);
				rootNode->rightChildHeap = heapify(rootNode->rightChildHeap);
			}
		}
	}
	else if(rootNode->rightChildHeap != NULL && rootNode->leftChildHeap == NULL){
		if(rootNode->rightChildHeap->deadline < rootNode->deadline){
			rootNode = swapRightWithRoot(rootNode);
			rootNode->rightChildHeap = heapify(rootNode->rightChildHeap);
		}
	}
	else if(rootNode->rightChildHeap == NULL && rootNode->leftChildHeap != NULL){
		if(rootNode->leftChildHeap->deadline < rootNode->deadline){
			rootNode = swapLeftWithRoot(rootNode);
			rootNode->leftChildHeap = heapify(rootNode->leftChildHeap);
		}
	}
	return rootNode;
}

OS_TCB* removeMostBottom(OS_TCB* rootNode){

    //Insert into most bottom
    if(rootNode->rightChildHeap == NULL && rootNode->leftChildHeap == NULL){
            return NULL;
    }
    
    //Swap with highest number of child
    if(rootNode->rightChildHeap != NULL && rootNode->leftChildHeap != NULL){
        if(rootNode->rightChildHeap->numberOfChild <= rootNode->leftChildHeap->numberOfChild){
			if(debug == 1)
				printf("Before swap root is %d and leftChild is %d\n", rootNode->deadline,rootNode->leftChildHeap->deadline);
            rootNode = swapLeftWithRoot(rootNode);
            rootNode->leftChildHeap = removeMostBottom(rootNode->leftChildHeap);
			if(debug == 1)
				printf("After swap root is %d and leftChild is %d\n", rootNode->deadline,rootNode->leftChildHeap->deadline);
        }
        else{
			if(debug == 1)
				printf("Before swap root is %d and rightChild is %d\n", rootNode->deadline,rootNode->rightChildHeap->deadline);
            rootNode = swapRightWithRoot(rootNode);
            rootNode->rightChildHeap = removeMostBottom(rootNode->rightChildHeap);
			if(debug == 1)
				printf("After swap root is %d and rightChild is %d\n", rootNode->deadline,rootNode->rightChildHeap->deadline);
        }
    }
    else if(rootNode->rightChildHeap != NULL && rootNode->leftChildHeap == NULL){
        if(debug == 1)
			printf("Before swap root is %d and rightChild is %d\n", rootNode->deadline,rootNode->rightChildHeap->deadline);
        rootNode = swapRightWithRoot(rootNode);
        rootNode->rightChildHeap = removeMostBottom(rootNode->rightChildHeap);
        if(debug == 1)
			printf("After swap root is %d and rightChild is %d\n", rootNode->deadline,rootNode->rightChildHeap->deadline);
    }
    else if(rootNode->rightChildHeap == NULL && rootNode->leftChildHeap != NULL){
        if(debug == 1)
			printf("Before swap root is %d and leftChild is %d\n", rootNode->deadline,rootNode->leftChildHeap->deadline);
        rootNode = swapLeftWithRoot(rootNode);
        rootNode->leftChildHeap = removeMostBottom(rootNode->leftChildHeap);
        if(debug == 1)
			printf("After swap root is %d and leftChild is %d\n", rootNode->deadline,rootNode->leftChildHeap->deadline);
    }
    updateNodeNumber(rootNode);
    
    //Heapify
    rootNode = heapify(rootNode);
	
    return rootNode;
}


