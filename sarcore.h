#ifndef REGEXP_SAR_H
#define REGEXP_SAR_H

typedef void (*sar_matchCallback)(int from, int to, void *args);
typedef void (*sar_freeCallbackArgs)(void *args);

typedef enum sar_nodeType_e
{
    SAR_NODE_TYPE_ALPHA,
    SAR_NODE_TYPE_ALPHA_NUMERIC,
    SAR_NODE_TYPE_SPACE,
    SAR_NODE_TYPE_DIGIT,
    SAR_NODE_TYPE_DOT,
    SAR_NODE_TYPE_CHAR,
} sar_nodeType_e;

typedef enum sar_nodeSuffix_e
{
    SAR_NODE_SUFFIX_NONE,
    SAR_NODE_SUFFIX_PLUS,
} sar_nodeSuffix_e;

typedef enum sar_nodeComplexSuffix_e
{
    SAR_NODE_COMPLEX_SUFFIX_NONE,
    SAR_NODE_COMPLEX_SUFFIX_PLUS,
    SAR_NODE_COMPLEX_SUFFIX_QUESTION,
    SAR_NODE_COMPLEX_SUFFIX_STAR,
} sar_nodeComplexSuffix_e;

typedef struct
{
    int isNeg;
    sar_nodeSuffix_e suffix;
    sar_nodeType_e type;
    char charVal;
} sar_nodeInstruction_t;

typedef struct
{
    int isNeg;
    sar_nodeComplexSuffix_e suffix;
    sar_nodeType_e type;
    char charVal;
} sar_nodeComplexInstruction_t;

typedef struct sar_linkedListNode_t
{
    void *val;
    struct sar_linkedListNode_t *next;
} sar_linkedListNode_t;

typedef struct sarNode_t
{
    // NOTE: this is not a pointer for performance issues
    sar_nodeInstruction_t *nodeInstruction;

    struct sarNode_t *alphaNode;
    struct sarNode_t *alphaNumNodes;
    struct sarNode_t *spaceNodes;
    struct sarNode_t *digitNode;
    struct sarNode_t *dotNode;

    struct sarNode_t *negativeNode;
    struct sarNode_t *plusNode;

    struct sarNode_t **charNodes;
    char *charVals;

    int charNodesSize;
    sar_linkedListNode_t *callbackList;
} sarNode_t;

typedef struct sarObject_t
{
    // matching info
    int continueFrom;
    int inMatch;

    int isGreedy;
    // TODO: some bugs might occur although very unlikely, will need matchKeyCount to circle entire time,
    //       ignore for now...
    unsigned int matchKeyCount;

    sarNode_t *rootNode;
} sarObject_t;

void sar_freeObject(sarObject_t *sarObject);
void sar_initObject(sarObject_t *obj);

void sar_buildPath(sarObject_t *sarObject,
                   char *regexpStr,
                   int len,
                   sar_matchCallback callback,
                   sar_freeCallbackArgs freeCallback,
                   void *callbackArgs);

void sar_matchAt(sarObject_t *sarObject, char *matchStr, int at, int len);
void sar_matchFrom(sarObject_t *sarObject, char *matchStr, int from, int len);

void sar_stopMatch(sarObject_t *sarObject);
void sar_continueFrom(sarObject_t *sarObject, int from);

#endif
