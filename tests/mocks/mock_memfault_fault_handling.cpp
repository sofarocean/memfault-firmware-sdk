//! @file
//!
//! Mock implementation of memfault fault handling functions

#include "CppUTestExt/MockSupport.h"
#include "memfault/panics/fault_handling.h"

void memfault_fault_handling_assert_extra(void *pc, void *lr, sMemfaultAssertInfo *extra_info) {
  mock()
    .actualCall(__func__)
    .withPointerParameter("pc", pc)
    .withPointerParameter("lr", lr)
    .withParameterOfType("sMemfaultAssertInfo", "extra_info", extra_info);
}
