#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct 
{
   void *pNext;
   void *pPrev;
} ListNode;

typedef struct 
{
   void *pHead;
   void *pTail;
   int count;
   int offset;  // offset of ListNode within the structure
   int (*OrderFunction)(void *pNode1, void *pNode2);
} List;


void ListInitialize(List *pList, int nextPrevOffset,  
                           int (*orderFunction)(void *pNode1, void *pNode2));
                           static void ListAddNode(List *pList, void *pStructToAdd);
void ListAddNodeInOrder(List *pList, void *pStructToAdd);
void ListRemoveNode(List *pList, void *pStructToRemove);
void *ListPopNode(List *pList);
void *ListGetNextNode(List *pList,  void *pCurrentStucture);






