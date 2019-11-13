#include <os.h>
#include <stdio.h>
struct node *root; 

void OSTaskCreatePeriodicMutex(struct node *mostLeftNode){
  //Check to see if still running
  if(mostLeftNode->taskToRun->TaskState != (OS_STATE)OS_TASK_STATE_DEL){
    printf("Task %s did not complete and deadline reached\n",mostLeftNode->p_name);
    removeHeldMutex(mostLeftNode->taskToRun);
  }
  printf("Create task %s at tick %d\n", mostLeftNode->p_name, OSTickCtr);
  if(canRun(mostLeftNode, &(mostLeftNode->forBlocking))){
      OS_ERR *err = mostLeftNode->p_err;
      OSTaskCreatePeriodic(mostLeftNode->taskToRun, mostLeftNode->p_name, mostLeftNode->p_task, mostLeftNode->p_arg, mostLeftNode->prio, mostLeftNode->p_stk_base, mostLeftNode->stk_limit, mostLeftNode->stk_size, mostLeftNode->q_size, mostLeftNode->time_quanta, mostLeftNode->p_ext, mostLeftNode->opt, err, mostLeftNode->period, mostLeftNode->realTickToRun, mostLeftNode->overflow, mostLeftNode);
  }  
  
}

void  OSTaskCreatePeriodic (OS_TCB        *p_tcb,
                    CPU_CHAR      *p_name,
                    OS_TASK_PTR    p_task,
                    void          *p_arg,
                    OS_PRIO        prio,
                    CPU_STK       *p_stk_base,
                    CPU_STK_SIZE   stk_limit,
                    CPU_STK_SIZE   stk_size,
                    OS_MSG_QTY     q_size,
                    OS_TICK        time_quanta,
                    void          *p_ext,
                    OS_OPT         opt,
                    OS_ERR        *p_err,
                    int frequency,
                    int tickLastRun,
                    int overflow,
                    struct node *avlNode)
{ 
  avlNode->childLeft = NULL;
  avlNode->childRight = NULL; 
  
  avlNode->realTickToRun = tickLastRun+frequency;
  avlNode->tickToRun = tickLastRun+frequency;
  avlNode->period = frequency;
  
  //Check if overflow
  if(tickLastRun+frequency < frequency){
    avlNode->overflow = (overflow + 1) % 2;
  }
  else{
    avlNode->overflow = overflow;
  }
  
  avlNode->sameKeyValue = 0;
  
  avlNode->p_err=p_err;
  avlNode->taskToRun=p_tcb;
  
  avlNode->p_name = p_name;
  avlNode->p_task = p_task;
  avlNode->p_arg = p_arg;
  avlNode->prio = prio;
  avlNode->p_stk_base = p_stk_base;
  avlNode->stk_limit = stk_limit;
  avlNode->stk_size = stk_size;
  avlNode->q_size = q_size;
  avlNode->time_quanta = time_quanta;
  avlNode->p_ext = p_ext;
  avlNode->opt = opt;
  
  p_tcb->numberOfChild = 0;
  p_tcb->leftChildHeap = NULL;
  p_tcb->rightChildHeap = NULL;
  p_tcb->deadline = avlNode->realTickToRun;
  
  p_tcb->period = frequency;
  
    CPU_STK_SIZE   i;
#if OS_CFG_TASK_REG_TBL_SIZE > 0u
    OS_OBJ_QTY     reg_nbr;
#endif
    CPU_STK       *p_sp;
    CPU_STK       *p_stk_limit;
    CPU_SR_ALLOC();



#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == DEF_TRUE) {
       *p_err = OS_ERR_ILLEGAL_CREATE_RUN_TIME;
        return;
    }
#endif

#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u
    if (OSIntNestingCtr > (OS_NESTING_CTR)0) {              /* ---------- CANNOT CREATE A TASK FROM AN ISR ---------- */
        *p_err = OS_ERR_TASK_CREATE_ISR;
        return;
    }
#endif

#if OS_CFG_ARG_CHK_EN > 0u                                  /* ---------------- VALIDATE ARGUMENTS ------------------ */
    if (p_tcb == (OS_TCB *)0) {                             /* User must supply a valid OS_TCB                        */
        *p_err = OS_ERR_TCB_INVALID;
        return;
    }
    if (p_task == (OS_TASK_PTR)0) {                         /* User must supply a valid task                          */
        *p_err = OS_ERR_TASK_INVALID;
        return;
    }
    if (p_stk_base == (CPU_STK *)0) {                       /* User must supply a valid stack base address            */
        *p_err = OS_ERR_STK_INVALID;
        return;
    }
    if (stk_size < OSCfg_StkSizeMin) {                      /* User must supply a valid minimum stack size            */
        *p_err = OS_ERR_STK_SIZE_INVALID;
        return;
    }
    if (stk_limit >= stk_size) {                            /* User must supply a valid stack limit                   */
        *p_err = OS_ERR_STK_LIMIT_INVALID;
        return;
    }
    if (prio >= OS_CFG_PRIO_MAX) {                          /* Priority must be within 0 and OS_CFG_PRIO_MAX-1        */
        *p_err = OS_ERR_PRIO_INVALID;
        return;
    }
#endif

#if OS_CFG_ISR_POST_DEFERRED_EN > 0u
    if (prio == (OS_PRIO)0) {
        if (p_tcb != &OSIntQTaskTCB) {
            *p_err = OS_ERR_PRIO_INVALID;                   /* Not allowed to use priority 0                          */
            return;
        }
    }
#endif

    if (prio == (OS_CFG_PRIO_MAX - 1u)) {
        if (p_tcb != &OSIdleTaskTCB) {
            *p_err = OS_ERR_PRIO_INVALID;                   /* Not allowed to use same priority as idle task          */
            return;
        }
    }

    OS_TaskInitTCB(p_tcb);                                  /* Initialize the TCB to default values                   */

    *p_err = OS_ERR_NONE;
                                                            /* --------------- CLEAR THE TASK'S STACK --------------- */
    if ((opt & OS_OPT_TASK_STK_CHK) != (OS_OPT)0) {         /* See if stack checking has been enabled                 */
        if ((opt & OS_OPT_TASK_STK_CLR) != (OS_OPT)0) {     /* See if stack needs to be cleared                       */
            p_sp = p_stk_base;
            for (i = 0u; i < stk_size; i++) {               /* Stack grows from HIGH to LOW memory                    */
                *p_sp = (CPU_STK)0;                         /* Clear from bottom of stack and up!                     */
                p_sp++;
            }
        }
    }
                                                            /* ------- INITIALIZE THE STACK FRAME OF THE TASK ------- */
#if (CPU_CFG_STK_GROWTH == CPU_STK_GROWTH_HI_TO_LO)
    p_stk_limit = p_stk_base + stk_limit;
#else
    p_stk_limit = p_stk_base + (stk_size - 1u) - stk_limit;
#endif

    p_sp = OSTaskStkInit(p_task,
                         p_arg,
                         p_stk_base,
                         p_stk_limit,
                         stk_size,
                         opt);

                                                            /* -------------- INITIALIZE THE TCB FIELDS ------------- */
    p_tcb->TaskEntryAddr = p_task;                          /* Save task entry point address                          */
    p_tcb->TaskEntryArg  = p_arg;                           /* Save task entry argument                               */

    p_tcb->NamePtr       = p_name;                          /* Save task name                                         */

    p_tcb->Prio          = prio;                            /* Save the task's priority                               */

    p_tcb->StkPtr        = p_sp;                            /* Save the new top-of-stack pointer                      */
    p_tcb->StkLimitPtr   = p_stk_limit;                     /* Save the stack limit pointer                           */

    //p_tcb->TimeQuanta    = time_quanta;                     /* Save the #ticks for time slice (0 means not sliced)    */
#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
    if (time_quanta == (OS_TICK)0) {
        p_tcb->TimeQuantaCtr = OSSchedRoundRobinDfltTimeQuanta;
    } else {
        p_tcb->TimeQuantaCtr = time_quanta;
    }
#endif
    p_tcb->ExtPtr        = p_ext;                           /* Save pointer to TCB extension                          */
    p_tcb->StkBasePtr    = p_stk_base;                      /* Save pointer to the base address of the stack          */
    p_tcb->StkSize       = stk_size;                        /* Save the stack size (in number of CPU_STK elements)    */
    p_tcb->Opt           = opt;                             /* Save task options                                      */

#if OS_CFG_TASK_REG_TBL_SIZE > 0u
    for (reg_nbr = 0u; reg_nbr < OS_CFG_TASK_REG_TBL_SIZE; reg_nbr++) {
        p_tcb->RegTbl[reg_nbr] = (OS_REG)0;
    }
#endif

#if OS_CFG_TASK_Q_EN > 0u
    OS_MsgQInit(&p_tcb->MsgQ,                               /* Initialize the task's message queue                    */
                q_size);
#endif

    OSTaskCreateHook(p_tcb);                                /* Call user defined hook                                 */

                                                            /* --------------- ADD TASK TO READY LIST --------------- */
    OS_CRITICAL_ENTER();
    OS_PrioInsert(p_tcb->Prio);
    OS_RdyListInsertTail(p_tcb);
    OS_TickListInsertPeriodic(avlNode);

#if OS_CFG_DBG_EN > 0u
    OS_TaskDbgListAdd(p_tcb);
#endif

    OSTaskQty++;                                            /* Increment the #tasks counter                           */

    if (OSRunning != OS_STATE_OS_RUNNING) {                 /* Return if multitasking has not started                 */
        OS_CRITICAL_EXIT();
        return;
    }

    OS_CRITICAL_EXIT_NO_SCHED();
    

    OSSched();
}


void  OSTaskDelPeriodic (struct node *avlNode){
    CPU_SR_ALLOC();

    OS_ERR  *p_err = avlNode->p_err;
    OS_TCB  *p_tcb = avlNode->taskToRun;
    
    root = deleteNode(root, avlNode->tickToRun);

#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u
    if (OSIntNestingCtr > (OS_NESTING_CTR)0) {              /* See if trying to delete from ISR                       */
       *p_err = OS_ERR_TASK_DEL_ISR;
        return;
    }
#endif

    if (p_tcb == &OSIdleTaskTCB) {                          /* Not allowed to delete the idle task                    */
        *p_err = OS_ERR_TASK_DEL_IDLE;
        return;
    }

#if OS_CFG_ISR_POST_DEFERRED_EN > 0u
    if (p_tcb == &OSIntQTaskTCB) {                          /* Cannot delete the ISR handler task                     */
        *p_err = OS_ERR_TASK_DEL_INVALID;
        return;
    }
#endif

    if (p_tcb == (OS_TCB *)0) {                             /* Delete 'Self'?                                         */
        CPU_CRITICAL_ENTER();
        p_tcb  = OSTCBCurPtr;                               /* Yes.                                                   */
        CPU_CRITICAL_EXIT();
    }

    OS_CRITICAL_ENTER();
    switch (p_tcb->TaskState) {
        case OS_TASK_STATE_RDY:
             OS_RdyListRemove(p_tcb);
             break;

        case OS_TASK_STATE_SUSPENDED:
             break;

        case OS_TASK_STATE_DLY:                             /* Task is only delayed, not on any wait list             */
        case OS_TASK_STATE_DLY_SUSPENDED:
             OS_TickListRemove(p_tcb);
             break;

        case OS_TASK_STATE_PEND:
        case OS_TASK_STATE_PEND_SUSPENDED:
        case OS_TASK_STATE_PEND_TIMEOUT:
        case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:
             OS_TickListRemove(p_tcb);
             switch (p_tcb->PendOn) {                       /* See what we are pending on                             */
                 case OS_TASK_PEND_ON_NOTHING:
                 case OS_TASK_PEND_ON_TASK_Q:               /* There is no wait list for these two                    */
                 case OS_TASK_PEND_ON_TASK_SEM:
                      break;

                 case OS_TASK_PEND_ON_FLAG:                 /* Remove from wait list                                  */
                 case OS_TASK_PEND_ON_MULTI:
                 case OS_TASK_PEND_ON_MUTEX:
                 case OS_TASK_PEND_ON_Q:
                 case OS_TASK_PEND_ON_SEM:
                      OS_PendListRemove(p_tcb);
                      break;

                 default:
                      break;
             }
             break;

        default:
            OS_CRITICAL_EXIT();
            *p_err = OS_ERR_STATE_INVALID;
            return;
    }

#if OS_CFG_TASK_Q_EN > 0u
    (void)OS_MsgQFreeAll(&p_tcb->MsgQ);                     /* Free task's message queue messages                     */
#endif

    OSTaskDelHook(p_tcb);                                   /* Call user defined hook                                 */

#if OS_CFG_DBG_EN > 0u
    OS_TaskDbgListRemove(p_tcb);
#endif
    OSTaskQty--;                                            /* One less task being managed                            */

    OS_TaskInitTCB(p_tcb);                                  /* Initialize the TCB to default values                   */
    p_tcb->TaskState = (OS_STATE)OS_TASK_STATE_SUSPENDED;         /* Indicate that the task was deleted                     */

    OS_CRITICAL_EXIT_NO_SCHED();
    OSSched();                                              /* Find new highest priority task                         */

    *p_err = OS_ERR_NONE;
}

void  OS_TickListInsertPeriodic (struct node *avlNode)
{
    //struct node *avlNode = findNodeWithTCB(root, p_tcb);
    
    
    //OS_TCB *p_tcb = avlNode->taskToRun;
    
    //int previousRunTime = avlNode->lastRunTime;
    //int period = avlNode->period;
   
    int tickTrack = avlNode->tickToRun + 1;
    while(tickTrack != avlNode->tickToRun){
      //printf("Inserting %d %s\n", avlNode->tickToRun, avlNode->p_name);
      tickTrack = avlNode->tickToRun;
      root = insertNode(avlNode, root);
    }    
    //printf("Run Complete\n");
    
    //Update node structure
    //avlNode->tickRemain = period;
    //avlNode->tickToRun = OSTickCtr + period;
    
    /* Pending as may affect readylist
    //Update tcb
    tick_next  = p_tcb->TickCtrPrev + time;
    tick_delta = tick_next - OSTickCtr - 1u;
    if (tick_delta < time) {                             // If next periodic delay did NOT already occur, ... 
        p_tcb->TickCtrMatch = tick_next;                 // ... set  next  periodic delay;                ...
    } else {
        p_tcb->TickCtrMatch = OSTickCtr + time;          // ... else reset periodic delay.                  
    }
    p_tcb->TickRemain   = p_tcb->TickCtrMatch - OSTickCtr;
    p_tcb->TickCtrPrev  = p_tcb->TickCtrMatch;
    */
   
    //Replace Spoke insertion with AVL Tree insertion
    //Insert p_tcb which is the structure of the entire task
    
   
    //*p_err               = OS_ERR_NONE;
}

void OS_StartPeriodicTask(void){
      flag = 1;
      baseTick = OSTickCtr;
}

void OS_TickListUpdatePeriodic(void)
{
    //No periodic task skip
  
    if(root == NULL || flag == 0)
    {
        return;
    }
  
    CPU_TS             ts_start;
    CPU_TS             ts_end;
    CPU_SR_ALLOC();
    
    //printf("Tick Count %d\n", OSTickCtr);

    OS_CRITICAL_ENTER();
    ts_start = OS_TS_GET();
       
    struct node *mostLeftNode = obtainMostLeft(root);
    
    //printf("Most left node tickToRun %d\n", mostLeftNode->tickToRun);
    //printf("Equation validity %d\n", !(mostLeftNode->tickToRun > OSTickCtr && mostLeftNode->sameKeyValue == 0));
    
    //printf("\n");
    
    while((mostLeftNode->tickToRun) + baseTick <= OSTickCtr || mostLeftNode->sameKeyValue == 1){ 
        mostLeftNode = obtainMostLeft(root);
        //printf("Running %s at tick %d\n", mostLeftNode->p_name, OSTickCtr);
        
        OS_ERR *err=mostLeftNode->p_err;
        
        root = deleteNode(root, mostLeftNode->tickToRun);
        //traverse(root);
        //printf("\n");
        
        //Update tickToRun and realTickToRun
        //mostLeftNode->realTickToRun = mostLeftNode->realTickToRun + mostLeftNode->period;
        //mostLeftNode->tickToRun = mostLeftNode->realTickToRun;
        
        //Reset node
        mostLeftNode->childLeft = NULL;
        mostLeftNode->childRight = NULL;
        mostLeftNode->height = 1;
        mostLeftNode->sameKeyValue = 0;
        
        //printf("Inserting %d\n", mostLeftNode->tickToRun);
        //Create periodic task and insert it into the tree
        OSTaskCreatePeriodicMutex(mostLeftNode);
        //OSTaskCreatePeriodic(mostLeftNode->taskToRun, mostLeftNode->p_name, mostLeftNode->p_task, mostLeftNode->p_arg, mostLeftNode->prio, mostLeftNode->p_stk_base, mostLeftNode->stk_limit, mostLeftNode->stk_size, mostLeftNode->q_size, mostLeftNode->time_quanta, mostLeftNode->p_ext, mostLeftNode->opt, err, mostLeftNode->period, mostLeftNode->realTickToRun, mostLeftNode->overflow, mostLeftNode);
        mostLeftNode = obtainMostLeft(root);
      
    }
    
 
    ts_end = OS_TS_GET() - ts_start;                                   /* Measure execution time of tick task         */
    if (ts_end > OSTickTaskPeriodicTimeMax) {
        OSTickTaskPeriodicTimeMax = ts_end;
    }
    OS_CRITICAL_EXIT();
}
