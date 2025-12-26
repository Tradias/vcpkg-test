#include "helloWorld.pb.h"

int main()
{
    std::vector<int> v;
    v.emplace_back(42);
    helloworld::HelloRequest request;
    request.add_rep_ptr()->set_message("test");
    request.mutable_rep_ptr()->erase(request.rep_ptr().begin());
    request.add_rep_ptr()->set_message("2");
    request.mutable_rep_ptr()->erase(request.rep_ptr().begin());
    return request.map_size();
}