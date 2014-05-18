#ifndef VW_EXAMPLE_H
#define VW_EXAMPLE_H

#include <string>
#include <vector>
#include <float.h>
#include "../util/fmatrix.h"
#include "../fm_core/fm_data.h"
#define MAX_ENTRY_PER_ROW 4096

typedef sparse_entry<float> Feature;

class VWExample{
public:
	float label;
	int weight;
	float initial ;
	std::string tag;
	std::vector<unsigned char> indices;
	std::vector<Feature> atomics[256]; 
	std::vector<Feature > features;
	
public:
	VWExample()
	{
		label = FLT_MAX;
		weight = 1.;
		initial = 0.;
		tag = "";
		features.reserve(MAX_ENTRY_PER_ROW);
		
	}	
	void clear(){
		label = FLT_MAX;
		weight = 1.;
		initial = 0.;
		tag = "";
		for(int i = 0; i<indices.size();i++){
			unsigned char index = indices[i];
			atomics[index].clear();
		}	
		indices.clear();
		features.clear();
		
	}
};



#endif	
