#include <cstddef>
#include <functional>
#include <variant>

//
#include <cassert>
#include <memory>
#include <string>
#include <string_view>

template <class Stub, class Request, class Responder>
using ClientUnaryRequest = std::unique_ptr<Responder> (Stub::*)(const double& request, void* cq);

template <class Stub, class Responder, class Response>
using PrepareAsyncClientClientStreamingRequest = std::unique_ptr<Responder> (Stub::*)(Response* cq);

template <class>
class ClientAsyncReaderInterface
{
};

template <class T>
class ClientAsyncReader : public ClientAsyncReaderInterface<T>
{
};

template <auto PrepareAsync, class = void>
inline constexpr auto RPC_TYPE = 1;

template <class Stub, class Request, class Response,
          std::unique_ptr<ClientAsyncReader<Response>> (Stub::*PrepareAsync)(const Request&, void*)>
inline constexpr auto RPC_TYPE<PrepareAsync, std::void_t<decltype(PrepareAsync)>> = 2;

template <class Stub, class Request, class Response,
          std::unique_ptr<ClientAsyncReaderInterface<Response>> (Stub::*PrepareAsync)(const Request&, void*)>
inline constexpr auto RPC_TYPE<PrepareAsync, std::void_t<decltype(PrepareAsync)>> = 2;

template <class Stub, class Request, class Response, template <class> class Writer,
          PrepareAsyncClientClientStreamingRequest<Stub, Writer<Request>, Response> PrepareAsync>
inline constexpr auto RPC_TYPE<PrepareAsync, std::void_t<decltype(PrepareAsync)>> = 3;

class Stub final
{
  public:
    std::unique_ptr<ClientAsyncReader<int>> PrepareAsyncServerStreaming(const double& request, void* cq);
};

template <auto PrepareAsync, int = RPC_TYPE<PrepareAsync>>
class RPC;

int main() {
    using R = RPC<&Stub::PrepareAsyncServerStreaming>;
}