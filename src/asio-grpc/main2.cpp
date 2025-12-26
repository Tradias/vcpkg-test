// Copyright 2021 Dennis Hezel
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "greeter.grpc.fb.h"

#include <agrpc/asio_grpc.hpp>
#include <agrpc/register_awaitable_rpc_handler.hpp>
// #include <agrpc/health_check_service.hpp>
#include <asio/detached.hpp>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/server_builder.h>

#include <thread>
#include <vector>

//
// #include <boost/asio/bind_executor.hpp>

//
// #include <unifex/just.hpp>
// #include <unifex/let_value.hpp>
// #include <unifex/sync_wait.hpp>
// #include <unifex/then.hpp>
// #include <unifex/when_all.hpp>

class GreeterClient
{
  public:
    GreeterClient(std::shared_ptr<grpc::Channel> channel) : stub_(Greeter::NewStub(channel)) {}

    std::string SayHello(const std::string& name)
    {
        flatbuffers::grpc::MessageBuilder mb;
        auto name_offset = mb.CreateString(name);
        auto request_offset = CreateHelloRequest(mb, name_offset);
        mb.Finish(request_offset);
        auto request_msg = mb.ReleaseMessage<HelloRequest>();

        flatbuffers::grpc::Message<HelloReply> response_msg;

        grpc::ClientContext context;

        auto status = stub_->SayHello(&context, request_msg, &response_msg);
        if (status.ok())
        {
            const HelloReply* response = response_msg.GetRoot();
            return response->message()->str();
        }
        else
        {
            std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
            return "RPC failed";
        }
    }

    void SayManyHellos(const std::string& name, int num_greetings, std::function<void(const std::string&)> callback)
    {
        flatbuffers::grpc::MessageBuilder mb;
        auto name_offset = mb.CreateString(name);
        auto request_offset = CreateManyHellosRequest(mb, name_offset, num_greetings);
        mb.Finish(request_offset);
        auto request_msg = mb.ReleaseMessage<ManyHellosRequest>();

        flatbuffers::grpc::Message<HelloReply> response_msg;

        grpc::ClientContext context;

        auto stream = stub_->SayManyHellos(&context, request_msg);
        while (stream->Read(&response_msg))
        {
            const HelloReply* response = response_msg.GetRoot();
            callback(response->message()->str());
        }
        auto status = stream->Finish();
        if (!status.ok())
        {
            std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
            callback("RPC failed");
        }
    }

  private:
    std::unique_ptr<Greeter::Stub> stub_;
};

int client_main()
{
    std::string server_address("localhost:50051");

    auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
    GreeterClient greeter(channel);

    std::string name("world");

    std::string message = greeter.SayHello(name);
    std::cerr << "Greeter received: " << message << std::endl;

    int num_greetings = 10;
    greeter.SayManyHellos(name, num_greetings,
                          [](const std::string& message)
                          {
                              std::cerr << "Greeter received: " << message << std::endl;
                          });

    return 0;
}

int main(int argc, const char** argv)
{
    // grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    const auto thread_count = argc >= 2 ? std::stoi(argv[1]) : 1;
    std::string host{"localhost:50051"};

    // namespace asio = boost::asio;

    grpc::ServerBuilder builder;
    std::unique_ptr<grpc::Server> server;
    Greeter::AsyncService service;
    builder.AddListeningPort(std::string{host}, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    // agrpc::add_health_check_service(builder);
    std::vector<std::unique_ptr<grpc::ServerCompletionQueue>> queues;
    for (size_t i = 0; i < thread_count; ++i)
    {
        queues.emplace_back(builder.AddCompletionQueue());
    }
    server = builder.BuildAndStart();

    std::vector<std::thread> threads;
    for (size_t i = 0; i < thread_count; ++i)
    {
        threads.emplace_back(
            [&, i]
            {
                agrpc::GrpcContext grpc_context{std::move(queues[i])};
                if (i == 0)
                {
                    // agrpc::start_health_check_service(*server, grpc_context);
                }
                using RPC = agrpc::ServerRPC<&Greeter::AsyncService::RequestSayHello>;
                auto a = RPC::service_name();
                auto b = RPC::method_name();
                agrpc::register_callback_rpc_handler<RPC>(
                    grpc_context, service,
                    [&](RPC::Ptr ptr, RPC::Request& request_msg)
                    {
                        flatbuffers::grpc::MessageBuilder mb_;

                        // We call GetRoot to "parse" the message. Verification is already
                        // performed by default. See the notes below for more details.
                        const HelloRequest* request = request_msg.GetRoot();

                        // Fields are retrieved as usual with FlatBuffers
                        const std::string& name = request->name()->str();

                        // `flatbuffers::grpc::MessageBuilder` is a `FlatBufferBuilder` with a
                        // special allocator for efficient gRPC buffer transfer, but otherwise
                        // usage is the same as usual.
                        auto msg_offset = mb_.CreateString("Hello, " + name);
                        auto hello_offset = CreateHelloReply(mb_, msg_offset);
                        mb_.Finish(hello_offset);

                        // The `ReleaseMessage<T>()` function detaches the message from the
                        // builder, so we can transfer the resopnse to gRPC while simultaneously
                        // detaching that memory buffer from the builer.
                        auto response_msg = mb_.ReleaseMessage<HelloReply>();
                        assert(response_msg.Verify());

                        auto& rpc = *ptr;
                        rpc.finish(response_msg, grpc::Status::OK, [&, p = std::move(ptr)](auto&& ok) {});
                    },
                    asio::detached);
                using RPC2 = agrpc::ServerRPC<&Greeter::AsyncService::RequestSayManyHellos>;
                agrpc::register_awaitable_rpc_handler<RPC2>(
                    grpc_context, service,
                    [&](RPC2& rpc, RPC2::Request& request_msg) -> asio::awaitable<void>
                    {
                        flatbuffers::grpc::MessageBuilder mb_;
                        // The streaming usage below is simply a combination of standard gRPC
                        // streaming with the FlatBuffers usage shown above.
                        const ManyHellosRequest* request = request_msg.GetRoot();
                        const std::string& name = request->name()->str();
                        int num_greetings = request->num_greetings();

                        for (int i = 0; i < num_greetings; i++)
                        {
                            auto msg_offset = mb_.CreateString("Many hellos, " + name);
                            auto hello_offset = CreateHelloReply(mb_, msg_offset);
                            mb_.Finish(hello_offset);
                            co_await rpc.write(mb_.ReleaseMessage<HelloReply>());
                        }

                        co_await rpc.finish(grpc::Status::OK);
                    },
                    asio::detached);
                grpc_context.run();
                // unifex::sync_wait(unifex::when_all(
                //    agrpc::repeatedly_request(
                //        &helloworld::Greeter::AsyncService::RequestSayHello, service,
                //        [&](auto&&, auto&& request, auto&& writer)
                //        {
                //            return unifex::let_value(unifex::just(helloworld::HelloReply{}),
                //                                     [&](auto& response)
                //                                     {
                //                                         response.set_message("Hello " + request.name());
                //                                         return agrpc::finish(writer, response, grpc::Status::OK,
                //                                                              agrpc::use_sender(grpc_context));
                //                                     });
                //        },
                //        agrpc::use_sender(grpc_context)),
                //    unifex::then(unifex::just(),
                //                 [&]
                //                 {
                //                     grpc_context.run();
                //                 })));
            });
    }
    std::thread t{[&]
                  {
                      client_main();
                  }};
    for (auto& thread : threads)
    {
        thread.join();
    }
    t.join();
}
