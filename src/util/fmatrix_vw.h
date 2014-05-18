/*

wrapper vw format file as LargeSparseMatrix

*/

template <typename T> class LargeSparseMatrixVW : public LargeSparseMatrix<T> {
	protected:
		DVector< sparse_row<T> > data; //num of rows
		DVector< sparse_entry<T> > cache; //num of values
		std::string filename;
		
		std::ifstream in;

		uint64 position_in_data_cache;
		uint number_of_valid_rows_in_cache;
		uint64 number_of_valid_entries_in_cache;
		uint row_index;

		uint num_cols;
		uint64 num_values;
		uint num_rows;	

		void parse_example(const std::string & line)
		{
			
				
				
		
		}
		
		void readcache() {
			if (row_index >= num_rows) { return; }
			number_of_valid_rows_in_cache = 0;
			number_of_valid_entries_in_cache = 0;
			position_in_data_cache = 0;
			do {
				if ((row_index + number_of_valid_rows_in_cache) > (num_rows-1)) {
					break;
				}
				if (number_of_valid_rows_in_cache >= data.dim) { break; }

				sparse_row<T>& this_row = data.value[number_of_valid_rows_in_cache];
				
				in.read(reinterpret_cast<char*>(&(this_row.size)), sizeof(uint));
				if ((this_row.size + number_of_valid_entries_in_cache) > cache.dim) {
					in.seekg(- (long int) sizeof(uint), std::ios::cur);
					break;
				}

				this_row.data = &(cache.value[number_of_valid_entries_in_cache]);
				in.read(reinterpret_cast<char*>(this_row.data), sizeof(sparse_entry<T>)*this_row.size);
			
				number_of_valid_rows_in_cache++;					
				number_of_valid_entries_in_cache += this_row.size;
			} while (true);
	
		}	
	public:
		LargeSparseMatrixVW(std::string filename, uint64 cache_size) { 
			this->filename = filename;
			in.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
			if (in.is_open()) {
				file_header fh;
				in.read(reinterpret_cast<char*>(&fh), sizeof(fh));
				assert(fh.id == FMATRIX_EXPECTED_FILE_ID);
				assert(fh.float_size == sizeof(T));
				this->num_values = fh.num_values;
				this->num_rows = fh.num_rows;
				this->num_cols = fh.num_cols;				
				//in.close();
			} else {
				throw "could not open " + filename;
			}

			if (cache_size == 0) {
				cache_size = std::numeric_limits<uint64>::max();
			}
			// determine cache sizes automatically:
			double avg_entries_per_line = (double) this->num_values / this->num_rows;
			uint num_rows_in_cache;
			{
				uint64 dummy = cache_size / (sizeof(sparse_entry<T>) * avg_entries_per_line + sizeof(uint));
				if (dummy > static_cast<uint64>(std::numeric_limits<uint>::max())) {
					num_rows_in_cache = std::numeric_limits<uint>::max();
				} else {
					num_rows_in_cache = dummy;
				}
			}
			num_rows_in_cache = std::min(num_rows_in_cache, this->num_rows);
			uint64 num_entries_in_cache = (cache_size - sizeof(uint)*num_rows_in_cache) / sizeof(sparse_entry<T>);
			num_entries_in_cache = std::min(num_entries_in_cache, this->num_values);
			std::cout << "num entries in cache=" << num_entries_in_cache << "\tnum rows in cache=" << num_rows_in_cache << std::endl;

			cache.setSize(num_entries_in_cache);
			data.setSize(num_rows_in_cache);
		}
//		~LargeSparseMatrixHD() { in.close(); }

		virtual uint getNumRows() { return num_rows; };
		virtual uint getNumCols() { return num_cols; };
		virtual uint64 getNumValues() { return num_values; };

		virtual void next() {
			row_index++;
			position_in_data_cache++;
			if (position_in_data_cache >= number_of_valid_rows_in_cache) {
				readcache();
			}
		}

		virtual void begin() {
			if ((row_index == position_in_data_cache) && (number_of_valid_rows_in_cache > 0)) {
				// if the beginning is already in the cache, do nothing
				row_index = 0;
				position_in_data_cache = 0;
				// close the file because everything is in the cache
				if (in.is_open()) {
					in.close();
				}
				return;
			}
			row_index = 0;
			position_in_data_cache = 0;
			number_of_valid_rows_in_cache = 0;
			number_of_valid_entries_in_cache = 0;
			in.seekg(sizeof(file_header), std::ios_base::beg);
			readcache();
		}

		virtual bool end() { return row_index >= num_rows; }

		virtual sparse_row<T>& getRow() { return data(position_in_data_cache); }
		virtual uint getRowIndex() { return row_index; }
	
	
};