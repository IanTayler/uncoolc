#ifndef _LIFETIME_H
#define _LIFETIME_H

enum class Lifetime {
  ATTRIBUTE,
  ARGUMENT,
  LOCAL,
  UNDEFINED,
  UNKNOWN,
};

#endif // !_LIFETIME_H
