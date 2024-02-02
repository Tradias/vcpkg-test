#include "lib.h"

#include "lib_export.h"

#include <array>
#include <oneapi/tbb.h>

namespace lib
{
void a()
{
    tbb::global_control g(tbb::global_control::max_allowed_parallelism, 2);
    tbb::parallel_for_each(std::array{0, 1, 2}, [](auto) {});
    std::array c{0, 1, 2};
    tbb::parallel_sort(c);
}
}