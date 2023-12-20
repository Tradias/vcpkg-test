// Make sure to include GRPC headers first because otherwise Abseil may create
// ambiguity with `nostd::variant` if compiled with Visual Studio 2015. Other
// modern compilers are unaffected.
#include "bind_tracer.hpp"
#include "messages.grpc.pb.h"
#include "tracer_common.h"

#include <agrpc/asio_grpc.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <grpcpp/grpcpp.h>
#include <opentelemetry/trace/semantic_conventions.h>

#include <iostream>
#include <memory>
#include <string>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using grpc_example::Greeter;
using grpc_example::GreetRequest;
using grpc_example::GreetResponse;

namespace
{
namespace asio = boost::asio;
namespace context = opentelemetry::context;
using namespace opentelemetry::trace;

class GreeterClient
{
  public:
    agrpc::GrpcContext& grpc_context;
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer;

    GreeterClient(std::shared_ptr<Channel> channel, agrpc::GrpcContext& grpc_context,
                  opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer)
        : stub_(Greeter::NewStub(channel)), grpc_context(grpc_context), tracer(tracer)
    {
    }

    asio::awaitable<GreetResponse> Greet(std::string ip, uint16_t port)
    {
        // Build gRPC Context objects and protobuf message containers
        GreetRequest request;
        GreetResponse response;
        ClientContext context;
        request.set_request("Nice to meet you!");

        // inject current context to grpc metadata
        auto current_ctx = context::RuntimeContext::GetCurrent();
        GrpcClientCarrier carrier(&context);
        auto prop = context::propagation::GlobalTextMapPropagator::GetGlobalPropagator();
        prop->Inject(carrier, current_ctx);

        // Send request to server
        Status status = co_await agrpc::ClientRPC<&Greeter::Stub::PrepareAsyncGreet>::request(
            grpc_context, *stub_, context, request, response,
            agrpc::bind_tracer<&Greeter::Stub::PrepareAsyncGreet>(tracer, asio::use_awaitable));

        co_return response;
    }

  private:
    std::unique_ptr<Greeter::Stub> stub_;
};

void RunClient(uint16_t port)
{
    agrpc::GrpcContext grpc_context{std::make_unique<grpc::CompletionQueue>()};
    auto channel = grpc::CreateChannel("0.0.0.0:" + std::to_string(port), grpc::InsecureChannelCredentials());
    auto tracer = get_tracer("grpc", grpc::Version());
    GreeterClient greeter(channel, grpc_context, std::move(tracer));
    auto future = asio::co_spawn(grpc_context, greeter.Greet("0.0.0.0", port), asio::use_future);
    grpc_context.run();
    future.get();
}
}  // namespace

int main(int argc, char** argv)
{
    initTracer();

    // set global propagator
    context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(
        opentelemetry::nostd::shared_ptr<context::propagation::TextMapPropagator>(new propagation::HttpTraceContext()));
    constexpr uint16_t default_port = 8800;
    uint16_t port;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    else
    {
        port = default_port;
    }
    RunClient(port);
    return 0;
}