#pragma once

#include <boost/asio/async_result.hpp>
#include <boost/asio/execution.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <opentelemetry/trace/semantic_conventions.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/tracer.h>
#include <grpcpp/grpcpp.h>
#include <agrpc/asio_grpc.hpp>

AGRPC_NAMESPACE_BEGIN()

namespace detail
{
template <class T>
constexpr auto as_string_view(const T& string)
{
    return opentelemetry::nostd::string_view(string.begin(), string.size());
}
}

template <auto PrepareAsync, class CompletionToken>
class TracerBinder
{
  public:
    template <class... Args>
    TracerBinder(opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer>&& tracer, Args&&... args)
        : tracer_(std::move(tracer)), token(std::forward<Args>(args)...)
    {
    }

    auto& get() noexcept { return token; }

    const auto& get() const noexcept { return token; }

    auto& tracer() noexcept { return tracer_; }

  private:
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer_;
    CompletionToken token;
};

// template <class CompletionToken>
// TracerBinder(opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer>, CompletionToken&&)
//     -> TracerBinder<std::remove_cvref_t<CompletionToken>>;

template <auto PrepareAsync, class CompletionToken>
inline auto bind_tracer(opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer, CompletionToken&& token)
{
    return TracerBinder<PrepareAsync, std::remove_cvref_t<CompletionToken>>{std::move(tracer),
                                                                            std::forward<CompletionToken>(token)};
}

// Implementation details
namespace detail
{
template <class Handler>
class TracerBinderCompletionHandler
{
  public:
    explicit TracerBinderCompletionHandler(opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span,
                                           Handler&& handler)
        : span(std::move(span)), handler(std::move(handler))
    {
    }

    const auto& get() const noexcept { return handler; }

    void operator()(grpc::Status&& status) &&
    {
        if (status.ok())
        {
            span->SetStatus(opentelemetry::trace::StatusCode::kOk);
        }
        else
        {
            span->SetStatus(opentelemetry::trace::StatusCode::kError);
        }
        span->SetAttribute(opentelemetry::trace::SemanticConventions::kRpcGrpcStatusCode, status.error_code());
        span->End();
        std::move(handler)(std::move(status));
    }

  private:
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span;
    Handler handler;
};

template <class Handler>
TracerBinderCompletionHandler(opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>, Handler)
    -> TracerBinderCompletionHandler<Handler>;

template <auto PrepareAsync, class Initiation>
struct TracerBinderAsyncResultInitWrapper
{
    template <class Handler, class... Args>
    void operator()(Handler&& handler, Args&&... args)
    {
        constexpr auto service_name = agrpc::detail::CLIENT_SERVICE_NAME_V<PrepareAsync>;
        constexpr auto method_name = agrpc::detail::CLIENT_METHOD_NAME_V<PrepareAsync>;
        opentelemetry::trace::StartSpanOptions options;
        options.kind = opentelemetry::trace::SpanKind::kClient;
        std::string span_name = "GreeterClient/Greet";
        auto span =
            tracer->StartSpan(span_name,
                              {
                                  {opentelemetry::trace::SemanticConventions::kRpcSystem, "grpc"},
                                  {opentelemetry::trace::SemanticConventions::kRpcService, detail::as_string_view(service_name)},
                                  {opentelemetry::trace::SemanticConventions::kRpcMethod, detail::as_string_view(method_name)},
                                  //  {opentelemetry::trace::SemanticConventions::NET_PEER_IP, ip},
                                  //  {opentelemetry::trace::SemanticConventions::NET_PEER_PORT, port}
                              },
                              options);
        std::move(initiation)(TracerBinderCompletionHandler(std::move(span), std::forward<Handler>(handler)),
                              std::forward<Args>(args)...);
    }

    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer;
    Initiation initiation;
};
}  // namespace detail

AGRPC_NAMESPACE_END

template <auto PrepareAsync, class CompletionToken, class Signature>
class agrpc::asio::async_result<agrpc::TracerBinder<PrepareAsync, CompletionToken>, Signature>
{
  public:
    template <class Initiation, class BoundCompletionToken, class... Args>
    static decltype(auto) initiate(Initiation&& initiation, BoundCompletionToken&& token, Args&&... args)
    {
        return asio::async_initiate<CompletionToken, Signature>(
            agrpc::detail::TracerBinderAsyncResultInitWrapper<PrepareAsync, std::remove_cvref_t<Initiation>>{
                std::move(token.tracer()), std::forward<Initiation>(initiation)},
            token.get(), std::forward<Args>(args)...);
    }
};

template <template <class, class> class Associator, class Handler, class DefaultCandidate>
struct agrpc::asio::associator<Associator, agrpc::detail::TracerBinderCompletionHandler<Handler>, DefaultCandidate>
{
    using type = typename Associator<Handler, DefaultCandidate>::type;

    static type get(const agrpc::detail::TracerBinderCompletionHandler<Handler>& b,
                    const DefaultCandidate& c = DefaultCandidate()) noexcept
    {
        return Associator<Handler, DefaultCandidate>::get(b.get(), c);
    }
};
