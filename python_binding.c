#include <python3.8/Python.h>
#include "multitiersimulator.c"

static PyObject *method_bind_simulator(PyObject *self, PyObject *args){
    

    char *tier_1_size, *tier_2_size, *tier_1_algo, *tier_2_algo, *output_file;
    char *input_file;
    
    if (!PyArg_ParseTuple(args, "ssssss", &input_file, &tier_1_size, &tier_1_algo, &tier_2_size, &tier_2_algo, &output_file)){
        return NULL;
    }

    printf("Tier 1 Size: %s\n", tier_1_size);
    printf("Tier 1 Algo: %s\n", tier_1_algo);
    printf("Tier 2 Size: %s\n", tier_2_size);
    printf("Tier 2 Algo: %s\n", tier_2_algo);

    read_file(input_file);
    return PyLong_FromLong(10000);

}


static PyMethodDef BindSimulatorMethod[] = {
    {"simulator", method_bind_simulator, METH_VARARGS, "Python interface for multi tier simulator library"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef bindsimulatormodule =
{
    PyModuleDef_HEAD_INIT,
    "simulator",
    "Python interface for multi tier simulator library",
    -1,
    BindSimulatorMethod
};

PyMODINIT_FUNC PyInit_simulator(void){
    return PyModule_Create(&bindsimulatormodule);
}
