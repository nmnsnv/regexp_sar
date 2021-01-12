
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "sarcore.h"

// internal structs:

typedef struct
{
    void *args;
    sar_matchCallback callback;
    sar_freeCallbackArgs freeCallback;

    // used to count how many times this struct is pointed at
    int pointedCount;

    // TODO: move this 2 fields out of here
    int lastFrom;
    int lastTo;

    // TODO: find better method, not critical
    unsigned int matchKey;
} sar_matchCallbackStruct_t;

// internal methods prototype:

static void sar_initNode(sarNode_t *node, sar_nodeInstruction_t *nodeInstruction);
static void sar_freeNode(sarNode_t *node);

static void sar_initNodeInstruction(sar_nodeInstruction_t *nodeInstruction,
                                    int isNeg, sar_nodeSuffix_e suffix, sar_nodeType_e type, char charVal);
static void sar_freeNodeInstruction(sar_nodeInstruction_t *nodeInstruction);

static void sar_initNodeComplexInstruction(sar_nodeComplexInstruction_t *nodeInstruction,
                                           int isNeg, sar_nodeComplexSuffix_e suffix,
                                           sar_nodeType_e type, char charVal);
static void sar_freeNodeComplexInstruction(sar_nodeComplexInstruction_t *complexNodeInstruction);

static void sar_initMatchCallbackStruct(sar_matchCallbackStruct_t *callbackStruct,
                                        sar_matchCallback func,
                                        sar_freeCallbackArgs freeFunc,
                                        void *args);
static void sar_freeMatchCallbackStruct(sar_matchCallbackStruct_t *callbacStruct);

static sar_linkedListNode_t *sar_buildComplexNodeInstructions(char *regexpStr, int len);

static int sar_buildCurNodeInstruction(sar_nodeComplexInstruction_t *nodeInstruction, char *regexpStr,
                                       int pos, int len, int expectSuffix, int expectPrefix);
static void sar_setBackslashChar(sar_nodeComplexInstruction_t *nodeInstruction, char charVal);
static void sar_setCharSuffix(sar_nodeComplexInstruction_t *nodeInstruction, char charVal);
static int sar_isCharSuffix(char charVal);

static void sar_matchAtInternal(sarObject_t *sarObject, char *matchStr, int at, int len);

static void sar_appendCallback(sarNode_t *node, sar_matchCallbackStruct_t *callbackStruct);
static void sar_insertNode(sarNode_t *intoNode, sarNode_t *node, int isNegNode, int isPlusNode);
static int sar_findNode(sar_nodeInstruction_t *nodeInstruction, sarNode_t *node, sarNode_t **resultNode);
static void sar_triggerCallbacks(sarNode_t *node, int from, int to, unsigned int matchKey);
static void sar_buildPathRecursive(sarNode_t *node, sar_linkedListNode_t *nodeInstructionList,
                                   sar_matchCallbackStruct_t *callbackStruct);
static int sar_getNextActiveNodes(sarNode_t *curNode, char curChar,
                                  sar_linkedListNode_t *newActiveNodesHead,
                                  sar_linkedListNode_t **lastNodePtr,
                                  int *isInLoop,
                                  int isInNegNodes);
static int sar_findCharNode(sarNode_t *node, char charVal, sarNode_t **resultNode);
static void sar_insertCharNode(sarNode_t *intoNode, sarNode_t *node);
static sar_linkedListNode_t *sar_findNegCharNode(sarNode_t *node, char charVal, sar_linkedListNode_t **lastNode);
static int sar_isNodeActivated(sarNode_t *node, char c);

static sar_linkedListNode_t *sar_convertConvertComplexInstructions(sar_linkedListNode_t *complexInstructions);
static sar_linkedListNode_t *sar_cloneAllRegexpsInstructions(sar_linkedListNode_t *originalRegexpsList);
static sar_linkedListNode_t *sar_removeFromRegexpsList(sar_linkedListNode_t *regexpsList, sar_linkedListNode_t *regexpInsts);

static int sar_compareRegexpInstructions(sar_linkedListNode_t *regexpInst1, sar_linkedListNode_t *regexpInst2);
static int sar_compareNodeInstruction(sar_nodeInstruction_t *node1, sar_nodeInstruction_t *node2);
static void sar_freeRegexpInstructions(sar_linkedListNode_t *regexpInstructions);

static unsigned int sar_genMatchingKey(sarObject_t *sarObject);

#ifdef SAR_DEBUG_PRINT
static void sar_debug_printNodeInstruction(sar_nodeInstruction_t *inst);
static void sar_debug_printRegexp(sar_linkedListNode_t *regexpInstructions);
static void sar_debug_printAllRegexps(sar_linkedListNode_t *instructions);
#endif

static void sar_initNode(sarNode_t *node, sar_nodeInstruction_t *nodeInstruction)
{
    node->nodeInstruction = nodeInstruction;

    node->alphaNode = NULL;
    node->alphaNumNodes = NULL;
    node->spaceNodes = NULL;
    node->digitNode = NULL;
    node->dotNode = NULL;
    node->negativeNode = NULL;
    node->plusNode = NULL;

    node->callbackList = malloc(sizeof(sar_linkedListNode_t));
    node->callbackList->next = NULL;
    node->callbackList->val = NULL;

    node->charVals = NULL;
    node->charNodes = NULL;
    node->charNodesSize = 0;
}

static void sar_freeNode(sarNode_t *node)
{
    sar_freeNodeInstruction(node->nodeInstruction);
    free(node->nodeInstruction);

    if (node->alphaNode != NULL)
    {
        sar_freeNode(node->alphaNode);
        free(node->alphaNode);
    }
    if (node->alphaNumNodes != NULL)
    {
        sar_freeNode(node->alphaNumNodes);
        free(node->alphaNumNodes);
    }
    if (node->spaceNodes != NULL)
    {
        sar_freeNode(node->spaceNodes);
        free(node->spaceNodes);
    }
    if (node->digitNode != NULL)
    {
        sar_freeNode(node->digitNode);
        free(node->digitNode);
    }
    if (node->dotNode != NULL)
    {
        sar_freeNode(node->dotNode);
        free(node->dotNode);
    }
    if (node->negativeNode != NULL)
    {
        sar_freeNode(node->negativeNode);
        free(node->negativeNode);
    }
    if (node->plusNode != NULL)
    {
        sar_freeNode(node->plusNode);
        free(node->plusNode);
    }

    int i;
    for (i = 0; i < node->charNodesSize; i++)
    {
        sar_freeNode(node->charNodes[i]);
        free(node->charNodes[i]);
    }
    free(node->charVals);
    free(node->charNodes);

    sar_linkedListNode_t *curCallbackNode = node->callbackList->next;
    while (curCallbackNode != NULL)
    {
        sar_freeMatchCallbackStruct(curCallbackNode->val);
        sar_linkedListNode_t *nextCallback = curCallbackNode->next;

        // free struct
        sar_matchCallbackStruct_t *curCallback = curCallbackNode->val;
        curCallback->pointedCount--;
        if (curCallback->pointedCount == 0)
        {
            free(curCallbackNode->val);
        }

        free(curCallbackNode);
        curCallbackNode = nextCallback;
    }
    free(node->callbackList);
}

void sar_freeObject(sarObject_t *sarObject)
{
    sar_freeNode(sarObject->rootNode);
    free(sarObject->rootNode);
}

void sar_initObject(sarObject_t *obj)
{
    obj->continueFrom = -1;
    obj->inMatch = 0;

    obj->isGreedy = 1;
    obj->matchKeyCount = 0;
    obj->rootNode = malloc(sizeof(sarNode_t));
    sar_nodeInstruction_t *dummyInstruction = malloc(sizeof(sar_nodeInstruction_t));
    sar_initNodeInstruction(dummyInstruction, 0, SAR_NODE_SUFFIX_NONE, SAR_NODE_TYPE_CHAR, '\0');
    sar_initNode(obj->rootNode, dummyInstruction);
}

// general methods:

void sar_buildPath(sarObject_t *sarObject,
                   char *regexpStr,
                   int len,
                   sar_matchCallback callback,
                   sar_freeCallbackArgs freeCallback,
                   void *callbackArgs)
{
    sar_matchCallbackStruct_t *callbackStruct = malloc(sizeof(sar_matchCallbackStruct_t));
    sar_initMatchCallbackStruct(callbackStruct, callback, freeCallback, callbackArgs);
    sar_linkedListNode_t *complexInstructions = sar_buildComplexNodeInstructions(regexpStr, len);
    sar_linkedListNode_t *allInstructions = sar_convertConvertComplexInstructions(complexInstructions);

    // printf("---Printing all simpleRegexps:---\n");
    // sar_debug_printAllRegexps(allInstructions);
    // printf("---DONE---\n");

    sar_linkedListNode_t *curInstructionNode = allInstructions;
    while (curInstructionNode != NULL && curInstructionNode->val != NULL)
    {
        sar_linkedListNode_t *curRegexpInstructions = curInstructionNode->val;
        sar_buildPathRecursive(sarObject->rootNode, curRegexpInstructions, callbackStruct);
        curInstructionNode = curInstructionNode->next;
    }

    // free complex instructions
    sar_linkedListNode_t *curNodeComplexInstructionNode = complexInstructions;
    while (curNodeComplexInstructionNode != NULL)
    {
        sar_linkedListNode_t *nextPtr = curNodeComplexInstructionNode->next;
        sar_nodeComplexInstruction_t *curComplexVal = curNodeComplexInstructionNode->val;
        sar_freeNodeComplexInstruction(curComplexVal);
        free(curComplexVal);
        free(curNodeComplexInstructionNode);
        curNodeComplexInstructionNode = nextPtr;
    }

    // free used linked list
    sar_linkedListNode_t *curPtr = allInstructions;
    while (curPtr != NULL)
    {
        sar_linkedListNode_t *nextPtr = curPtr->next;
        sar_linkedListNode_t *curRegexpInstructions = curPtr->val;
        while (curRegexpInstructions != NULL)
        {
            sar_linkedListNode_t *nextInstPtr = curRegexpInstructions->next;
            free(curRegexpInstructions);
            curRegexpInstructions = nextInstPtr;
        }
        free(curPtr);
        curPtr = nextPtr;
    }
}

void sar_continueFrom(sarObject_t *sarObject, int from)
{
    sarObject->continueFrom = from;
}

void sar_stopMatch(sarObject_t *sarObject)
{
    sarObject->inMatch = 0;
}

void sar_matchFrom(sarObject_t *sarObject, char *matchStr, int from, int len)
{
    sarObject->inMatch = 1;
    sarObject->continueFrom = -1;

    int i = from;
    while (i < len)
    {
        sar_matchAtInternal(sarObject, matchStr, i, len);
        if (!sarObject->inMatch)
        {
            break;
        }

        if (sarObject->continueFrom >= 0)
        {
            i = sarObject->continueFrom;
            sarObject->continueFrom = -1;
        }
        else
        {
            i++;
        }
    }

    // TODO: check if we need this?
    sarObject->inMatch = 0;
}

void sar_matchAt(sarObject_t *sarObject, char *matchStr, int at, int len)
{
    sar_matchAtInternal(sarObject, matchStr, at, len);
}

// static methods:

static void sar_matchAtInternal(sarObject_t *sarObject, char *matchStr, int at, int len)
{
    unsigned int matchKey = sar_genMatchingKey(sarObject);
    sar_linkedListNode_t *activeNodes = malloc(sizeof(sar_linkedListNode_t));
    // Head is start of list
    activeNodes->val = NULL;
    activeNodes->next = malloc(sizeof(sar_linkedListNode_t));

    activeNodes->next->val = sarObject->rootNode;
    activeNodes->next->next = NULL;

    int i;
    for (i = at; i < len && activeNodes->next != NULL; i++)
    {
        sar_linkedListNode_t nextActiveNodes;
        nextActiveNodes.val = NULL;
        nextActiveNodes.next = NULL;
        sar_linkedListNode_t *lastNextActiveNodesNode = &nextActiveNodes;

        sar_linkedListNode_t *curPtr = activeNodes->next;
        while (curPtr != NULL)
        {
            sarNode_t *curNode = curPtr->val;
            sar_linkedListNode_t newActiveNodesHead;
            sar_linkedListNode_t *lastNode;
            newActiveNodesHead.next = NULL;
            newActiveNodesHead.val = NULL;
            int isInLoop = 0;
            int hasNextNodes = sar_getNextActiveNodes(curNode, matchStr[i], &newActiveNodesHead, &lastNode, &isInLoop, 0);
            if (hasNextNodes)
            {
                lastNextActiveNodesNode->next = newActiveNodesHead.next;
                lastNextActiveNodesNode = lastNode;
            }
            if (sarObject->isGreedy)
            {
                if (!isInLoop)
                {
                    sar_triggerCallbacks(curNode, at, i, matchKey);
                }
            }
            else
            {
                sar_triggerCallbacks(curNode, at, i, matchKey);
            }

            sar_linkedListNode_t *curPtrTemp = curPtr;
            curPtr = curPtr->next;

            // No longer needed
            free(curPtrTemp);
        }
        activeNodes->next = nextActiveNodes.next;
    }

    // in case of match at end of string
    sar_linkedListNode_t *curPtr = activeNodes->next;
    while (curPtr != NULL)
    {
        sarNode_t *curNode = curPtr->val;
        sar_triggerCallbacks(curNode, at, i, matchKey);
        sar_linkedListNode_t *curPtrTemp = curPtr;
        curPtr = curPtr->next;

        // No longer needed
        free(curPtrTemp);
    }
    free(activeNodes);
}

static int sar_getNextActiveNodes(sarNode_t *curNode, char curChar,
                                  sar_linkedListNode_t *newActiveNodesHead,
                                  sar_linkedListNode_t **lastNodePtr,
                                  int *isInLoop,
                                  int isInNegNodes)
{
    *isInLoop = 0;
    sar_linkedListNode_t *curPtr = newActiveNodesHead;
    if (curNode->negativeNode != NULL)
    {
        sar_linkedListNode_t *negativeLastActiveNodes;
        if (sar_getNextActiveNodes(curNode->negativeNode, curChar, curPtr, &negativeLastActiveNodes, isInLoop, 1))
        {
            curPtr = negativeLastActiveNodes;
        }
    }
    if (curNode->plusNode != NULL)
    {
        sar_linkedListNode_t *plusActiveNodes;
        // TODO: check if this can't cause for same node to be in next active nodes multiple times
        //       since it will recheck curNode->plusNode, and as we know,
        //       it has same instruction as current one
        if (sar_getNextActiveNodes(curNode->plusNode, curChar, curPtr, &plusActiveNodes, isInLoop, isInNegNodes))
        {
            curPtr = plusActiveNodes;
        }
    }
    if (curNode->alphaNode != NULL)
    {
        int isCharAlpha = isalpha(curChar) > 0;
        if (isCharAlpha != isInNegNodes)
        {
            curPtr->next = malloc(sizeof(sar_linkedListNode_t));
            curPtr->next->val = curNode->alphaNode;
            curPtr->next->next = NULL;
            curPtr = curPtr->next;
        }
    }
    if (curNode->alphaNumNodes != NULL)
    {
        int isCharAlNum = isalnum(curChar) > 0;
        if (isCharAlNum != isInNegNodes)
        {
            curPtr->next = malloc(sizeof(sar_linkedListNode_t));
            curPtr->next->val = curNode->alphaNumNodes;
            curPtr->next->next = NULL;
            curPtr = curPtr->next;
        }
    }
    if (curNode->spaceNodes != NULL)
    {
        // we do the following to normalize bigger than 0 val to 1
        int isCharSpace = isspace(curChar) > 0;
        if (isCharSpace != isInNegNodes)
        {
            curPtr->next = malloc(sizeof(sar_linkedListNode_t));
            curPtr->next->val = curNode->spaceNodes;
            curPtr->next->next = NULL;
            curPtr = curPtr->next;
        }
    }
    if (curNode->digitNode != NULL)
    {
        // we do the following to normalize bigger than 0 val to 1
        int isCharDigit = isdigit(curChar) > 0;
        if (isCharDigit != isInNegNodes)
        {
            curPtr->next = malloc(sizeof(sar_linkedListNode_t));
            curPtr->next->val = curNode->digitNode;
            curPtr->next->next = NULL;
            curPtr = curPtr->next;
        }
    }
    if (curNode->dotNode != NULL && !isInNegNodes)
    {
        curPtr->next = malloc(sizeof(sar_linkedListNode_t));
        curPtr->next->val = curNode->dotNode;
        curPtr->next->next = NULL;
        curPtr = curPtr->next;
    }

    if (isInNegNodes)
    {
        sar_linkedListNode_t *lastItem;
        sar_linkedListNode_t *negCharNodes = sar_findNegCharNode(curNode, curChar, &lastItem);
        if (negCharNodes != NULL)
        {
            curPtr->next = negCharNodes;
            curPtr = lastItem;
        }
    }
    else
    {
        sarNode_t *charNode;
        if (sar_findCharNode(curNode, curChar, &charNode))
        {
            curPtr->next = malloc(sizeof(sar_linkedListNode_t));
            curPtr->next->val = charNode;
            curPtr->next->next = NULL;
            curPtr = curPtr->next;
        }
    }

    if (curNode->nodeInstruction->suffix == SAR_NODE_SUFFIX_PLUS &&
        (sar_isNodeActivated(curNode, curChar) != isInNegNodes))
    {
        *isInLoop = 1;
        curPtr->next = malloc(sizeof(sar_linkedListNode_t));
        curPtr->next->val = curNode;
        curPtr->next->next = NULL;
        curPtr = curPtr->next;
    }

    // check if found any matches node
    if (curPtr == newActiveNodesHead)
    {
        *lastNodePtr = NULL;
        return 0;
    }
    else
    {
        *lastNodePtr = curPtr;
        return 1;
    }
}

// node instruction:

static void sar_initNodeInstruction(sar_nodeInstruction_t *nodeInstruction,
                                    int isNeg, sar_nodeSuffix_e suffix, sar_nodeType_e type, char charVal)
{
    nodeInstruction->isNeg = isNeg;
    nodeInstruction->suffix = suffix;
    nodeInstruction->type = type;
    nodeInstruction->charVal = charVal;
}

static void sar_initNodeComplexInstruction(sar_nodeComplexInstruction_t *nodeInstruction,
                                           int isNeg, sar_nodeComplexSuffix_e suffix, sar_nodeType_e type, char charVal)
{
    nodeInstruction->isNeg = isNeg;
    nodeInstruction->suffix = suffix;
    nodeInstruction->type = type;
    nodeInstruction->charVal = charVal;
}

static void sar_freeNodeInstruction(sar_nodeInstruction_t *nodeInstruction)
{
    // Nothing to free
}

static void sar_freeNodeComplexInstruction(sar_nodeComplexInstruction_t *complexNodeInstruction)
{
    // Nothing to free
}

static void sar_initMatchCallbackStruct(sar_matchCallbackStruct_t *callbackStruct,
                                        sar_matchCallback func,
                                        sar_freeCallbackArgs freeFunc,
                                        void *args)
{
    callbackStruct->args = args;
    callbackStruct->callback = func;
    callbackStruct->freeCallback = freeFunc;
    callbackStruct->pointedCount = 0;
    callbackStruct->lastTo = -1;
    callbackStruct->lastFrom = -1;
    callbackStruct->matchKey = 0;
}

static void sar_freeMatchCallbackStruct(sar_matchCallbackStruct_t *callbacStruct)
{
    callbacStruct->freeCallback(callbacStruct->args);
}

// general static methods:

static sar_linkedListNode_t *sar_convertConvertComplexInstructions(sar_linkedListNode_t *complexInstructions)
{
    // allSimpleRegexps linked list of linked list of sar_nodeInstruction_t.
    // NOTE: the inner linked list first value is always NULL!
    sar_linkedListNode_t *allSimpleRegexps = malloc(sizeof(sar_linkedListNode_t));
    allSimpleRegexps->val = malloc(sizeof(sar_linkedListNode_t));
    allSimpleRegexps->next = NULL;

    ((sar_linkedListNode_t *)allSimpleRegexps->val)->next = NULL;
    ((sar_linkedListNode_t *)allSimpleRegexps->val)->val = NULL;

    sar_linkedListNode_t *curNode = complexInstructions;
    while (curNode != NULL)
    {
        sar_nodeComplexInstruction_t *curComplexInst = curNode->val;
        sar_nodeInstruction_t *simpleInst = malloc(sizeof(sar_nodeInstruction_t));
        sar_initNodeInstruction(simpleInst, curComplexInst->isNeg,
                                SAR_NODE_SUFFIX_NONE, curComplexInst->type,
                                curComplexInst->charVal);

        sar_linkedListNode_t *withoutCurInstList = NULL;
        if (curComplexInst->suffix == SAR_NODE_COMPLEX_SUFFIX_PLUS)
        {
            simpleInst->suffix = SAR_NODE_SUFFIX_PLUS;
        }
        else if (curComplexInst->suffix == SAR_NODE_COMPLEX_SUFFIX_STAR)
        {
            simpleInst->suffix = SAR_NODE_SUFFIX_PLUS;
            withoutCurInstList = sar_cloneAllRegexpsInstructions(allSimpleRegexps);
        }
        else if (curComplexInst->suffix == SAR_NODE_COMPLEX_SUFFIX_QUESTION)
        {
            simpleInst->suffix = SAR_NODE_SUFFIX_NONE;
            withoutCurInstList = sar_cloneAllRegexpsInstructions(allSimpleRegexps);
        }
        else if (curComplexInst->suffix == SAR_NODE_COMPLEX_SUFFIX_NONE)
        {
            simpleInst->suffix = SAR_NODE_SUFFIX_NONE;
        }

        sar_linkedListNode_t *curRegexpListNode = allSimpleRegexps;
        while (curRegexpListNode != NULL)
        {
            sar_linkedListNode_t *curRegexpList = curRegexpListNode->val;

            sar_linkedListNode_t *newSimpleInstNode = malloc(sizeof(sar_linkedListNode_t));

            sar_nodeInstruction_t *curSimpleInstClone = malloc(sizeof(sar_nodeInstruction_t));
            *curSimpleInstClone = *simpleInst;
            newSimpleInstNode->next = NULL;
            newSimpleInstNode->val = curSimpleInstClone;

            sar_linkedListNode_t *lastNode = curRegexpList;
            while (lastNode->next != NULL)
            {
                lastNode = lastNode->next;
            }
            lastNode->next = newSimpleInstNode;

            if (withoutCurInstList != NULL)
            {
                withoutCurInstList = sar_removeFromRegexpsList(withoutCurInstList, curRegexpList);
            }

            // break so we'll have last node
            if (curRegexpListNode->next == NULL)
            {
                break;
            }
            else
            {
                curRegexpListNode = curRegexpListNode->next;
            }
        }

        sar_freeNodeInstruction(simpleInst);
        free(simpleInst);

        // append withoutCurInstList
        curRegexpListNode->next = withoutCurInstList;

        curNode = curNode->next;
    }

    // advance 1 in every regexp instructions list
    sar_linkedListNode_t *curRegexpInstNode = allSimpleRegexps;
    while (curRegexpInstNode != NULL)
    {
        sar_linkedListNode_t *regexpInsts = curRegexpInstNode->val;
        curRegexpInstNode->val = regexpInsts->next;
        free(regexpInsts);
        curRegexpInstNode = curRegexpInstNode->next;
    }

    return allSimpleRegexps;
}

static sar_linkedListNode_t *sar_removeFromRegexpsList(sar_linkedListNode_t *regexpsList, sar_linkedListNode_t *regexpInsts)
{
    // NOTE: we use head so there will be a previous node even for first item on the list
    sar_linkedListNode_t head;
    head.val = NULL;
    head.next = regexpsList;

    sar_linkedListNode_t *regexpsListPrevNode = &head;
    sar_linkedListNode_t *regexpsListCurNode = regexpsListPrevNode->next;
    while (regexpsListCurNode != NULL)
    {
        sar_linkedListNode_t *curRegexpInst = regexpsListCurNode->val;

        // NOTE: we get next item from list since by standard first item will be null
        if (sar_compareRegexpInstructions(curRegexpInst->next, regexpInsts->next))
        {
            regexpsListPrevNode->next = regexpsListCurNode->next;
            sar_freeRegexpInstructions(curRegexpInst);
            free(regexpsListCurNode);
            break;
        }

        regexpsListPrevNode = regexpsListCurNode;
        regexpsListCurNode = regexpsListCurNode->next;
    }
    return head.next;
}

static void sar_freeRegexpInstructions(sar_linkedListNode_t *regexpInstructions)
{
    sar_linkedListNode_t *curInstNode = regexpInstructions;
    while (curInstNode != NULL)
    {
        sar_nodeInstruction_t *curInst = curInstNode->val;
        sar_freeNodeInstruction(curInst);
        free(curInst);
        sar_linkedListNode_t *nextNode = curInstNode->next;
        free(curInstNode);
        curInstNode = nextNode;
    }
}

static int sar_compareRegexpInstructions(sar_linkedListNode_t *regexpInst1, sar_linkedListNode_t *regexpInst2)
{
    sar_linkedListNode_t *r1CurNode = regexpInst1;
    sar_linkedListNode_t *r2CurNode = regexpInst2;
    int retval = 1;
    while (r1CurNode != NULL && r2CurNode != NULL)
    {
        sar_nodeInstruction_t *curNodeInst1 = r1CurNode->val;
        sar_nodeInstruction_t *curNodeInst2 = r2CurNode->val;
        if (!sar_compareNodeInstruction(curNodeInst1, curNodeInst2))
        {
            retval = 0;
            break;
        }
        r1CurNode = r1CurNode->next;
        r2CurNode = r2CurNode->next;
    }

    // NULL == r1CurNode == r2CurNode
    // this means they dont have same length!
    if (!(r1CurNode == r2CurNode && r1CurNode == NULL))
    {
        retval = 0;
    }
    return retval;
}

static int sar_compareNodeInstruction(sar_nodeInstruction_t *node1, sar_nodeInstruction_t *node2)
{
    if (node1->charVal != node2->charVal)
    {
        return 0;
    }
    else if (node1->isNeg != node2->isNeg)
    {
        return 0;
    }
    else if (node1->suffix != node2->suffix)
    {
        return 0;
    }
    else if (node1->type != node2->type)
    {
        return 0;
    }
    return 1;
}

static sar_linkedListNode_t *sar_cloneAllRegexpsInstructions(sar_linkedListNode_t *originalRegexpsList)
{
    sar_linkedListNode_t *listHead = malloc(sizeof(sar_linkedListNode_t));
    listHead->val = NULL;
    listHead->next = NULL;

    sar_linkedListNode_t *lastNewListNode = listHead;
    sar_linkedListNode_t *curRegexpNode = originalRegexpsList;
    int i = 0;
    while (1)
    {
        sar_linkedListNode_t *curRegexpInsts = curRegexpNode->val;
        // NOTE: we take next node, since by definition, first item is null
        sar_linkedListNode_t *newList = malloc(sizeof(sar_linkedListNode_t));
        newList->val = NULL;
        newList->next = NULL;
        sar_linkedListNode_t *newListCurNode = newList;
        // TODO: validate curRegexpInsts cannot equal NULL
        while (1)
        {
            sar_nodeInstruction_t *curNode = curRegexpInsts->val;
            sar_nodeInstruction_t *curNodeClone = malloc(sizeof(sar_nodeInstruction_t));
            if (curNode == NULL)
            {
                free(curNodeClone);
                curNodeClone = NULL;
            }
            else
            {
                *curNodeClone = *curNode;
            }
            newListCurNode->val = curNodeClone;
            if (curRegexpInsts->next == NULL)
            {
                break;
            }
            curRegexpInsts = curRegexpInsts->next;
            newListCurNode->next = malloc(sizeof(sar_linkedListNode_t));
            newListCurNode->next->val = NULL;
            newListCurNode->next->next = NULL;
            newListCurNode = newListCurNode->next;
        }
        lastNewListNode->val = newList;
        curRegexpNode = curRegexpNode->next;
        if (curRegexpNode == NULL)
        {
            // TODO: I think this is not important, validate!
            lastNewListNode->next = NULL;
            break;
        }
        lastNewListNode->next = malloc(sizeof(sar_linkedListNode_t));
        lastNewListNode->next->val = NULL;
        lastNewListNode->next->next = NULL;
        lastNewListNode = lastNewListNode->next;
        i++;
    }

    return listHead;
}

static sar_linkedListNode_t *sar_buildComplexNodeInstructions(char *regexpStr, int len)
{
    int curStrPos = 0;
    sar_linkedListNode_t head;
    head.val = NULL;
    head.next = NULL;

    sar_linkedListNode_t *curNode = &head;
    while (curStrPos < len)
    {
        sar_nodeComplexInstruction_t *curInstruction = malloc(sizeof(sar_nodeComplexInstruction_t));
        sar_initNodeComplexInstruction(curInstruction, 0, SAR_NODE_COMPLEX_SUFFIX_NONE, SAR_NODE_TYPE_CHAR, '\0');
        curStrPos = sar_buildCurNodeInstruction(curInstruction, regexpStr, curStrPos, len, 0, 1);
        if (curStrPos == -1)
        {
            sar_freeNodeComplexInstruction(curInstruction);
            break;
        }

        sar_linkedListNode_t *nextNode = malloc(sizeof(sar_linkedListNode_t));
        nextNode->next = NULL;
        nextNode->val = curInstruction;

        curNode->next = nextNode;
        curNode = nextNode;
    }
    return head.next;
}

static int sar_buildCurNodeInstruction(sar_nodeComplexInstruction_t *nodeInstruction, char *regexpStr,
                                       int pos, int len, int expectSuffix, int expectPrefix)
{
    if (pos == len)
    {
        return pos;
    }
    char curChar = regexpStr[pos];

    if (expectSuffix)
    {
        if (sar_isCharSuffix(curChar))
        {
            sar_setCharSuffix(nodeInstruction, curChar);
            return pos + 1;
        }
        else
        {
            nodeInstruction->suffix = SAR_NODE_SUFFIX_NONE;
        }
        return pos;
    }

    if (curChar == '\\' && expectPrefix)
    {
        if (len <= pos + 1)
        {
            return -1;
        }

        char nextChar = regexpStr[pos + 1];
        if (nextChar == '^')
        {
            nodeInstruction->isNeg = !nodeInstruction->isNeg;
            return sar_buildCurNodeInstruction(nodeInstruction, regexpStr, pos + 2, len, 0, 1);
        }
        else
        {
            sar_setBackslashChar(nodeInstruction, nextChar);
            return sar_buildCurNodeInstruction(nodeInstruction, regexpStr, pos + 2, len, 1, 0);
        }
    }

    if (curChar == '.')
    {
        nodeInstruction->type = SAR_NODE_TYPE_DOT;
    }
    else
    {
        nodeInstruction->type = SAR_NODE_TYPE_CHAR;
        nodeInstruction->charVal = curChar;
    }
    return sar_buildCurNodeInstruction(nodeInstruction, regexpStr, pos + 1, len, 1, 0);
}

static void sar_setBackslashChar(sar_nodeComplexInstruction_t *nodeInstruction, char charVal)
{
    if (charVal == 'w')
    {
        nodeInstruction->type = SAR_NODE_TYPE_ALPHA_NUMERIC;
    }
    else if (charVal == 'a')
    {
        nodeInstruction->type = SAR_NODE_TYPE_ALPHA;
    }
    else if (charVal == 'd')
    {
        nodeInstruction->type = SAR_NODE_TYPE_DIGIT;
    }
    else if (charVal == 's')
    {
        nodeInstruction->type = SAR_NODE_TYPE_SPACE;
    }
    else
    {
        nodeInstruction->type = SAR_NODE_TYPE_CHAR;
        nodeInstruction->charVal = charVal;
    }
}

static void sar_setCharSuffix(sar_nodeComplexInstruction_t *nodeInstruction, char charVal)
{
    sar_nodeComplexSuffix_e chosenSuffix = SAR_NODE_COMPLEX_SUFFIX_NONE;
    if (charVal == '+')
    {
        chosenSuffix = SAR_NODE_COMPLEX_SUFFIX_PLUS;
    }
    else if (charVal == '?')
    {
        chosenSuffix = SAR_NODE_COMPLEX_SUFFIX_QUESTION;
    }
    else if (charVal == '*')
    {
        chosenSuffix = SAR_NODE_COMPLEX_SUFFIX_STAR;
    }
    else
    {
        // TODO: throw error!
        //       should never get here!
    }
    nodeInstruction->suffix = chosenSuffix;
}

static int sar_isCharSuffix(char charVal)
{
    char *suffixChars = "+*?";
    char *curChar = suffixChars;
    while (*curChar != '\0')
    {
        if (charVal == *curChar)
        {
            return 1;
        }
        curChar++;
    }
    return 0;
}

static void sar_buildPathRecursive(sarNode_t *node, sar_linkedListNode_t *nodeInstructionList,
                                   sar_matchCallbackStruct_t *callbackStruct)
{
    sarNode_t *nextNode;
    sar_nodeInstruction_t *curNodeInstruction = nodeInstructionList->val;
    if (sar_findNode(curNodeInstruction, node, &nextNode))
    {
        free(curNodeInstruction);
    }
    else
    {
        nextNode = malloc(sizeof(sarNode_t));
        sar_initNode(nextNode, curNodeInstruction);
        sar_insertNode(node, nextNode, 0, 0);
    }

    if (nodeInstructionList->next == NULL)
    {
        sar_appendCallback(nextNode, callbackStruct);
    }
    else
    {
        sar_buildPathRecursive(nextNode, nodeInstructionList->next, callbackStruct);
    }
}

static void sar_triggerCallbacks(sarNode_t *node, int from, int to, unsigned int matchKey)
{
    sar_linkedListNode_t *curCallbackNode;
    for (curCallbackNode = node->callbackList->next; curCallbackNode != NULL; curCallbackNode = curCallbackNode->next)
    {
        sar_matchCallbackStruct_t *callbackStruct = curCallbackNode->val;
        // Already called it in here!
        if (callbackStruct->lastFrom == from && callbackStruct->lastTo == to && callbackStruct->matchKey == matchKey)
        {
            continue;
        }
        callbackStruct->lastFrom = from;
        callbackStruct->lastTo = to;
        callbackStruct->matchKey = matchKey;
        callbackStruct->callback(from, to, callbackStruct->args);
    }
}

static void sar_appendCallback(sarNode_t *node, sar_matchCallbackStruct_t *callbackStruct)
{
    sar_linkedListNode_t *curCallbackNode;

    // get last item
    for (curCallbackNode = node->callbackList; curCallbackNode->next != NULL; curCallbackNode = curCallbackNode->next)
    {
    }

    sar_linkedListNode_t *newCallbackNode = malloc(sizeof(sar_linkedListNode_t));
    newCallbackNode->next = NULL;
    newCallbackNode->val = callbackStruct;

    curCallbackNode->next = newCallbackNode;
    callbackStruct->pointedCount++;
}

static void sar_insertNode(sarNode_t *intoNode, sarNode_t *node, int isNegNode, int isPlusNode)
{
    sar_nodeInstruction_t *insertInstruction = node->nodeInstruction;

    if (!isNegNode && insertInstruction->isNeg)
    {
        if (intoNode->negativeNode == NULL)
        {
            intoNode->negativeNode = malloc(sizeof(sarNode_t));
            sar_nodeInstruction_t *negativeNodeInstruction = malloc(sizeof(sar_nodeInstruction_t));
            *negativeNodeInstruction = *(intoNode->nodeInstruction);
            negativeNodeInstruction->isNeg = 1;
            sar_initNode(intoNode->negativeNode, negativeNodeInstruction);
        }
        sar_insertNode(intoNode->negativeNode, node, 1, isPlusNode);
    }
    else if (!isPlusNode && insertInstruction->suffix == SAR_NODE_SUFFIX_PLUS)
    {
        if (intoNode->plusNode == NULL)
        {
            intoNode->plusNode = malloc(sizeof(sarNode_t));
            sar_nodeInstruction_t *plusNodeInstruction = malloc(sizeof(sar_nodeInstruction_t));

            // TODO: check if the following line is neccecery
            *plusNodeInstruction = *(intoNode->nodeInstruction);
            sar_initNode(intoNode->plusNode, plusNodeInstruction);
        }
        sar_insertNode(intoNode->plusNode, node, isNegNode, 1);
    }
    else if (insertInstruction->type == SAR_NODE_TYPE_CHAR)
    {
        sar_insertCharNode(intoNode, node);
    }
    else if (insertInstruction->type == SAR_NODE_TYPE_ALPHA)
    {
        intoNode->alphaNode = node;
    }
    else if (insertInstruction->type == SAR_NODE_TYPE_ALPHA_NUMERIC)
    {
        intoNode->alphaNumNodes = node;
    }
    else if (insertInstruction->type == SAR_NODE_TYPE_SPACE)
    {
        intoNode->spaceNodes = node;
    }
    else if (insertInstruction->type == SAR_NODE_TYPE_DIGIT)
    {
        intoNode->digitNode = node;
    }
    else if (insertInstruction->type == SAR_NODE_TYPE_DOT)
    {
        intoNode->dotNode = node;
    }
}

static void sar_insertCharNode(sarNode_t *intoNode, sarNode_t *node)
{
    char insertCharVal = node->nodeInstruction->charVal;
    if (intoNode->charNodes == NULL)
    {
        intoNode->charNodes = malloc(sizeof(sarNode_t *));
        intoNode->charNodes[0] = node;
        intoNode->charVals = malloc(sizeof(char));
        intoNode->charVals[0] = insertCharVal;
        intoNode->charNodesSize = 1;
    }
    else
    {
        intoNode->charNodes = realloc(intoNode->charNodes, (intoNode->charNodesSize + 1) * sizeof(sarNode_t *));
        intoNode->charVals = realloc(intoNode->charVals, (intoNode->charNodesSize + 1) * sizeof(char));
        intoNode->charVals[intoNode->charNodesSize] = (char)0x00;
        // TODO: use binary search. not critical
        int inShiftMode = 0;
        sarNode_t *toShiftNode;
        char toShiftVal;
        intoNode->charNodesSize++;

        int i;
        for (i = 0; i < intoNode->charNodesSize; i++)
        {
            char curCharVal = intoNode->charVals[i];
            if (!inShiftMode)
            {
                if (insertCharVal <= curCharVal)
                {
                    inShiftMode = 1;

                    toShiftNode = intoNode->charNodes[i];
                    toShiftVal = intoNode->charVals[i];

                    intoNode->charNodes[i] = node;
                    intoNode->charVals[i] = insertCharVal;
                }
            }
            else
            {
                sarNode_t *nextShiftNode = intoNode->charNodes[i];
                int nextShiftVal = intoNode->charVals[i];
                intoNode->charNodes[i] = toShiftNode;
                intoNode->charVals[i] = toShiftVal;
                toShiftNode = nextShiftNode;
                toShiftVal = nextShiftVal;
            }
        }
        if (!inShiftMode)
        {
            // it is largest val, insert to last node
            intoNode->charNodes[intoNode->charNodesSize - 1] = node;
            intoNode->charVals[intoNode->charNodesSize - 1] = insertCharVal;
        }
    }
}

static int sar_findNode(sar_nodeInstruction_t *nodeInstruction, sarNode_t *node, sarNode_t **resultNode)
{
    int retval = 0;
    *resultNode = NULL;
    // TODO: check if we need to get the 'node->negativeNode != NULL' into inner if condition
    //       as done with next cond running on SAR_NODE_SUFFIX_PLUS
    if (nodeInstruction->isNeg && node->negativeNode != NULL)
    {
        sar_nodeInstruction_t nonNegInst = *nodeInstruction;
        nonNegInst.isNeg = 0;
        retval = sar_findNode(&nonNegInst, node->negativeNode, resultNode);
    }
    else if (nodeInstruction->suffix == SAR_NODE_SUFFIX_PLUS)
    {
        // NOTE: we do the inner and not together to avoid entering other nodes
        if (node->plusNode != NULL)
        {
            sar_nodeInstruction_t plusInst = *nodeInstruction;
            plusInst.suffix = SAR_NODE_SUFFIX_NONE;
            retval = sar_findNode(&plusInst, node->plusNode, resultNode);
        }
    }
    else if (nodeInstruction->type == SAR_NODE_TYPE_CHAR)
    {
        retval = sar_findCharNode(node, nodeInstruction->charVal, resultNode);
    }
    else if (nodeInstruction->type == SAR_NODE_TYPE_ALPHA && node->alphaNode != NULL)
    {
        retval = 1;
        *resultNode = node->alphaNode;
    }
    else if (nodeInstruction->type == SAR_NODE_TYPE_ALPHA_NUMERIC && node->alphaNumNodes != NULL)
    {
        retval = 1;
        *resultNode = node->alphaNumNodes;
    }
    else if (nodeInstruction->type == SAR_NODE_TYPE_SPACE && node->spaceNodes != NULL)
    {
        retval = 1;
        *resultNode = node->spaceNodes;
    }
    else if (nodeInstruction->type == SAR_NODE_TYPE_DIGIT && node->digitNode != NULL)
    {
        retval = 1;
        *resultNode = node->digitNode;
    }
    else if (nodeInstruction->type == SAR_NODE_TYPE_DOT && node->dotNode != NULL)
    {
        retval = 1;
        *resultNode = node->dotNode;
    }
    return retval;
}

static int sar_findCharNode(sarNode_t *node, char charVal, sarNode_t **resultNode)
{
    *resultNode = NULL;
    int retval = 0;

    if (!node->charNodesSize)
    {
        return retval;
    }

    int low = 0;
    int high = node->charNodesSize - 1;
    char *allChars = node->charVals;

    while (low <= high)
    {
        int mid = (low + high) / 2;
        char curVal = allChars[mid];
        if (curVal < charVal)
        {
            low = mid + 1;
        }
        else if (curVal > charVal)
        {
            high = mid - 1;
        }
        else if (curVal == charVal)
        {
            *resultNode = node->charNodes[mid];
            retval = 1;
            break;
        }
    }
    return retval;
}

static sar_linkedListNode_t *sar_findNegCharNode(sarNode_t *node, char charVal, sar_linkedListNode_t **lastNodePtr)
{
    *lastNodePtr = NULL;

    // only a placeholder, its value has no meaning
    sar_linkedListNode_t resultNodes;
    resultNodes.next = NULL;

    if (!node->nodeInstruction->isNeg)
    {
        return NULL;
    }

    int i;
    sar_linkedListNode_t *lastNode = &resultNodes;
    for (i = 0; i < node->charNodesSize; i++)
    {
        if (node->charVals[i] != charVal)
        {
            sar_linkedListNode_t *nextNode = malloc(sizeof(sar_linkedListNode_t));
            nextNode->next = NULL;
            nextNode->val = node->charNodes[i];

            lastNode->next = nextNode;
            lastNode = nextNode;
        }
    }
    *lastNodePtr = lastNode;
    return resultNodes.next;
}

static unsigned int sar_genMatchingKey(sarObject_t *sarObject)
{
    return sarObject->matchKeyCount++;
}

static int sar_isNodeActivated(sarNode_t *node, char c)
{
    sar_nodeInstruction_t *inst = node->nodeInstruction;
    int isActive = 0;

    if (inst->type == SAR_NODE_TYPE_ALPHA)
    {
        isActive = isalpha(c) > 0;
    }
    else if (inst->type == SAR_NODE_TYPE_ALPHA_NUMERIC)
    {
        isActive = isalnum(c) > 0;
    }
    else if (inst->type == SAR_NODE_TYPE_SPACE)
    {
        isActive = isspace(c) > 0;
    }
    else if (inst->type == SAR_NODE_TYPE_DIGIT)
    {
        isActive = isdigit(c) > 0;
    }
    else if (inst->type == SAR_NODE_TYPE_DOT)
    {
        isActive = 1;
    }
    else if (inst->type == SAR_NODE_TYPE_CHAR)
    {
        isActive = inst->charVal == c;
    }
    else
    {
        // TODO: alert error, impossible
        return 0;
    }

    return isActive != inst->isNeg;
}

#ifdef SAR_DEBUG_PRINT
static void sar_debug_printAllRegexps(sar_linkedListNode_t *instructions)
{
    sar_linkedListNode_t *curInstNode = instructions;
    while (curInstNode != NULL)
    {
        sar_linkedListNode_t *curRegexpInst = curInstNode->val;
        sar_debug_printRegexp(curRegexpInst);
        curInstNode = curInstNode->next;
    }
}

static void sar_debug_printRegexp(sar_linkedListNode_t *regexpInstructions)
{
    sar_linkedListNode_t *curInstNode = regexpInstructions;
    while (curInstNode != NULL)
    {
        sar_nodeInstruction_t *curInst = curInstNode->val;
        sar_debug_printNodeInstruction(curInst);
        curInstNode = curInstNode->next;
    }
    printf("\n");
}

static void sar_debug_printNodeInstruction(sar_nodeInstruction_t *inst)
{
    if (inst->isNeg)
    {
        printf("\\^");
    }

    if (inst->type == SAR_NODE_TYPE_CHAR)
    {
        printf("%c", inst->charVal);
    }
    else if (inst->type == SAR_NODE_TYPE_DOT)
    {
        printf(".");
    }
    else
    {
        printf("\\");
        char charVal = '\0';
        if (inst->type == SAR_NODE_TYPE_ALPHA_NUMERIC)
        {
            charVal = 'w';
        }
        else if (inst->type == SAR_NODE_TYPE_ALPHA)
        {
            charVal = 'a';
        }
        else if (inst->type == SAR_NODE_TYPE_DIGIT)
        {
            charVal = 'd';
        }
        else if (inst->type == SAR_NODE_TYPE_SPACE)
        {
            charVal = 's';
        }
        printf("%c", charVal);
    }

    if (inst->suffix == SAR_NODE_SUFFIX_PLUS)
    {
        printf("+");
    }
}
#endif
