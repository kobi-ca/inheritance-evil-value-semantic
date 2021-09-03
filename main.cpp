#include <iostream>
#include <memory>
#include <vector>

namespace {

    template <typename T>
    void print(const T& t, std::ostream& out, const std::size_t position) {
        out << std::string(position, ' ') << t << '\n';
    }
    class object final {
    public:
        template<typename T>
        explicit object(T t) : self_ (new model<T>(std::move(t))) {}

        object(const object& o) : self_(o.self_->copy_()) {}
        object(object&&) noexcept = default;
        object& operator=(const object& o) {
            object obj(o);
            *this = std::move(obj);
            return *this;
        }
        object& operator=(object&&) noexcept = default;

        friend void print(const object& o, std::ostream& out, const std::size_t position) {
            o.self_->print_(out, position);
        }
    private:
        struct base {
            virtual ~base() = default;
            [[nodiscard]] virtual base* copy_() const = 0;
            virtual void print_(std::ostream& out, std::size_t sz) const = 0;
        };

        template<typename T>
        struct model final : base {
            explicit model(T t) : data_(std::move(t)) {}
            [[nodiscard]] base* copy_() const override { return new model(*this); }
            void print_(std::ostream& out, const std::size_t position) const override {
                print(data_, out, position);
            }
            T data_;
        };
        std::unique_ptr<base> self_;
    };

    using document = std::vector<object>;

    void print(const document& doc, std::ostream& out, const std::size_t position) {
        out << std::string(position, ' ') << "<document>" << '\n';
        for(const auto& e : doc) {
            print(e, out, position + 2);
        }
        out << std::string(position, ' ') << "</document>" << '\n';
    }

    // example object with print function
    struct example_object final {
        int id_{};
        explicit example_object(const int id) : id_(id) {}
        void print(std::ostream& out, const std::size_t position) const {
            out << "example_object:" << id_;
        }
    };

    void print(const example_object& obj, std::ostream& out, const std::size_t position) {
        out << std::string(position, ' ');
        obj.print(out, position);
        out << '\n';
    }
}

int main() {
    document  doc;
    doc.emplace_back(0);
    using namespace std::string_literals;
    doc.emplace_back("hello"s);
    doc.emplace_back(2);
    doc.emplace_back(example_object{0});
    print(doc, std::cout,  0);
    return 0;
}
