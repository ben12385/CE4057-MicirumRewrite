#include <os.h>
#include <stdio.h>

//TODO: Handle deletion of incomplete task holding on to mutex
//void updateSystemCeiling(struct stackList *newSystemCeiling, int resourceCeiling);
void popSystemCeiling(void);
void addMutexToMutexList(struct os_mutexSCP *mutex);

//highest preemptive level among all locked resources

struct stackList *scpSystemCeiling;
struct os_mutexSCP *mutexList;
struct rbNode *blockingRoot;

struct stackList idle = {-1,NULL, NULL};
void initSCP(){
  scpSystemCeiling = &idle;
  blockingRoot = NULL;
  mutexList = NULL;
}

void OSMutexSRPCreate(struct os_mutexSCP *mutex, CPU_CHAR *p_name){
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  
  mutex->available = 1;
  mutex->resourceCeiling = -1;
  mutex->currentHolder = NULL;
  mutex->nextMutex = NULL;
  mutex->NamePtr = p_name;
  addMutexToMutexList(mutex);
  
  CPU_CRITICAL_EXIT();
}

void OSMutexSRPDelete(struct os_mutexSCP *mutex){
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  
  mutex->available = 1;
  mutex->resourceCeiling = -1;
  mutex->currentHolder = NULL;
  mutex->nextMutex = NULL;
  
  popSystemCeiling();
  
  struct os_mutexSCP *toDeleteParent = mutexList;
  while(toDeleteParent->nextMutex != mutex){
    toDeleteParent = toDeleteParent->nextMutex;
  }
  toDeleteParent->nextMutex = mutex->nextMutex;
  
  CPU_CRITICAL_EXIT();
}

void scpMutexPend(struct os_mutexSCP *mutex, struct stackList *newSystemCeiling){
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  
  CPU_TS             ts_start;
  ts_start = OS_TS_GET();
  
  printf("%s Pend %s at tick %d\n", OSTCBCurPtr->NamePtr, mutex->NamePtr, OSTickCtr);
  if(mutex->available == 1){
    mutex->available = 0;
    mutex->currentHolder = OSTCBCurPtr;
    updateSystemCeiling(newSystemCeiling, mutex);
  }
  else{
    printf("Error in SRP implementation, pend yet unavailable\n");
  }
  
  //printf("Execution time for Pend is %d\n", OS_TS_GET() - ts_start);
  
  CPU_CRITICAL_EXIT();
}

void scpMutexPost(struct os_mutexSCP *mutex){
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  
  CPU_TS             ts_start;
  ts_start = OS_TS_GET();
  
  printf("%s Post %s at tick %d\n", OSTCBCurPtr->NamePtr, mutex->NamePtr, OSTickCtr);
  //Ensure that mutex is actually unavailable and that the holder is the one calling the post
  if(mutex->available == 0 && mutex->currentHolder == OSTCBCurPtr){
    mutex->available = 1;
    mutex->currentHolder = NULL;
    popSystemCeiling();
    
    //Ensure that there are tasks pending
    if(blockingRoot != NULL){
      //Most left is highest priority task
      struct rbNode *nodeToResume = obtainMostLeftrbNode(blockingRoot);
      blockingRoot = deleteNodeInRBTree(blockingRoot, nodeToResume);
      
      //Unblock all tasks that are able to run with the current system ceiling
      while(nodeToResume != NULL && canRun(nodeToResume->avlNode, nodeToResume)){
        struct node *mostLeftNode = nodeToResume->avlNode;
        
        //If preemtion level is higher than system ceiling, create the task because it means that task does not require any resources that have a resource ceiling lower than the system ceiling else it will be equal to less than the system ceiling
        OSTaskCreatePeriodic(mostLeftNode->taskToRun, mostLeftNode->p_name, mostLeftNode->p_task, mostLeftNode->p_arg, mostLeftNode->prio, mostLeftNode->p_stk_base, mostLeftNode->stk_limit, mostLeftNode->stk_size, mostLeftNode->q_size, mostLeftNode->time_quanta, mostLeftNode->p_ext, mostLeftNode->opt, mostLeftNode->p_err, mostLeftNode->period, mostLeftNode->realTickToRun, mostLeftNode->overflow, mostLeftNode);
        
        nodeToResume = obtainMostLeftrbNode(blockingRoot);
        blockingRoot = deleteNodeInRBTree(blockingRoot, nodeToResume);
      }
    }
    //Check if newly created task will preempt current task
    //printf("Execution time for Post is %d\n", OS_TS_GET() - ts_start);
    CPU_CRITICAL_EXIT();
    OSSched();
  }
  else{
    printf("Task tried to post mutex but either does not hold it or it is currently available\n");
    //printf("Execution time for Post is %d\n", OS_TS_GET() - ts_start);
    CPU_CRITICAL_EXIT();
  }
}

//To be called at the start for each mutex and the task that will be using
void calculateResourceCeiling(struct os_mutexSCP *mutexToAdjust, int period){
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  
  if(mutexToAdjust->resourceCeiling == -1 || period < mutexToAdjust->resourceCeiling){
    mutexToAdjust->resourceCeiling = period;
  }
  
  CPU_CRITICAL_EXIT();
}
 
int canRun(struct node *nodeToCheck, struct rbNode *rbNodeToBlock){
  CPU_TS             ts_start;
  ts_start = OS_TS_GET();
  
  //Check if system ceiling is set
  if(scpSystemCeiling->ceiling == -1){
    //No mutex currently being held so just insert into heap
    //printf("Able to run task %s\n", nodeToCheck->p_name);
    //printf("Execution time for SCP before insertion is %d\n", OS_TS_GET() - ts_start);
    return 1;
  }
  else{
    //Check if there are any tasks running
    if(OSRdyList[3].HeadPtr->deadline != NULL){
      //If there is a periodic task running\pending then check if preemption level of nodeToCheck is higher than system ceiling
      if(nodeToCheck->period < scpSystemCeiling->ceiling){
        //If preemption level is higher than system ceiling means that will preempt current task
        //printf("Able to run task %s and will preempt currently running task\n", nodeToCheck->p_name);
        //printf("Execution time for SCP before insertion is %d\n", OS_TS_GET() - ts_start);
        return 1;
      }
    }
    //Currently no task running/pending in ready list for periodic task so just add, this should not happen as system ceiling would then be -1
    else{
      //printf("Correct yet wrong because if no periodic task pending or running then system ceiling should be -1 and therefore clear at higher level\n");
      //printf("Execution time for SCP before insertion is %d\n", OS_TS_GET() - ts_start);
      return 1;
    }
  }
  
  printf("Block task %s from running\n", nodeToCheck->p_name);
  //Reset node and set key and taskTCB values
  rbNodeToBlock->parent=NULL;
  rbNodeToBlock->left=NULL;
  rbNodeToBlock->right=NULL;
  rbNodeToBlock->color=0;
  rbNodeToBlock->sameKeyValue=0;
  
  //Set to highest priority or highest preemeption level?
  rbNodeToBlock->key = nodeToCheck->period;
  rbNodeToBlock->avlNode = nodeToCheck; 
         
  blockingRoot = insertNodeToRBTree(blockingRoot, rbNodeToBlock);
  
  //printf("Execution time for SCP before insertion is %d\n", OS_TS_GET() - ts_start);
  return 0;
}

void updateSystemCeiling(struct stackList *newSystemCeiling, struct os_mutexSCP *mutexCeiling){
   //Less than is higher priority because of edf
  if(scpSystemCeiling->ceiling == -1 || scpSystemCeiling->ceiling > mutexCeiling->resourceCeiling){
    newSystemCeiling->ceiling = mutexCeiling->resourceCeiling;
    newSystemCeiling->ceilingCreator = mutexCeiling; 
  }
  else{
    newSystemCeiling->ceiling = scpSystemCeiling->ceiling;
    newSystemCeiling->ceilingCreator = scpSystemCeiling->ceilingCreator; 
  }
  newSystemCeiling->next = scpSystemCeiling;
  scpSystemCeiling = newSystemCeiling;
  printf("Current system ceiling %d\n", scpSystemCeiling->ceiling);
}

void popSystemCeiling(void){
  if(scpSystemCeiling->ceiling == -1){
    printf("Something went wrong with the system ceiling\n");
  }
  scpSystemCeiling = scpSystemCeiling->next;
  printf("Current system ceiling %d\n", scpSystemCeiling->ceiling);
}

void addMutexToMutexList(struct os_mutexSCP *mutex){
  if(mutexList != NULL){
    struct os_mutexSCP *toAppend = mutexList;
    while(toAppend->nextMutex != NULL){
      toAppend = toAppend->nextMutex;
    }
    toAppend->nextMutex = mutex;
  }
  else{
    mutexList = mutex;
  }
}

void removeHeldMutex(OS_TCB *toCheck){
  struct os_mutexSCP *mutexToCheck = mutexList;
  while(mutexToCheck != NULL){
    if(mutexToCheck->currentHolder == toCheck && mutexToCheck->available == 0){
        mutexToCheck->available = 1;
        mutexToCheck->currentHolder = NULL;
        popSystemCeiling();
        
        //Ensure that there are tasks pending
        if(blockingRoot != NULL){
          //Most left is highest priority task
          struct rbNode *nodeToResume = obtainMostLeftrbNode(blockingRoot);
          blockingRoot = deleteNodeInRBTree(blockingRoot, nodeToResume);
          
          //Unblock all tasks that are able to run with the current system ceiling
          while(nodeToResume != NULL && canRun(nodeToResume->avlNode, nodeToResume)){
            struct node *mostLeftNode = nodeToResume->avlNode;
            
            //If preemtion level is higher than system ceiling, create the task because it means that task does not require any resources that have a resource ceiling lower than the system ceiling else it will be equal to less than the system ceiling
            OSTaskCreatePeriodic(mostLeftNode->taskToRun, mostLeftNode->p_name, mostLeftNode->p_task, mostLeftNode->p_arg, mostLeftNode->prio, mostLeftNode->p_stk_base, mostLeftNode->stk_limit, mostLeftNode->stk_size, mostLeftNode->q_size, mostLeftNode->time_quanta, mostLeftNode->p_ext, mostLeftNode->opt, mostLeftNode->p_err, mostLeftNode->period, mostLeftNode->realTickToRun, mostLeftNode->overflow, mostLeftNode);
            
            nodeToResume = obtainMostLeftrbNode(blockingRoot);
            blockingRoot = deleteNodeInRBTree(blockingRoot, nodeToResume);
          }
        }
    }
    mutexToCheck = mutexToCheck->nextMutex;
  }
}

