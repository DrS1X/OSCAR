//
// Created by Administrator on 2022/12/4.
//


#include "hdf5Opt.h"

using std::string;
using std::cout;
using std::endl;

bool hdf5Opt::read(std::string fileName){
    using namespace H5;

    H5std_string FILE_NAME( "test.h5");
    H5std_string DATASET_NAME( "precipitation" );
    const int    NX = 1800;		// output buffer dimensions
    const int    NY = 3600;
    const int    RANK_OUT = 3;

    int i, j;
    float(* data_out)[NY] = new float[NX][NY]; /// output buffer ///
    for (j = 0; j < NX; j++)
    {
    for (i = 0; i < NY; i++)
    {
    data_out[j][i] = 0;
    }
    }
    try {
    
     // Turn off the auto-printing when failure occurs so that we can
     // handle the errors appropriately
     
//    Exception::dontPrint();

    
     // Open the specified file-operator and the specified dataset in the file-operator.
     
    H5File file(FILE_NAME, H5F_ACC_RDONLY);
    DataSet dataset = file.openDataSet(DATASET_NAME);

    
     // Get the class of the datatype that is used by the dataset.
     
    H5T_class_t type_class = dataset.getTypeClass();

    assert(type_class == H5T_FLOAT); //IEEE float32 little endian

    
     // Get dataspace of the dataset.
     
    DataSpace dataspace = dataset.getSpace();

    
     // Get the number of dimensions in the dataspace.
     
    int rank = dataspace.getSimpleExtentNdims();

    
     // Get the dimension size of each dimension in the dataspace and
     // display them.
     
    hsize_t dims_out[2];
    int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
    cout << "rank " << rank << ", dimensions " <<
    (unsigned long) (dims_out[0]) << " x " <<
    (unsigned long) (dims_out[1]) << endl;


    
     // Read data from hyperslab in the file-operator into the hyperslab in
     // memory and display the data.
     
    dataset.read(data_out, PredType::IEEE_F32LE);

    for (j = 0; j < NX; j++) {
    for (i = 0; i < NY; i++)
    cout << data_out[j][i] << " ";
    cout << endl;
    }
    }
    // catch failure caused by the H5File operations
    catch( FileIException error )
    {
    error.printErrorStack();
    return false;
    }

    // catch failure caused by the DataSet operations
    catch( DataSetIException error )
    {
    error.printErrorStack();
    return false;
    }

    // catch failure caused by the DataSpace operations
    catch( DataSpaceIException error )
    {
    error.printErrorStack();
    return false;
    }

    // catch failure caused by the DataSpace operations
    catch( DataTypeIException error )
    {
    error.printErrorStack();
    return false;
    }

    return true; // successfully terminated
}