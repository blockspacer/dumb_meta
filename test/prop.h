#include "meta_gen/reflection.h"
#include "inspection/attribute.h"

#include <string>
#include <memory>

namespace test {
class Meta() Human {
public:
  Human() = default;

  Meta(RangeI(0, 100)) int age;

  std::string name;

  const float height = 55;

  std::shared_ptr<float> pt;

  int *remote_id;

  Meta(RangeF(.0f, 1.f)) float range;

  Meta(RangeC('a', 'z')) char code;

  MetaExclude() char secret;

  int foo() { return 42; }


  char t1;
  short int t2;
  unsigned short int t3;
  int t4;
  unsigned int t5;
  long long int t6;
  unsigned long long int t7;
  float t8;
  double t9;

private:
  int id;
};
} // namespace test
