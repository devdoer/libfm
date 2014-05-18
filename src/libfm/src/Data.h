/*
	Data container for Factorization Machines 

	Author:   Steffen Rendle, http://www.libfm.org/
	modified: 2013-07-04

	Copyright 2010-2013 Steffen Rendle, see license.txt for more information
*/

#ifndef DATA_H_
#define DATA_H_

#include <limits>
#include "../../util/matrix.h"
#include "../../util/fmatrix.h"
#include "../../fm_core/fm_data.h"
#include "../../fm_core/fm_model.h"

typedef FM_FLOAT DATA_FLOAT;



class DataMetaInfo {
	public:
		DVector<uint> attr_group; // attribute_id -> group_id
		uint num_attr_groups;
		DVector<uint> num_attr_per_group;
		uint num_relations;

		DataMetaInfo(uint num_attributes) {
			attr_group.setSize(num_attributes);
			attr_group.init(0);
			num_attr_groups = 1;
			num_attr_per_group.setSize(num_attr_groups);
			num_attr_per_group(0) = num_attributes;
		}
		void loadGroupsFromFile(std::string filename) {
			assert(fileexists(filename));
			attr_group.load(filename);
			num_attr_groups = 0;
			for (uint i = 0; i < attr_group.dim; i++) {
				num_attr_groups = std::max(num_attr_groups, attr_group(i)+1);
			}
			num_attr_per_group.setSize(num_attr_groups);
			num_attr_per_group.init(0);
			for (uint i = 0; i < attr_group.dim; i++) {
				num_attr_per_group(attr_group(i))++;
			}
		}
	
		void debug() {
			std::cout << "#attr=" << attr_group.dim << "\t#groups=" << num_attr_groups << std::endl;
			for (uint g = 0; g < num_attr_groups; g++) {
				std::cout << "#attr_in_group[" << g << "]=" << num_attr_per_group(g) << std::endl;
			}
		}
};

#include "relation.h"

class Data {
	protected:
		uint64 cache_size;
		bool has_xt;
		bool has_x;
		bool vw_format_on;
	public:	
		Data(uint64 cache_size, bool has_x, bool has_xt, bool vw_format_on=false) { 
			this->data_t = NULL;
			this->data = NULL;
			this->cache_size = cache_size;
			this->has_x = has_x;//no original data
			this->has_xt = has_xt;//no transpose
			this->vw_format_on = vw_format_on;
		}

		LargeSparseMatrix<DATA_FLOAT>* data_t;
		LargeSparseMatrix<DATA_FLOAT>* data;// LargeSparseMatrixMemory or LargeSparseMatrixHD
		DVector<DATA_FLOAT> target;//target and features are sperated.

		int num_feature;
		uint num_cases;
 
		DATA_FLOAT min_target;
		DATA_FLOAT max_target;
		DVector<RelationJoin> relation;
		
		void load(std::string filename);	
		void debug();

};

void Data::load(std::string filename) {

	std::cout << "has x = " << has_x << std::endl;
	std::cout << "has xt = " << has_xt << std::endl;
	std::cout<< "vw_format = "<< vw_format_on <<std::endl;
 	assert(has_x || has_xt);

	if(vw_format_on){
		this->data = new LargeSparseMatrixHD<DATA_FLOAT>(filename, cache_size);
		return;
	}
	int load_from = 0;
	if ((! has_x || fileexists(filename + ".data")) && (! has_xt || fileexists(filename + ".datat")) && fileexists(filename + ".target")) {
		load_from = 1;
	} else if ((! has_x || fileexists(filename + ".x")) && (! has_xt || fileexists(filename + ".xt")) && fileexists(filename + ".y")) {
		load_from = 2;
	}


	if (load_from > 0) {
		uint num_values = 0;
		uint64 this_cs = cache_size;
		if (has_xt && has_x) { this_cs /= 2; }
		
		if (load_from == 1) {
			this->target.loadFromBinaryFile(filename + ".target");
		} else {//load_from ==2, binary target data
			this->target.loadFromBinaryFile(filename + ".y");
		}
		if (has_x) {
			std::cout << "data... ";
			if (load_from == 1) {
				this->data = new LargeSparseMatrixHD<DATA_FLOAT>(filename + ".data", this_cs);
			} else {//load_from == 2, binary predictor data
				this->data = new LargeSparseMatrixHD<DATA_FLOAT>(filename + ".x", this_cs);
			}
			assert(this->target.dim == this->data->getNumRows());
			this->num_feature = this->data->getNumCols();	
			num_values = this->data->getNumValues();
		} else {
			data = NULL;
		}
		
		data_t = NULL;
		
		
		min_target = +std::numeric_limits<DATA_FLOAT>::max();
		max_target = -std::numeric_limits<DATA_FLOAT>::max();
		for (uint i = 0; i < this->target.dim; i++) {
			min_target = std::min(this->target(i), min_target);
			max_target = std::max(this->target(i), max_target);				
		}
		num_cases = target.dim;

		std::cout << "num_cases=" << this->num_cases << "\tnum_values=" << num_values << "\tnum_features=" << this->num_feature << "\tmin_target=" << min_target << "\tmax_target=" << max_target << std::endl;
		return;
	}//end load_from>0

	this->data = new LargeSparseMatrixMemory<DATA_FLOAT>();
	
	DVector< sparse_row<DATA_FLOAT> >& data = ((LargeSparseMatrixMemory<DATA_FLOAT>*)this->data)->data;

	int num_rows = 0;
	uint64 num_values = 0;
	num_feature = 0;
	bool has_feature = false;
	min_target = +std::numeric_limits<DATA_FLOAT>::max();
	max_target = -std::numeric_limits<DATA_FLOAT>::max();
	
	// (1) determine the number of rows and the maximum feature_id, ie. num_rows, num_feature by max feature id
	//     num_values: the number of values， ie. sparse matrix elem numbers
	{
		std::ifstream fData(filename.c_str());
		if (! fData.is_open()) {
			throw "unable to open " + filename;
		}
		DATA_FLOAT _value;
		int nchar, _feature;
		while (!fData.eof()) {
			std::string line;
			std::getline(fData, line);
			const char *pline = line.c_str();
			while ((*pline == ' ')  || (*pline == 9)) { pline++; } // skip leading spaces
			if ((*pline == 0)  || (*pline == '#')) { continue; }  // skip empty rows
			if (sscanf(pline, "%f%n", &_value, &nchar) >=1) {
				pline += nchar;
				min_target = std::min(_value, min_target);
				max_target = std::max(_value, max_target);			
				num_rows++;  //instance number
				while (sscanf(pline, "%d:%f%n", &_feature, &_value, &nchar) >= 2) {
					pline += nchar;	
					num_feature = std::max(_feature, num_feature);//max feature id
					has_feature = true;
					num_values++;	
				}
				while ((*pline != 0) && ((*pline == ' ')  || (*pline == 9))) { pline++; } // skip trailing spaces
				if ((*pline != 0)  && (*pline != '#')) { 
					throw "cannot parse line \"" + line + "\" at character " + pline[0];
				}
			} else {
				throw "cannot parse line \"" + line + "\" at character " + pline[0];
			}
		} 
		fData.close();
	}	

	if (has_feature) {	
		num_feature++; // number of feature is bigger (by one) than the largest value
	}
	std::cout << "num_rows=" << num_rows << "\tnum_values=" << num_values << "\tnum_features=" << num_feature << "\tmin_target=" << min_target << "\tmax_target=" << max_target << std::endl;
	data.setSize(num_rows);//data is of type DVector< sparse_row<DATA_FLOAT> >
	target.setSize(num_rows);
	
	((LargeSparseMatrixMemory<DATA_FLOAT>*)this->data)->num_cols = num_feature;
	((LargeSparseMatrixMemory<DATA_FLOAT>*)this->data)->num_values = num_values;

	MemoryLog::getInstance().logNew("data_float", sizeof(sparse_entry<DATA_FLOAT>), num_values);			
	// cache  store all values
	sparse_entry<DATA_FLOAT>* cache = new sparse_entry<DATA_FLOAT>[num_values]; 
	
	// (2) read the data
	{
		std::ifstream fData(filename.c_str());
		if (! fData.is_open()) {
			throw "unable to open " + filename;
		}
		int row_id = 0;
		uint64 cache_id = 0;
		DATA_FLOAT _value;
		int nchar, _feature;
		while (!fData.eof()) {
			std::string line;
			std::getline(fData, line);
			const char *pline = line.c_str();
			while ((*pline == ' ')  || (*pline == 9)) { pline++; } // skip leading spaces
			if ((*pline == 0)  || (*pline == '#')) { continue; }  // skip empty rows
			if (sscanf(pline, "%f%n", &_value, &nchar) >=1) {
				pline += nchar;
				assert(row_id < num_rows);
				target.value[row_id] = _value;
				data.value[row_id].data = &(cache[cache_id]);//set the row's memory space
				data.value[row_id].size = 0;
			
				while (sscanf(pline, "%d:%f%n", &_feature, &_value, &nchar) >= 2) {
					pline += nchar;	
					assert(cache_id < num_values);
					cache[cache_id].id = _feature;
					cache[cache_id].value = _value;
					cache_id++;
					data.value[row_id].size++;
				}
				row_id++;//new line

				while ((*pline != 0) && ((*pline == ' ')  || (*pline == 9))) { pline++; } // skip trailing spaces
				if ((*pline != 0)  && (*pline != '#')) { 
					throw "cannot parse line \"" + line + "\" at character " + pline[0];
				}
			} else {
				throw "cannot parse line \"" + line + "\" at character " + pline[0];
			}
		}
		fData.close();
		
		assert(num_rows == row_id);
		assert(num_values == cache_id);		
	}	

	num_cases = target.dim;

}




void Data::debug() {
	if (has_x) {
		for (data->begin(); (!data->end()) && (data->getRowIndex() < 4); data->next() ) {
			std::cout << target(data->getRowIndex());
			for (uint j = 0; j < data->getRow().size; j++) {
				std::cout << " " << data->getRow().data[j].id << ":" << data->getRow().data[j].value;	
			}
			std::cout << std::endl;
		}
	}	
}

#endif /*DATA_H_*/
