#include "google/protobuf/descriptor.upb.h"
#include "helloWorld.upb.h"
#include "helloWorld.upbdefs.h"

#include <upb/mem/arena.hpp>
// #include <utf8_validity.h>

#include <cassert>

void func();

int main()
{
    upb::Arena a;
    auto r = helloworld_HelloRequest_new(a.ptr());
    upb_DefPool* pool = upb_DefPool_New();
    auto def = helloworld_HelloRequest_getmsgdef(pool);
    auto field = upb_MessageDef_FindFieldByNumber(def, 1);
    auto type = upb_FieldDef_CType(field);
    assert(kUpb_CType_String == type);
    // utf8_range::IsStructurallyValid("test");
    return 0;
}