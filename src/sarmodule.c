
#define Py_LIMITED_API

#include <Python.h>
#include "sarcore.h"

static void sarmod_callback(int from, int to, void *args);
static void sarmod_freeCallback(void *args);

static void sarmod_destroyObject(PyObject *sarObject);
static PyObject *sarmod_createObject(PyObject *self, PyObject *args);

static PyObject *sarmod_addRegexp(PyObject *self, PyObject *args);
static PyObject *sarmod_matchFrom(PyObject *self, PyObject *args);
static PyObject *sarmod_matchAt(PyObject *self, PyObject *args);
static PyObject *sarmod_stopMatch(PyObject *self, PyObject *args);
static PyObject *sarmod_continueFrom(PyObject *self, PyObject *args);

static PyMethodDef SarmodMethods[] = {
    {"create_object", sarmod_createObject, METH_VARARGS, "Creates a SAR object"},
    {"add_regexp", sarmod_addRegexp, METH_VARARGS, "Adds a regexp"},
    {"match_from", sarmod_matchFrom, METH_VARARGS, "Starts a match from a certain position"},
    {"match_at", sarmod_matchAt, METH_VARARGS, "Starts a match only from specific position"},
    {"continue_from", sarmod_continueFrom, METH_VARARGS, "continues match from specific position"},
    {"stop_match", sarmod_stopMatch, METH_VARARGS, "stops match entirely"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef sarmodule = {
    PyModuleDef_HEAD_INIT,
    "_regexp_sar_c",
    "SAR Regexp Engine for Python",
    -1,
    SarmodMethods};

PyMODINIT_FUNC PyInit__regexp_sar_c(void)
{
    return PyModule_Create(&sarmodule);
}

// Python interface:

static void sarmod_destroyObject(PyObject *sarObject)
{
    sarObject_t *sarObj = PyCapsule_GetPointer(sarObject, NULL);
    sar_freeObject(sarObj);
    free(sarObj);
}

static PyObject *sarmod_createObject(PyObject *self, PyObject *args)
{
    sarObject_t *newObject = malloc(sizeof(sarObject_t));
    sar_initObject(newObject);
    return PyCapsule_New(newObject, NULL, sarmod_destroyObject);
}

static PyObject *sarmod_addRegexp(PyObject *self, PyObject *args)
{
    PyObject *sarObjectCapsule;
    char *regexpStr;
    int strLen;
    PyObject *regexpCallback;

    if (!PyArg_ParseTuple(args, "Os#O", &sarObjectCapsule, &regexpStr, &strLen, &regexpCallback))
    {
        return NULL;
    }

    sarObject_t *sarObject = PyCapsule_GetPointer(sarObjectCapsule, NULL);
    Py_XINCREF(regexpCallback);

    sar_buildPath(sarObject, regexpStr, strLen, sarmod_callback, sarmod_freeCallback, regexpCallback);
    Py_RETURN_NONE;
}

static PyObject *sarmod_matchFrom(PyObject *self, PyObject *args)
{
    PyObject *sarObjectCapsule;
    char *matchStr;
    int strLen;
    int matchFromIdx;
    if (!PyArg_ParseTuple(args, "Os#i", &sarObjectCapsule, &matchStr, &strLen, &matchFromIdx))
    {
        return NULL;
    }

    sarObject_t *sarObject = PyCapsule_GetPointer(sarObjectCapsule, NULL);
    sar_matchFrom(sarObject, matchStr, matchFromIdx, strLen);
    Py_RETURN_NONE;
}

static PyObject *sarmod_matchAt(PyObject *self, PyObject *args)
{
    PyObject *sarObjectCapsule;
    char *matchStr;
    int strLen;
    int matchAtIdx;

    if (!PyArg_ParseTuple(args, "Os#i", &sarObjectCapsule, &matchStr, &strLen, &matchAtIdx))
    {
        return NULL;
    }

    sarObject_t *sarObject = PyCapsule_GetPointer(sarObjectCapsule, NULL);
    sar_matchAt(sarObject, matchStr, matchAtIdx, strLen);
    Py_RETURN_NONE;
}

static PyObject *sarmod_continueFrom(PyObject *self, PyObject *args)
{
    PyObject *sarObjectCapsule;
    int continueFromIdx;

    if (!PyArg_ParseTuple(args, "Oi", &sarObjectCapsule, &continueFromIdx))
    {
        return NULL;
    }

    sarObject_t *sarObject = PyCapsule_GetPointer(sarObjectCapsule, NULL);
    sar_continueFrom(sarObject, continueFromIdx);
    Py_RETURN_NONE;
}

static PyObject *sarmod_stopMatch(PyObject *self, PyObject *args)
{
    PyObject *sarObjectCapsule;

    if (!PyArg_ParseTuple(args, "O", &sarObjectCapsule))
    {
        return NULL;
    }

    sarObject_t *sarObject = PyCapsule_GetPointer(sarObjectCapsule, NULL);
    sar_stopMatch(sarObject);
    Py_RETURN_NONE;
}

static void sarmod_callback(int from, int to, void *args)
{
    PyObject *py_lambdaObj = (PyObject *)args;
    PyObject *arglist;
    PyObject *result;

    arglist = Py_BuildValue("ii", from, to);
    result = PyObject_CallObject(py_lambdaObj, arglist);
    Py_XDECREF(arglist);
    Py_XDECREF(result);
}

static void sarmod_freeCallback(void *args)
{
    PyObject *py_lambdaObj = (PyObject *)args;
    Py_XDECREF(py_lambdaObj);
}
