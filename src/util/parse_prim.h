#ifndef PARSE_PRIM_H
#define PARSE_PRIM_H

struct substring {
  const char *begin;
  const char *end;
};

void tokenize(char delim, substring s, v_array<substring>& ret)
{
  ret.erase();
  const char *last = s.begin;
  for (; s.begin != s.end; s.begin++) {
    if (*s.begin == delim) {
      if (s.begin != last)
	{
	  substring temp = {last, s.begin};
	  ret.push_back(temp);
	}
      last = s.begin+1;
    }
  }
  if (s.begin != last)
    {
      substring final = {last, s.begin};
      ret.push_back(final);
    }
}

inline const char* safe_index(const char *start, char v, const char *max)
{
  while (start != max && *start != v)
    start++;
  return start;
}
inline float parseFloat(const char * p,  const char ** end)
{
  const char* start = p;

  if (!*p)
    return 0;
  int s = 1;
  while (*p == ' ') p++;
  
  if (*p == '-') {
    s = -1; p++;
  }
  
  float acc = 0;
  while (*p >= '0' && *p <= '9')
    acc = acc * 10 + *p++ - '0';
  
  int num_dec = 0;
  if (*p == '.') {
    p++;
    while (*p >= '0' && *p <= '9') {
      acc = acc *10 + (*p++ - '0') ;
      num_dec++;
    }
  }
  int exp_acc = 0;
  if(*p == 'e' || *p == 'E'){
    p++;
    int exp_s = 1;
    if (*p == '-') {
      exp_s = -1; p++;
    }
    while (*p >= '0' && *p <= '9')
      exp_acc = exp_acc * 10 + *p++ - '0';
    exp_acc *= exp_s;
    
  }
  if (*p == ' ')//easy case succeeded.
    {
      acc *= powf(10,(float)(exp_acc-num_dec));
      *end = p;
      return s * acc;
    }
  else
    return (float)strtod(start,(char **)end);
}


inline bool nanpattern( float value ) { return ((*(uint32_t*)&value) & 0x7fC00000) == 0x7fC00000; } 

inline float float_of_substring(substring s)
{
  const char* endptr = s.end;
  float f = parseFloat(s.begin, &endptr);
  if ((endptr == s.begin && s.begin != s.end) || nanpattern(f))
    {
      std::cout << "warning: " << std::string(s.begin, s.end-s.begin).c_str() << " is not a good float, replacing with 0" << std::endl;
      f = 0;
    }
  return f;
}

#endif