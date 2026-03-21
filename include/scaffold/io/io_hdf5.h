#ifndef SCAFFOLD_IO_IO_HDF5_H_
#define SCAFFOLD_IO_IO_HDF5_H_

#include "H5Cpp.h"
#include "hdf5_datatype.h"

namespace scaffold { namespace io {

class IO
{
public:
  std::string file_name;
  bool is_writer;  ///< If false, all write operations are no-ops (for non-master procs in a group)
  IO() : is_writer(true) {}
  void Load(std::string &tmp_file_name)
  {
    file_name = tmp_file_name;
  }

  void Create()
  {
    if (!is_writer) return;
    // Create file
    H5::H5File* file = new H5::H5File(file_name, H5F_ACC_TRUNC);

    // Delete pointer
    delete file;
  }

  // Read
  template<class T>
  void Read(const std::string &dataset_name, T& data)
  {
    // Open file
    H5::H5File* file = new H5::H5File(file_name, H5F_ACC_RDONLY);

    // Do read
    H5::DataSet* dataset = new H5::DataSet(file->openDataSet(dataset_name));
    H5::DataSpace dataspace = dataset->getSpace();
    H5::AtomType datatype = HDF5TypeTraits<T>::GetType(data);
    dataset->read(HDF5TypeTraits<T>::GetAddr(data), datatype, dataspace, dataspace);

    // Delete pointers
    delete dataset;
    delete file;
  }

  // Write
  template<class T>
  void Write(const std::string &dataset_name, T& data)
  {
    if (!is_writer) return;
    // Open file
    H5::H5File* file = new H5::H5File(file_name, H5F_ACC_RDWR);

    // Do write
    H5::AtomType datatype(HDF5TypeTraits<T>::GetType(data));
    datatype.setOrder(H5T_ORDER_LE);
    int data_rank = HDF5TypeTraits<T>::GetRank(data);
    hsize_t data_shape[data_rank];
    for (int i=0; i<data_rank; ++i)
      data_shape[i] = HDF5TypeTraits<T>::GetDim(data,i);
    H5::DataSpace dataspace(data_rank, data_shape);
    H5::DataSet* dataset = new H5::DataSet(file->createDataSet(dataset_name, datatype, dataspace));
    dataset->write(HDF5TypeTraits<T>::GetAddr(data), datatype);

    // Delete pointers
    delete dataset;
    delete file;
  }

  // Rewrite
  template<class T>
  void Rewrite(const std::string &dataset_name, T& data)
  {
    if (!is_writer) return;
    // Open file
    H5::H5File* file = new H5::H5File(file_name, H5F_ACC_RDWR);

    // Do write
    H5::AtomType datatype(HDF5TypeTraits<T>::GetType(data));
    datatype.setOrder(H5T_ORDER_LE);
    int data_rank = HDF5TypeTraits<T>::GetRank(data);
    hsize_t data_shape[data_rank];
    for (int i=0; i<data_rank; ++i)
      data_shape[i] = HDF5TypeTraits<T>::GetDim(data,i);
    H5::DataSpace dataspace(data_rank, data_shape);
    H5::DataSet* dataset = new H5::DataSet(file->openDataSet(dataset_name));
    dataset->write(HDF5TypeTraits<T>::GetAddr(data), datatype);

    // Delete pointers
    delete dataset;
    delete file;
  }

  // Create Group
  void CreateGroup(const std::string &group_name)
  {
    if (!is_writer) return;
    // Open file
    H5::H5File* file = new H5::H5File(file_name, H5F_ACC_RDWR);

    // Create group
    H5::Group group = file->createGroup(group_name);

    // Delete pointer
    delete file;
  }

  // Create extendable dataset
  template<class T>
  void CreateExtendableDataSet(const std::string &prefix, const std::string &dataset_name, T& data)
  {
    if (!is_writer) return;
    // Open file
    H5::H5File* file = new H5::H5File(file_name, H5F_ACC_RDWR);

    // Get data information
    int data_rank = HDF5TypeTraits<T>::GetRank(data);
    hsize_t data_shape[data_rank];
    for (int i=0; i<data_rank; ++i)
      data_shape[i] = HDF5TypeTraits<T>::GetDim(data,i);
    H5::AtomType datatype = HDF5TypeTraits<T>::GetType(data);

    // Create the data space with one unlimited dimension.
    int rank = data_rank + 1;
    hsize_t dims[rank];
    hsize_t maxdims[rank];
    dims[0] = 1;
    maxdims[0] = H5S_UNLIMITED;
    for (int i=1; i<rank; i++) {
      dims[i] = data_shape[i-1];
      maxdims[i] = data_shape[i-1];
    }
    H5::DataSpace mspace(rank, dims, maxdims);

    // Modify dataset creation properties, i.e. enable chunking.
    H5::DSetCreatPropList cparms;
    const hsize_t* chunk_dims = dims; // Default chunk size to 1 x shape(data)
    cparms.setChunk(rank, chunk_dims);

    // Set fill value for the dataset
    int fill_val = 0;
    cparms.setFillValue(datatype, &fill_val);

    // Create a new dataset within the file using cparms creation properties.
    std::string full_name = prefix + dataset_name;
    H5::DataSet* dataset = new H5::DataSet(file->createDataSet(full_name, datatype, mspace, cparms));

    // Extend the dataset.
    dataset->extend(dims);

    // Select a hyperslab.
    H5::DataSpace fspace = dataset->getSpace();
    hsize_t offset[rank];
    for (int i=0; i<rank; i++)
      offset[i] = 0;
    fspace.selectHyperslab(H5S_SELECT_SET, dims, offset);

    // Write original data into hyperslab
    dataset->write(HDF5TypeTraits<T>::GetAddr(data), datatype, mspace, fspace);

    // Delete pointers
    delete dataset;
    delete file;
  }

  // Extend dataset
  template<class T>
  void AppendDataSet(const std::string &prefix, const std::string &dataset_name, T& data)
  {
    if (!is_writer) return;
    // Open file
    H5::H5File* file = new H5::H5File(file_name, H5F_ACC_RDWR);

    // Get data information
    int data_rank = HDF5TypeTraits<T>::GetRank(data);
    hsize_t data_shape[data_rank];
    for (int i=0; i<data_rank; ++i)
      data_shape[i] = HDF5TypeTraits<T>::GetDim(data,i);
    H5::AtomType datatype = HDF5TypeTraits<T>::GetType(data);

    // Open data set
    std::string full_name = prefix + dataset_name;
    H5::DataSet* dataset = new H5::DataSet(file->openDataSet(full_name));

    // Get old dataspace properties
    H5::DataSpace fspace = dataset->getSpace();
    int rank = data_rank + 1;
    hsize_t dims_old[rank], maxdims[rank];
    fspace.getSimpleExtentDims(dims_old, maxdims);

    // Extend the dataset.
    hsize_t dims_new[rank];
    dims_new[0] = dims_old[0]+1;
    for (int i=1; i<rank; i++)
      dims_new[i] = data_shape[i-1];
    dataset->extend(dims_new);

    // Select a hyperslab.
    fspace = dataset->getSpace();
    hsize_t offset[rank], dims_orig[rank];
    offset[0] = dims_old[0];
    dims_orig[0] = 1;
    for (int i=1; i<rank; i++) {
      offset[i] = 0;
      dims_orig[i] = data_shape[i-1];
    }
    fspace.selectHyperslab(H5S_SELECT_SET, dims_orig, offset);

    // Define memory space
    H5::DataSpace mspace(rank, dims_orig);

    // Write the data to the hyperslab.
    dataset->write(HDF5TypeTraits<T>::GetAddr(data), datatype, mspace, fspace);

    // Delete pointers
    delete dataset;
    delete file;
  }

};

// Specialization for strings

// Read
template<>
void IO::Read(const std::string &dataset_name, std::string &data)
{
  // Open file
  H5::H5File* file = new H5::H5File(file_name, H5F_ACC_RDONLY);

  // Do read
  char t_data[data.size()];
  H5::DataSet* dataset = new H5::DataSet(file->openDataSet(dataset_name));
  H5::DataSpace dataspace = dataset->getSpace();
  H5::DataType datatype = dataset->getDataType();
  dataset->read(t_data, datatype, dataspace, dataspace);
  data = std::string(t_data);

  // Delete pointers
  delete dataset;
  delete file;
}

// Write
template<>
void IO::Write(const std::string &dataset_name, std::string &data)
{
  if (!is_writer) return;
  // Open file
  H5::H5File* file = new H5::H5File(file_name, H5F_ACC_RDWR);

  // Do write
  char t_data[data.size()+1];
  strcpy(t_data, data.c_str());
  H5::StrType datatype(H5::PredType::C_S1, data.size());
  int data_rank = 1;
  hsize_t data_shape[1];
  data_shape[0] = 1;
  H5::DataSpace dataspace(data_rank, data_shape);
  H5::DataSet* dataset = new H5::DataSet(file->createDataSet(dataset_name, datatype, dataspace));
  dataset->write(t_data, datatype);

  // Delete pointers
  delete dataset;
  delete file;
}

// Rewrite
template<>
void IO::Rewrite(const std::string &dataset_name, std::string &data)
{
  if (!is_writer) return;
  // Open file
  H5::H5File* file = new H5::H5File(file_name, H5F_ACC_RDWR);

  // Do write
  char t_data[data.size()+1];
  strcpy(t_data, data.c_str());
  H5::StrType datatype(H5::PredType::C_S1, data.size());
  int data_rank = 1;
  hsize_t data_shape[1];
  data_shape[0] = 1;
  H5::DataSpace dataspace(data_rank, data_shape);
  H5::DataSet* dataset = new H5::DataSet(file->openDataSet(dataset_name));
  dataset->write(t_data, datatype);

  // Delete pointers
  delete dataset;
  delete file;
}

// Specialization for bools

// Write
template<>
void IO::Write(const std::string &dataset_name, bool &t_data)
{
  int data = int(t_data);
  Write(dataset_name, data);
}

}} // namespace

#endif // SCAFFOLD_IO_IO_HDF5_H_
