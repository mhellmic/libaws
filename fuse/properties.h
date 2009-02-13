#ifndef SAUSA_UTILS_S3LAYER_H
#define SAUSA_UTILS_S3LAYER_H

#include <map>
#include <string>
#include <iostream>

namespace s3fs { namespace utils {

class Properties
{
public:
  static const char* BUCKET_NAME;
  static const char* AWS_ACCESS_KEY;
  static const char* AWS_SECRET_ACCESS_KEY;
};

class PropertyUtil
{
  enum { DEBUG = 0 };

public:
  typedef std::map<std::string, std::string> PropertyMapT;
  typedef PropertyMapT::value_type           value_type;
  typedef PropertyMapT::iterator             iterator;

  static void read(const char *filename, PropertyMapT &map);
  static void read(std::istream &is, PropertyMapT &map);
  static void write(const char *filename, PropertyMapT &map, const char *header = NULL);
  static void write(std::ostream &os, PropertyMapT &map, const char *header = NULL);
  static void print(std::ostream &os, PropertyMapT &map);

private:
  static inline char m_hex(int nibble)
  {   
    static const char *digits = "0123456789ABCDEF";
    return digits[nibble & 0xf];
  }   
};

}}

#endif
