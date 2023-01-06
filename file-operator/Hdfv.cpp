//
// Created by Administrator on 2022/12/4.
//


#include "Hdfv.h"

using std::string;
using std::cout;
using std::endl;
using namespace H5;

Hdfv::Hdfv(string fileName, string _groupName, string _datasetName):
    Rst(fileName), groupName(_groupName), datasetName(_datasetName){}


Hdfv::Hdfv(string fileName): Rst(fileName){}

bool Hdfv::read(string fName) {
    try {
         // Turn off the auto-printing when failure occurs so that we can
         // handle the errors appropriately
         Exception::dontPrint();

         // Open the specified file-operator and the specified dataset in the file-operator.
        H5File h5File(name.c_str(), H5F_ACC_RDONLY);
        Group group = h5File.openGroup(groupName.c_str());
        DataSet dataset = group.openDataSet(datasetName.c_str());

        // Get the class of the datatype that is used by the dataset.
        dataType = dataset.getDataType();

        // Get dataspace of the dataset.
        DataSpace dataspace = dataset.getSpace();

        // Get the dimension size of each dimension in the dataspace and
        hsize_t dims_out[3];
        dataspace.getSimpleExtentDims(dims_out);
        /*cout << "[getMeta] dimensions: " <<
             " x " << (unsigned long) (dims_out[0]) <<
             " y " << (unsigned long) (dims_out[1]) <<
             " z " << (unsigned long) (dims_out[2]) <<
             endl;*/
        meta.nBand = dims_out[0];
        meta.nRow = dims_out[1];
        meta.nCol = dims_out[2];
        meta.nPixel = meta.nRow * meta.nCol;

        data = new float[meta.nPixel];

        // get date from file header
        static const char* AttributeName = "FileHeader";
        static const string StartDate = "StartGranuleDateTime";
        if(h5File.attrExists(AttributeName)) {
            Attribute fileHeader = h5File.openAttribute(AttributeName);
            int num = fileHeader.getInMemDataSize();
            char* context = new char[num + 1];
            DataType dataType = fileHeader.getDataType();
            fileHeader.read(dataType, context);

            auto attMap = parseAttribute(context);
            meta.date = attMap[StartDate].substr(0, 10);
        }

         // Read data from hyperslab in the file-operator into the hyperslab in
         dataset.read(data, dataType);
    }
    // catch failure caused by the H5File operationsparseAttribute
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

bool Hdfv::write(string fName) {
    return true;
}

std::unordered_map<string, string> Hdfv::parseAttribute(char* attText){
    static const string split = "=";
    static const string split2 = ";";
    string text = attText;
    int prev = 0, cur,next;
    std::unordered_map<string, string> attMap;
    while(prev < text.size() && (cur = text.find(split, prev)) != -1){
        string key = text.substr(prev, cur - prev);
        string value = "";
        ++cur;
        if((next = text.find(split2, cur)) != -1){
            value = text.substr(cur, next - cur);
        };

        attMap[key] = value;

        prev = next + 2;
    }
    return attMap;
}
