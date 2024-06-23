#include "helloWorld.pb.h"

int main()
{
    helloworld::HelloRequest request;
    return request.a().size();
}