/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.

@desp: parse a line into one example
 */

#ifndef PARSE_EXAMPLE_H_
#define PARSE_EXAMPLE_H_

#include <math.h>
#include <ctype.h>
#include "hash.h"
#include "cache.h"
// #include "unique_sort.h"
#include "v_array.h"
#include "vw_example.h"
#include "parse_prim.h"

using namespace std;



size_t hashstring (substring s, uint32_t h)
{//h is a hash base
  size_t ret = 0;
  //trim leading whitespace but not UTF-8
  for(; s.begin < s.end && *(s.begin) <= 0x20 && (int)*(s.begin) >= 0; s.begin++);
  //trim trailing white space but not UTF-8
  for(; s.end > s.begin && *(s.end-1) <= 0x20 && (int)*(s.end-1) >=0; s.end--);

  const char *p = s.begin;
  while (p != s.end)
    if (*p >= '0' && *p <= '9')
      ret = 10*ret + *(p++) - '0';
    else
      return uniform_hash((unsigned char *)s.begin, s.end - s.begin, h);

  return ret + h;
}

size_t hashall (substring s, uint32_t h)
{
  return uniform_hash((unsigned char *)s.begin, s.end - s.begin, h);
}

typedef size_t (*hash_func_t)(substring, uint32_t);

hash_func_t getHasher(const string& s){
  if (s=="strings")
    return hashstring;
  else if(s=="all")
    return hashall;
  else{
    cerr << "Unknown hash function: " << s << ". Exiting " << endl;
    throw exception();
  }
}



class TC_parser {
  
public:
  const char* beginLine;
  const char* reading_head;
  const char* endLine;
  float cur_channel_v;
  bool  new_index;
  size_t anon; 
  size_t channel_hash;
  const char* base;
  unsigned char index;
  float v;
  VWExample * ae;
  hash_func_t hasher;
  uint32_t weights_per_problem;
  
  ~TC_parser(){ }
  
    //read feature value
  inline float featureValue(){
    if(reading_head == endLine || *reading_head == '|' || *reading_head == ' ' || *reading_head == '\t' || *reading_head == '\r')
      return 1.;
    else if(*reading_head == ':'){
      // featureValue --> ':' 'Float'
      ++reading_head;
      const char *end_read = NULL;
      v = parseFloat(reading_head,&end_read);
      if(end_read == reading_head){
	cout << "malformed example !\nFloat expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
      }
      if(nanpattern(v)) {
        v = 0.f;
        cout << "warning: invalid feature value:\"" << std::string(reading_head, end_read - reading_head).c_str() << "\" read as NaN. Replacing with 0." << endl;
      }
      reading_head = end_read;
      return v;
    }else{
      // syntax error
      cout << "malformed example !\n'|' , ':' , space or EOL expected after : \"" << 				std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
      return 0.f;
    }
  }

  inline substring read_name(){
    substring ret;
    ret.begin = reading_head;
    while( !(*reading_head == ' ' || *reading_head == '\t' || *reading_head == ':' ||*reading_head == '|' || reading_head == endLine || *reading_head == '\r' ))
      ++reading_head;
    ret.end = reading_head;
    return ret;
  }
  
  inline void maybeFeature(){
//read a feature
    if(*reading_head == ' ' || *reading_head == '\t' || *reading_head == '|'|| reading_head == endLine || *reading_head == '\r' ){
      // maybeFeature --> ø
    }else {
      // maybeFeature --> 'String' FeatureValue
      substring feature_name=read_name();
      v = cur_channel_v * featureValue();
      size_t word_hash;
      if (feature_name.end != feature_name.begin)
	word_hash = (this->hasher(feature_name,(uint32_t)channel_hash));
      else
	word_hash = channel_hash + anon++;//add based namespace hash
      if(v == 0) return; //dont add 0 valued features to list of features
      // feature f = {v,(uint32_t)word_hash * weights_per_problem};
      Feature f = {(uint32_t)word_hash * weights_per_problem,v};
      // ae->sum_feat_sq[index] += v*v;//for this namespace
      ae->atomics[index].push_back(f);
	 }
}

  inline void nameSpaceInfoValue(){
      if(*reading_head == ' ' || *reading_head == '\t' || reading_head == endLine || *reading_head == '|' || *reading_head == '\r' ){
          // nameSpaceInfoValue -->  ø
      }else if(*reading_head == ':'){
          // nameSpaceInfoValue --> ':' 'Float'
          ++reading_head;
          const char *end_read = NULL;
          cur_channel_v = parseFloat(reading_head,&end_read);
          if(end_read == reading_head){
              cout << "malformed example !\nFloat expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
          }
          if(nanpattern(cur_channel_v)) {
              cur_channel_v = 1.f;
              cout << "warning: invalid namespace value:\"" << std::string(reading_head, end_read - reading_head).c_str() << "\" read as NaN. Replacing with 1." << endl;
          }
          reading_head = end_read;
      }else{
          // syntax error
          cout << "malformed example !\n'|' , ':' , space or EOL expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
      }
  }

  inline void nameSpaceInfo(){
      //read feature namespace
      if(reading_head == endLine ||*reading_head == '|' || *reading_head == ' ' || *reading_head == '\t' || *reading_head == ':' || *reading_head == '\r'){
          // syntax error
          cout << "malformed example !\nString expected after : " << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
      }else{
          // NameSpaceInfo --> 'String' NameSpaceInfoValue
          index = (unsigned char)(*reading_head); //index is namespace
          // if(ae->atomics[index].begin == ae->atomics[index].end)
          if(ae->atomics[index].size()==0)
              new_index = true;
          substring name = read_name();

          channel_hash = this->hasher(name, hash_base);
          nameSpaceInfoValue();
      }
  }

  /**
   * list features of one namespace
   */
  inline void listFeatures(){
      while(*reading_head == ' ' || *reading_head == '\t'){
          //listFeatures --> ' ' MaybeFeature ListFeatures
          ++reading_head;
          maybeFeature();//read a feature
      }
      if(!(*reading_head == '|' || reading_head == endLine || *reading_head == '\r')){
          //syntax error
          cout << "malformed example !\n'|' , space or EOL expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str() << "\"" << endl;
      }
  }


  inline void nameSpace(){
//process one namespace
      cur_channel_v = 1.0;
      base = NULL;
      index = 0;
      new_index = false;
      anon = 0;
      if(*reading_head == ' ' || *reading_head == '\t' || reading_head == endLine || *reading_head == '|' || *reading_head == '\r' ){//no namespace
          // NameSpace --> ListFeatures
          index = (unsigned char)' ';
          // if(ae->atomics[index].begin == ae->atomics[index].end)
          if(ae->atomics[index].size() == 0)
              new_index = true;
          
          channel_hash = 0;
          listFeatures();
      }else if(*reading_head != ':'){
          // NameSpace --> NameSpaceInfo ListFeatures
          nameSpaceInfo();
          listFeatures();
      }else{
          // syntax error
          cout << "malformed example !\n'|' , String, space or EOL expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
      }
      // if(new_index && ae->atomics[index].begin != ae->atomics[index].end)
      if(new_index && ae->atomics[index].size() != 0)
          //save namespace into index
		  cout<<"new namespace "<<index<<endl;
          ae->indices.push_back(index);
  }

  inline void listNameSpace(){
	  
      while(*reading_head == '|'){
          // ListNameSpace --> '|' NameSpace ListNameSpace
          ++reading_head;
          nameSpace();
      }
      if(reading_head != endLine && *reading_head != '\r')
      {
          // syntax error
          cout << "malformed example !\n'|' or EOL expected after : \"" << std::string(beginLine, reading_head - beginLine).c_str()<< "\"" << endl;
      }
  }

  TC_parser(const char* reading_head, const char* endLine,  VWExample * ae){
      this->beginLine = reading_head;
      this->reading_head = reading_head;
      this->endLine = endLine;
	  this->hasher = hashstring;
      this->ae = ae;
      this->weights_per_problem = 1;// ie. dims per feature
      listNameSpace();
  }

};




class VWExampleParser{
public:
	v_array<substring> words;
public:
	void parse_label(VWExample *ex,  v_array<substring>& words)
	{//parse the instance label

	  //cerr << "call parse_simple_label" << words.size()  << endl;

	  switch(words.size()) {
	  case 0:
	    break;
	  case 1:
	    ex->label = float_of_substring(words[0]);
	    break;
	  case 2:
	    ex->label = float_of_substring(words[0]);
	    ex->weight = float_of_substring(words[1]);
	    break;
	  case 3:
	    ex->label = float_of_substring(words[0]);
	    ex->weight = float_of_substring(words[1]);
	    ex->initial = float_of_substring(words[2]);
	    break;
	  default:
	    cerr << "malformed example!\n";
	    cerr << "words.size() = " << words.size() << endl;
	  }
	  if (words.size() > 0  && fabs(ex->label) != 1.f)
	    cout << "You are using a label not -1 or 1 with a loss function expecting that!" << endl;
	}
	
	void substring_to_example( VWExample* ae, substring line)
	{
	    //parse label header?  
	
	    const char* bar_location = safe_index(line.begin, '|', line.end);
	    const char* tab_location = safe_index(line.begin, '\t', bar_location);
	    substring label_space;
	    if (tab_location != bar_location){
	        label_space.begin = tab_location + 1;
	    }else{
	        label_space.begin = line.begin;//strip begin tab
	    }
	    label_space.end = bar_location;

	    if (*line.begin == '|')	{
	        this->words.erase();
	    } else 	{
	        tokenize(' ', label_space, this->words);
	        if (this->words.size() > 0 && (this->words.last().end == label_space.end|| *(this->words.last().begin) == '\'')) //The last field is a tag, so record and strip it off
	        {
	            substring tag = this->words.pop();
	            //cerr<<"tag:"<<string(tag.begin,tag.end).c_str()<<endl;
	            if (*tag.begin == '\'')
	                tag.begin++;
				
				ae->tag = std::string(tag.begin, tag.end);
	        }
	    }

	    if (this->words.size() > 0)
	        this->parse_label(ae, this->words);

	    //parse namespaces of features
	    TC_parser parser_line(bar_location,line.end, ae);
	}



	void read_line_cstyle(  const char* line, VWExample* ex)
	{
	    substring ss = {line, line+strlen(line)};
	    this->substring_to_example( ex, ss);  
	}

	void read_line( const string & line, VWExample * ex)
	{
		this->read_line_cstyle(line.c_str(), ex);
	}
};

#endif
