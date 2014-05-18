/*
	Large-Scale Sparse Matrix

	Author:   Steffen Rendle, http://www.libfm.org/
	modified: 2012-06-08

	Copyright 2011-2012 Steffen Rendle, see license.txt for more information
*/

#ifndef FMATRIX_H_
#define FMATRIX_H_

#include <limits>
#include <vector>
#include <assert.h>
#include <iostream>
#include <fstream>
#include "../util/random.h"
#include "../util/matrix.h"


const uint FMATRIX_EXPECTED_FILE_ID = 2;

template <typename T> struct sparse_entry {
    uint id;
    T value;
};
	
template <typename T> struct sparse_row {
	sparse_entry<T>* data;
	uint size;
};

struct file_header {
	uint id;
	uint float_size;
	uint64 num_values;
	uint num_rows;
	uint num_cols;
}; 

template <typename T> class LargeSparseMatrix {
	public:
		virtual void begin() = 0; // go to the beginning
		virtual bool end() = 0;   // are we at the end?
		virtual void next() = 0; // go to the next line
		virtual sparse_row<T>& getRow() = 0; // pointer to the current row 
		virtual uint getRowIndex() = 0; // index of current row (starting with 0)
		virtual uint getNumRows() = 0; // get the number of Rows
		virtual uint getNumCols() = 0; // get the number of Cols
		virtual uint64 getNumValues() = 0; // get the number of Values
		

		void saveToBinaryFile(std::string filename) {
			std::cout << "printing to " << filename << std::endl; std::cout.flush();
			std::ofstream out(filename.c_str(), std::ios_base::out | std::ios_base::binary);
			if (out.is_open()) {
				file_header fh;
				fh.id = FMATRIX_EXPECTED_FILE_ID;
				fh.num_values = getNumValues();
				fh.num_rows = getNumRows();
				fh.num_cols = getNumCols();
				fh.float_size = sizeof(T);
				out.write(reinterpret_cast<char*>(&fh), sizeof(fh));
				for (begin(); !end(); next()) {
					out.write(reinterpret_cast<char*>(&(getRow().size)), sizeof(uint));
					out.write(reinterpret_cast<char*>(getRow().data), sizeof(sparse_entry<T>)*getRow().size);
				}
				out.close();
			} else {
				throw "could not open " + filename;
			}
		}

		void saveToTextFile(std::string filename) {
			std::cout << "printing to " << filename << std::endl; std::cout.flush();
			std::ofstream out(filename.c_str());
			if (out.is_open()) {
				for (begin(); !end(); next()) {
					for (uint i = 0; i < getRow().size; i++) {
						out << getRow().data[i].id << ":" << getRow().data[i].value;
						if ((i+1) < getRow().size) {
							out << " ";
						} else {
							out << "\n";
						}
					}
				}
				out.close();
			} else {
				throw "could not open " + filename;
			}
		}
};


template <typename T> class LargeSparseMatrixMemory : public LargeSparseMatrix<T> {
	protected:
		 uint index;
	public:
		DVector< sparse_row<T> > data;
		uint num_cols;
		uint64 num_values;
		virtual void begin() { index = 0; };
		virtual bool end() { return index >= data.dim; }
		virtual void next() { index++;}
		virtual sparse_row<T>& getRow() { return data(index); };
		virtual uint getRowIndex() { return index; };
		virtual uint getNumRows() { return data.dim; };
		virtual uint getNumCols() { return num_cols; };
		virtual uint64 getNumValues() { return num_values; };

//		void loadFromTextFile(std::string filename);
};




#endif /*FMATRIX_H_*/
