/*

wrapper vw format file as LargeSparseMatrix

*/
#ifndef FMATRIX_VW_H
#define FMATRIX_VW_H

#include "fmatrix.h"

template <typename T> class LargeSparseMatrixVW : public LargeSparseMatrix<T> {
	protected:
		DVector< sparse_row<T> > data; //vector of rows for cache
		DVector< sparse_entry<T> > cache; //vector of values
		std::string filename;		
		std::ifstream in;


		uint max_rows_in_cache; // cache capacity
		uint64 max_entries_in_cache;
		
		uint64 position_in_cache;
		uint number_of_rows_in_cache; // cached real data
		uint64 number_of_entries_in_cache;
		
		uint row_index; 
		uint num_cols; //  size of the file
		uint64 num_values;
		uint num_rows;	
		
		VWExample * ae;
		VWExampleParser *parser;

		int parse_vw_line(const std::string & line, VWExample *ae)
		{
				
			return parser.read_line(line, ae);
				
		}
		
		void readcache() {
			// if (row_index >= num_rows) { return; }
			
			//reset 
			number_of_rows_in_cache = 0;
			number_of_entries_in_cache = 0;
			position_in_cache = 0;
			do {
				// if ((row_index + number_of_rows_in_cache) > (num_rows-1)) {
				if(!std::getline(in, line)){
					cerr<<"All rows are read."<<<endl;
					in.close();
					break;
				}
				
				if(!this->parse_vw_line(line, ae))
				{
					cerr<<"Warning! Got an invalid vw line."<<endl;
					continue;
				}
				if (number_of_rows_in_cache >= data.dim) { break; }//cache is full

				sparse_row<T>& this_row = data.value[number_of_rows_in_cache];//row
				
				in.read(reinterpret_cast<char*>(&(this_row.size)), sizeof(uint));
				if ((this_row.size + number_of_entries_in_cache) > cache.dim) {
					in.seekg(- (long int) sizeof(uint), std::ios::cur);
					break;
				}

				this_row.data = &(cache.value[number_of_entries_in_cache]);//mapping to cached entry
				in.read(reinterpret_cast<char*>(this_row.data), sizeof(sparse_entry<T>)*this_row.size);
			
				number_of_rows_in_cache++;					
				number_of_entries_in_cache += this_row.size;
			} while (true);
	
		}	
	public:
		LargeSparseMatrixVW(std::string filename,  uint max_rows_in_cache, uint number_of_enties_per_row = 256) { 
			assert(number_of_entires_per_row > 0);
			this->parser = new VWExampleParser();
			if(this->parser == NULL){
				
				throw "new VWExampleParser failed!"
			}
			
			this->filename = filename;
			if (max_rows_in_cache == 0) {
				this->max_rows_in_cache = std::numeric_limits<uint64>::max();
			}
			else
			{
				this->max_rows_in_cache = max_rows_in_cache;
			}
			
			in.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
			if (in.is_open()) {
				
				this->num_values = 0;
				this->num_rows = 0;
				this->num_cols = 0;		
				this->number_of_rows_in_cache = 0;
				this->number_of_entires_in_cache = 0;		
				//in.close();
			} else {
				throw "could not open " + filename;
			}

			
			// determine cache sizes automatically:
			this->max_entries_in_cache = this->max_rows_in_cache * number_of_entires_per_row;
			
			
			std::cout << "max entries in cache=" << max_entries_in_cache << "\tmax rows in cache=" << max_rows_in_cache << std::endl;

			cache.setSize(max_entries_in_cache);//num of cached entries
			data.setSize(max_rows_in_cache);//num of cached rows
		}
//		~LargeSparseMatrixHD() { in.close(); }

		virtual uint getNumRows() { return num_rows; };
		virtual uint getNumCols() { return num_cols; };
		virtual uint64 getNumValues() { return num_values; };

		virtual void next() {
			row_index++;
			position_in_cache++;
			if (position_in_cache >= number_of_rows_in_cache) {
				readcache();
			}
		}

		virtual void begin() {
			if ((row_index == position_in_cache) && (number_of_rows_in_cache > 0)) {
				// if the beginning is already in the cache, do nothing
				row_index = 0;
				position_in_cache = 0;
				// close the file because everything is in the cache
				if (in.is_open()) {
					in.close();
				}
				return;
			}
			row_index = 0;
			position_in_cache = 0;
			number_of_rows_in_cache = 0;
			number_of_entries_in_cache = 0;
			in.seekg(sizeof(file_header), std::ios_base::beg);
			readcache();
		}

		virtual bool end() { return row_index >= num_rows; }

		virtual sparse_row<T>& getRow() { return data(position_in_cache); }
		virtual uint getRowIndex() { return row_index; }
	
	
};

#endif