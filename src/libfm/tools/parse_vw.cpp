#include <iostream>
#include <fstream>
#include "../../util/parse_example.h"
using namespace std;


int main(int argc, char * argv[])
{
	if(argc <= 1){
		
		cerr<<"Need input file."<<endl;
		return 1;
	}
	VWExample ae;
	VWExampleParser parser;
	ifstream inf(argv[1]);
	string line;
	while(getline(inf, line)){
		ae.clear();
		parser.read_line(line, &ae);
		cout<<"tag:"<<ae.tag<<endl;
		cout<<"label:"<<ae.label<<endl;
		cout<<"namespaces num:"<<ae.indices.size()<<endl;
		for(uint i = 0; i<ae.indices.size();i++){
			unsigned char index = ae.indices[i];
			cout<<"namespace: "<<index<<endl;
			for(int j=0; j< ae.atomics[index].size(); j++)
				cout<<ae.atomics[index][j].id<<' '<<ae.atomics[index][j].value<<endl;
			
		}
		cout<<"-------------------"<<endl;
			
	}

}