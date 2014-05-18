#ifndef VW_EXAMPLE_H
#define VW_EXAMPLE_H

#include <string>
#include "fmatrix.h"
#include "parse_example.h" //from vw

class VWExample{
public:
	DATA_FLOAT label;
	int weight;
	sparse_row<DATA_FLOAT> row;
	
public:
	VWExample()
	{
		label = FLT_MAX;
		weight = 1;
		tag = "";
	}	
};

class VWExampleLoad{
	
public:
	int hash(){
		
		
	}
	
	int parse_line(const std::string &line){
		//  label weight tag|namespace fea:val fea:val |namespace ...
		//  label weight |
		//  label tag|
		//   label |
		
	}


};


#endif	
