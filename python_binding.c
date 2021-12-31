#include <Python.h>
#include "multitiersimulator.c"

static PyObject *method_read_simulator(PyObject *self, PyObject *args){
    

    // char *tier_1_size, *tier_2_size, *tier_1_algo, *tier_2_algo, *output_file;
    char *input_file;
    
    if (!PyArg_ParseTuple(args, "s", &input_file)){
        return NULL;
    }

    read_file(input_file);
    return PyLong_FromLong(10000);

}

static PyObject *method_read_write_stats(PyObject *self, PyObject *args){

    char *input_filename;
    int object_id, op_field;

    if(!PyArg_ParseTuple(args, "sii", &input_filename, &object_id, &op_field)){
        return NULL;
    }
    
    opTypeStats values = trace_raw_stats(input_filename, object_id, op_field);

    PyObject * python_val = Py_BuildValue("(ii)", values.read_count, values.write_count);
    return python_val;
}


static PyObject *method_exclusive_cache(PyObject *self, PyObject *args){

    char *input_filename;
    uint64_t tier_1_size, tier_2_size, obj_id, op;
    char * op_filename;
    ///char *filename, uint64_t tier_1_size, uint64_t tier_2_size, uint64_t obj_id, uint64_t op, char* op_fname
    if(!PyArg_ParseTuple(args, "siiiis", &input_filename, &tier_1_size, &tier_2_size, &obj_id, &op, &op_filename)){
        return NULL;
    }

    traceStats exclusive_stats = exclusive_caching(input_filename, tier_1_size, tier_2_size, obj_id, op);

    bool write_value = write_stat_to_file(op_filename, exclusive_stats);

    char *string1 = "Writing output to file: ";
    char *string3;

    if(write_value){
        string3 = malloc(strlen(string1) + strlen(op_filename) + 1);
        sprintf(string3, "%s %s", string1, op_filename);    
        return PyUnicode_FromString(string3);
    }else{
        char *return_string = "Failed writing to file.";
        PyObject *return_ = Py_BuildValue("s", return_string); 
        return return_;
    }
    // sprintf("%s %s", string1, op_filename);

}

static PyMethodDef BindSimulatorMethod[] = {
    {"read_file", method_read_simulator, METH_VARARGS, "Python interface for reading the file."},
    {"read_write_stats", method_read_write_stats, METH_VARARGS, "Python interface for getting read, write stats."},
    {"exclusive_cache", method_exclusive_cache, METH_VARARGS, "Python interface for exclusive cache."},
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

// int main(int argc, char* argv[]){

//     if(argc != 6){
//         printf("Usage: ./new.o <file_name> <tier_1_size> <tier_2_size> <output_file> <incl_excl(0,1)>\n");
//     }
//     uint64_t tier_1_size = atoi(argv[2]);
//     uint64_t tier_2_size = atoi(argv[3]);
//     uint64_t incl_excl = atoi(argv[5]);

//     // read_file(argv[1]);
//     opTypeStats values = trace_raw_stats(argv[1], 2, 3);
//     printf("Read Count is: %d\n", values.read_count);
//     printf("Write Count is: %d\n", values.write_count);
//     // if(incl_excl == 0){
//     //     inclusive_caching(argv[1], tier_1_size, tier_2_size, argv[4]);
//     // }else{
//     //     exclusive_caching(argv[1], tier_1_size, tier_2_size, argv[4]);
//     // }
// }