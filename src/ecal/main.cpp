#include <ecal/ecal.h>

int main()
{
    eCAL::CServiceClient c{"test"};
    c.Call("test", "aasd", 1);
}