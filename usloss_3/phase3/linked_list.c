/* ------------------------------------------------------------------------

Doubly Linked List 

Implementation of a doubly linked list that can be used with any structure.
Add a next and previous pointer to the structure.  The pointers are node
pointers.  Initialize the list with the offset of the structure's node 
pointers.

See the ListTest function for an example. 

Make sure to consider packing when calcuating offsets.

------------------------------------------------------------------------ */
// #include <stdio.h>
// #include <stdlib.h>
#include "./list.h"


/* ------------------------------------------------------------------------
   ListInitialize

   Purpose -      Initializes a List type.
   Parameters -   nextPrevOffset - offset from beginning of structure to
                                   the next and previous pointers with
                                   the structure that makes up the nodes.
                  orderFunction - used for sorting the list
   Returns -      None
   Side Effects - 
   ----------------------------------------------------------------------- */
void ListInitialize(List *pList, int nextPrevOffset,  
                           int (*orderFunction)(void *pNode1, void *pNode2))
{
   pList->pHead = pList->pTail = NULL;
   pList->count = 0;
   pList->OrderFunction = orderFunction;
   pList->offset = nextPrevOffset;
}

/* ------------------------------------------------------------------------
   ListAddNode

   Purpose -      Adds a node to the end of the list.

   Parameters -   List *pList        -  pointer to the list
                  void *pStructToAdd -  pointer to the structure to add

   Returns -      none
   Side Effects - 
   ----------------------------------------------------------------------- */
void ListAddNode(List *pList, void *pStructToAdd)
{
   ListNode *pTailNode;
   ListNode *pNodeToAdd;  // the next and prev pointers within proc to add
   int listOffset;

   listOffset = pList->offset;
   pNodeToAdd = (ListNode *)((unsigned char *)pStructToAdd + listOffset);
   pNodeToAdd->pNext = NULL;

   if (pList->pHead == NULL)
   {
      pList->pHead = pList->pTail = pStructToAdd;
      pNodeToAdd->pPrev = NULL;
   }
   else
   {
      // point to the list within proc_ptr
      pTailNode = (ListNode *)((unsigned char *)pList->pTail + listOffset);      
      pTailNode->pNext = pStructToAdd;
      pNodeToAdd->pPrev = pList->pTail;
      pNodeToAdd->pNext = NULL; 
      pList->pTail = pStructToAdd;
   }
   pList->count++;
}

/* ------------------------------------------------------------------------
   ListAddNodeInOrder

   Purpose -      Adds a node to the list based on the order function

   Parameters -   List *pList        -  pointer to the list
                  void *pStructToAdd -  pointer to the structure to add

   Returns -      none
   Side Effects - 
   ----------------------------------------------------------------------- */
void ListAddNodeInOrder(List *pList, void *pStructToAdd)
{
   ListNode *pCurrentNode;
   ListNode *pNodeToAdd;  // the next and prev pointers within proc to add
   void *pCurrentStructure;
   int listOffset;
   int positionFound = 0;
   
   // must have an order function
   if (pList->OrderFunction == NULL)
   {
      return;
   }

   listOffset = pList->offset;
   pNodeToAdd = (ListNode *)((unsigned char *)pStructToAdd + listOffset);
   pNodeToAdd->pNext = NULL;

   if (pList->pHead == NULL)
   {
      pList->pHead = pList->pTail = pStructToAdd;
      pNodeToAdd->pPrev = NULL;
      pList->count++;
   }
   else
   {
      // start at the beginning
      pCurrentNode = (ListNode *)((unsigned char *)pList->pHead + listOffset);

      // traverse the list looking for the insertion place.
      while (pCurrentNode != NULL && !positionFound)
      { 
         // keep a pointer to the structure         
         pCurrentStructure = (unsigned char *)pCurrentNode-listOffset;

         // OrderFunction returns a value of <= 0 if this is the position to insert
         if ((pList->OrderFunction(pCurrentStructure, pStructToAdd) <= 0) ||
              (pCurrentNode == NULL))
         {
            positionFound = 1;
         }
         else
         {
            /* if we are not at the end of the list, then move to the next node */
            if (pCurrentNode->pNext != NULL)
            {
                pCurrentNode = (ListNode *)((unsigned char *)pCurrentNode->pNext + listOffset);
            }
            else
            {
                pCurrentNode = NULL;
            }
         }
      }

      if (pCurrentNode == NULL)
      {
         // add to the end of the list
         ListAddNode(pList, pStructToAdd);
      }
      else
      {         
         // insert BEFORE the current node
         pNodeToAdd->pNext = pCurrentStructure;
         pNodeToAdd->pPrev = pCurrentNode->pPrev;
         pCurrentNode->pPrev = pStructToAdd;

         // move the head pointer if needed
         if (pNodeToAdd->pPrev == NULL)
         {
            pList->pHead = pStructToAdd;
         }
         else
         {
            ListNode *pPrevNode;
            pPrevNode = (ListNode *)((unsigned char *)pNodeToAdd->pPrev + listOffset);
            pPrevNode->pNext = pStructToAdd;
         }
         pList->count++;

      }
   }
}

/* ------------------------------------------------------------------------
   ListRemoveNode

   Purpose -      Removes the specified node from the list.

   Parameters -   List *pList           -  pointer to the list
                  void *pStructToRemove -  pointer to the structure to remove

   Returns -      None

   Side Effects - 
   ----------------------------------------------------------------------- */
void ListRemoveNode(List *pList, void *pStructToRemove)
{
   ListNode *pPrevNode=NULL;
   ListNode *pNextNode=NULL;
   ListNode *pNodeToRemove;  // the next and prev pointers within proc to add
   int listOffset;

   listOffset = pList->offset;
   if (pList->count > 0)
   {
      pNodeToRemove = (ListNode *)((unsigned char *)pStructToRemove + listOffset);

      // if this is not the head and
      // prev and next are NULL, then the node is not on the list
      if (pList->pHead == pStructToRemove || pNodeToRemove->pNext != NULL || pNodeToRemove->pPrev != NULL)
      {
         if (pNodeToRemove->pPrev != NULL)
         {
            pPrevNode = (ListNode *)((unsigned char *)pNodeToRemove->pPrev  + listOffset);      
         }
         if (pNodeToRemove->pNext  != NULL)
         {
            pNextNode = (ListNode *)((unsigned char *)pNodeToRemove->pNext  + listOffset);
         }

         if (pPrevNode != NULL && pNextNode != NULL)
         {
            pPrevNode->pNext = pNodeToRemove->pNext;
            pNextNode->pPrev = pNodeToRemove->pPrev;
         }
         else
         {
            if (pPrevNode == NULL)
            {
               /* replace the first node */
               pList->pHead = pNodeToRemove->pNext; 
               if (pList->pHead)
               {
                  pNextNode->pPrev = NULL;
               }
            }
            if (pNextNode == NULL)
            {
               /* replace the tail */
               pList->pTail = pNodeToRemove->pPrev;
               if (pList->pTail)
               {
                  pPrevNode->pNext = NULL;
               }
            }
         }
         pList->count--;

         pNodeToRemove->pNext = NULL;
         pNodeToRemove->pPrev = NULL;
      }
   }
}
/* ------------------------------------------------------------------------
   ListPopNode

   Purpose -      Removes the first node from the list and returns a pointer 
                  to it.

   Parameters -   List *pList         -  pointer to the list
                  
   Returns -      The function returns a pointer to the removed node.

   Side Effects - 
   ----------------------------------------------------------------------- */
void *ListPopNode(List *pList)
{
   void *pNode=NULL;   
   ListNode *pNodeToRemove;  // the next and prev pointers within proc to add
   int listOffset;

   listOffset = pList->offset;
   if (pList->count > 0)
   {
      pNodeToRemove = (ListNode *)((unsigned char *)pList->pHead + listOffset);

      pNode = pList->pHead;
      pList->pHead = pNodeToRemove->pNext;
      pList->count--;

      // clear prev and next
      pNodeToRemove->pNext = NULL;
      pNodeToRemove->pPrev = NULL;
   }
   return pNode;
}


/* ------------------------------------------------------------------------
   ListGetNextNode

   Purpose -      Gets the next node releative to current node.  Returns
                  the first node if pCurrentStucture is NULL

   Parameters -   List *pList             -  pointer to the list
                  void *pCurrentStucture  -  pointer to current struct
                  
   Returns -      The function returns a pointer to the next structure.

   Side Effects - 
   ----------------------------------------------------------------------- */
void *ListGetNextNode(List *pList,  void *pCurrentStucture)
{
   void *pNode=NULL;   
   ListNode *pCurrentNode;  // Node part of the current structure
   int listOffset;

   if (pList != NULL)
   {
      if (pCurrentStucture == NULL)
      {
         pNode = pList->pHead;
      }
      else
      {
         listOffset = pList->offset;
         if (pList->count > 0)
         {
            pCurrentNode = (ListNode *)((unsigned char *)pCurrentStucture + listOffset);
            pNode = pCurrentNode->pNext;
         }
      }
   }
   return pNode;
}




/* ------------------------------------------------------------------------
   List Testing
   ----------------------------------------------------------------------- */
typedef struct test_list
{
    struct test_list *pNextAll;            // All List pNext
    struct test_list *pPrevAll;            // All List pPrev
    struct test_list *pNextEvenOrOdd;      // Even or Odd List pNext
    struct test_list *pPrevEvenOrOdd;      // Even or Odd List pPrev
    struct test_list *pNextAllOrdered;            // All List pNext
    struct test_list *pPrevAllOrdered;            // All List pPrev
    int value;
} TestListStructure;


/* Function orders ascending. Return <= 0 when p1 is greater than or equal to p2 */
int OrderByTest(void *pStruct1, void *pStruct2)
{
   TestListStructure *p1, *p2;
   p1 = (TestListStructure *)pStruct1;
   p2 = (TestListStructure *)pStruct2;
   return p2->value - p1->value;
}

#ifdef STANDALONE_TEST
int main()
#else
int ListTest()
#endif
{
    TestListStructure allPossibleNodes[100];
    TestListStructure *pTestStruct;
    List listAll, listEvens, listOdds, listAllOrdered;
    int offsetList2;
    int offsetList3;

    memset(allPossibleNodes, 0, sizeof(allPossibleNodes));

    // calculate the offset of list 2
    offsetList2 = (void *)&allPossibleNodes[0].pNextEvenOrOdd - (void *)&allPossibleNodes[0];
    printf("The offset of the second list is %d bytes.\n", offsetList2);

    offsetList3 = (void *)&allPossibleNodes[0].pNextAllOrdered - (void *)&allPossibleNodes[0];

    ListInitialize(&listAll, 0, OrderByTest);
    ListInitialize(&listAllOrdered, offsetList3, OrderByTest);
    ListInitialize(&listEvens, offsetList2, OrderByTest);
    ListInitialize(&listOdds, offsetList2, OrderByTest);

    for (int i=0; i < 10; ++i)
    {
        allPossibleNodes[i].value = rand() % 100;
        ListAddNode(&listAll, &allPossibleNodes[i]);
        if ((allPossibleNodes[i].value % 2) == 0)
        {
            ListAddNode(&listEvens, &allPossibleNodes[i]);
        }
        else
        {
            ListAddNode(&listOdds, &allPossibleNodes[i]);
        }
        printf("Ordered: adding %d at %d, 0x%x\n", allPossibleNodes[i].value,i, &allPossibleNodes[i]);
        ListAddNodeInOrder(&listAllOrdered, &allPossibleNodes[i]);
    }
    
    printf("There are %d even values.\n", listEvens.count);
    while ((pTestStruct = ListPopNode(&listEvens)) != NULL)
    {
        printf("%d\n", pTestStruct->value);
    }
    
    printf("There are %d odd values.\n", listOdds.count);
    while ((pTestStruct = ListPopNode(&listOdds)) != NULL)
    {
        printf("%d\n", pTestStruct->value);
    }

    printf("There are %d total values.\n", listAll.count);
    while ((pTestStruct = ListPopNode(&listAll)) != NULL)
    {
        printf("%d\n", pTestStruct->value);
    }

    for (int i=0; i < 10; ++i)
    {
        ListAddNode(&listAll, &allPossibleNodes[i]);
    }
 
    // remove from front
    ListRemoveNode(&listAll, &allPossibleNodes[0]);
    // remove from middle
    ListRemoveNode(&listAll, &allPossibleNodes[4]);
    // remove from end
    ListRemoveNode(&listAll, &allPossibleNodes[9]);
    printf("There are %d values in the test list after removing 0, 4, and 9.\n", listAll.count);
    while ((pTestStruct = ListPopNode(&listAll)) != NULL)
    {
        printf("%d\n", pTestStruct->value);
    }

    printf("All Ordered: there are %d total values.\n", listAllOrdered.count);
    while ((pTestStruct = ListPopNode(&listAllOrdered)) != NULL)
    {
        printf("%d\n", pTestStruct->value);
    }

    return 0;
}
