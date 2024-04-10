#ifndef _LIFETIME_H
#define _LIFETIME_H

enum class Lifetime {
  SELF,
  ATTRIBUTE,
  ARGUMENT,
  LOCAL,
  UNDEFINED,
  UNKNOWN,
};

#endif // !_LIFETIME_H
