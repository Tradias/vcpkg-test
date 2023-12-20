#include <array>
#include <limits>
#include <string>

static constexpr auto MARKER = std::numeric_limits<size_t>::max();

struct A
{
    char c;
    size_t pos{MARKER};
    A* next{};
    A* prev{};

    friend bool operator<(A lhs, A rhs) noexcept { return lhs.pos > rhs.pos; }
};

template <class T>
struct List
{
    T* head{};
    T* tail{};

    void emplace_back(T* t)
    {
        if (!tail)
        {
            head = t;
        }
        else
        {
            tail->prev = t;
        }
        auto* cur_tail = tail;
        tail = t;
        t->next = cur_tail;
        t->prev = nullptr;
    }

    T* pop_front()
    {
        auto r = head;
        if (head && head->prev)
        {
            head = head->prev;
            head->next = nullptr;
        }
        return r;
    }

    T* pop_back()
    {
        auto r = tail;
        if (tail && tail->next)
        {
            tail = tail->next;
            tail->prev = nullptr;
        }
        return r;
    }

    T* pop(T* t)
    {
        if (head == t)
        {
            return pop_front();
        }
        if (tail == t)
        {
            return pop_back();
        }
        t->next->prev = t->prev;
        t->prev->next = t->next;
        return t;
    }

    T* front() const { return head; }
};

std::string min_win(std::string input, std::string pattern)
{
    std::array<A, 256> chars{};
    const auto get_a = [&](auto c) -> decltype(auto)
    {
        return chars[(size_t)c];
    };
    for (auto&& c : pattern)
    {
        get_a(c) = {c, 0};
    }
    List<A> list;
    size_t size{};
    size_t min_distance{MARKER};
    size_t pos{};

    for (size_t cur{}; cur != input.size(); ++cur)
    {
        char c = input[cur];
        auto& a = get_a(c);
        if (a.pos == MARKER)
        {
            continue;
        }
        a.pos = cur;
        if (size == 0 || (!a.next && !a.prev))
        {
            list.emplace_back(&a);
            ++size;
        }
        if (a.prev)
        {
            auto top = list.pop(&a);
            list.emplace_back(top);
        }
        if (size == pattern.size())
        {
            auto top = list.front();
            const auto distance = cur - top->pos;
            if (distance < min_distance)
            {
                min_distance = distance;
                pos = cur;
            }
        }
    }
    if (min_distance == MARKER)
    {
        return {};
    }
    return input.substr(pos - min_distance, min_distance + 1);
}

int main()
{
    auto b = min_win("ADOBECODEBANC", "ABC");
    // auto b = min_win("ADOBECODEBANC", "AA");
    int a{};
}