
#include <rttr/registration>

#include "D:\Projects\dumb_meta\test\prop.h"
RTTR_REGISTRATION
{
 using namespace rttr;
// D:\Projects\dumb_meta\test\prop.h
{
    registration::class_<test::Human>("test::Human")
        
.property("age", &test::Human::age, registration::public_access)
( metadata("RangeI", RangeI { 0, 100 }) )
.property_readonly("height", &test::Human::height, registration::public_access)

.property("name", &test::Human::name, registration::public_access)

.property("pt", &test::Human::pt, registration::public_access)

.property("remote_id", &test::Human::remote_id, registration::public_access)

.property("range", &test::Human::range, registration::public_access)
( metadata("RangeF", RangeF { .0f, 1.f }) )
.property("code", &test::Human::code, registration::public_access)
( metadata("RangeC", RangeC { 'a', 'z' }) )
.property("t1", &test::Human::t1, registration::public_access)

.property("t2", &test::Human::t2, registration::public_access)

.property("t3", &test::Human::t3, registration::public_access)

.property("t4", &test::Human::t4, registration::public_access)

.property("t5", &test::Human::t5, registration::public_access)

.property("t6", &test::Human::t6, registration::public_access)

.property("t7", &test::Human::t7, registration::public_access)

.property("t8", &test::Human::t8, registration::public_access)

.property("t9", &test::Human::t9, registration::public_access)

        
.constructor<>(registration::public_access)

.method("foo", &test::Human::foo, registration::public_access)

        ;
}
}