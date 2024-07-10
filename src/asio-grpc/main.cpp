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

#include "helloWorld.grpc.pb.h"

#include <agrpc/asio_grpc.hpp>
#include <grpcpp/server_builder.h>
// #include <grpcpp/ext/proto_server_reflection_plugin.h>

#include <asio/detached.hpp>

#include <vector>
#include <thread>

//
// #include <boost/asio/bind_executor.hpp>

//
// #include <unifex/just.hpp>
// #include <unifex/let_value.hpp>
// #include <unifex/sync_wait.hpp>
// #include <unifex/then.hpp>
// #include <unifex/when_all.hpp>

int main(int argc, const char** argv)
{
    // grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    const auto thread_count = argc >= 2 ? std::stoi(argv[1]) : 1;
    std::string host{"localhost:50051"};

    // namespace asio = boost::asio;

    grpc::ServerBuilder builder;
    std::unique_ptr<grpc::Server> server;
    helloworld::Greeter::AsyncService service;
    builder.AddListeningPort(std::string{host}, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
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
                auto e = grpc_context.get_executor();
                auto a = grpc_context.get_allocator();
                // boost::container::pmr::polymorphic_allocator<int>c{};
                // static_assert(std::is_same_v<std::pmr::unsynchronized_pool_resource,
                //                              agrpc::detail::GrpcContextLocalMemoryResource>);
                using RPC = agrpc::ServerRPC<&helloworld::Greeter::AsyncService::RequestSayHello>;
                agrpc::register_callback_rpc_handler<RPC>(
                    grpc_context, service,
                    [&](RPC::Ptr ptr, RPC::Request& request)
                    {
                        helloworld::HelloReply response;
                        response.set_message("Hello " + request.name());
                        auto& rpc = *ptr;
                        rpc.finish(response, grpc::Status::OK, [p = std::move(ptr)](bool) {});
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
    for (auto& thread : threads)
    {
        thread.join();
    }
}
