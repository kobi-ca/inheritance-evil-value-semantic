#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <array>

namespace {

    template <typename T>
    void print(const T& t, std::ostream& out, const std::size_t position) {
        out << std::string(position, ' ') << t << '\n';
    }

    // this is needed to catch non T* ones. More overloads can added here
    template <typename T>
    void inject(T) {}

    template <typename T>
    void inject(T* params) {
        params[0] = std::byte(0xFF);
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

        friend void inject(const object& o) {
            o.self_->inject_();
        }
    private:
        struct base {
            virtual ~base() = default;
            [[nodiscard]] virtual base* copy_() const = 0;
            virtual void print_(std::ostream& out, std::size_t sz) const = 0;
            virtual void inject_() const = 0;
        };

        template<typename T/*, typename P*/>
        struct model final : base {
            explicit model(T t) : data_(std::move(t)) {}
            [[nodiscard]] base* copy_() const override { return new model(*this); }
            void print_(std::ostream& out, const std::size_t position) const override {
                print(data_, out, position);
            }
            void inject_() const override {
                inject(data_);
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

    class Database final  {
    public:
        enum class trigger_types {
            type1,
            type2,
        };
    private:
        std::map<trigger_types, object> database_;
    public:
        [[nodiscard]]
        static Database& instance() {
            static Database database;
            return database;
        }
        void install(const trigger_types tt, object obj) {
            // override if exists
            database_.insert(std::make_pair(tt, std::move(obj)));
        }
        [[nodiscard]]
        const object& get(const trigger_types tt) const {
            return database_.at(tt);
        }
    };

    class TriggableExample final {
    private:
        static inline constexpr auto NUM_BYTES = 100ULL;
        std::array<std::byte, NUM_BYTES> buffer_{};
    public:
        void init(){
            auto& db = Database::instance();
            db.install(Database::trigger_types::type1, object(buffer_.begin()));
        }
        void do_something() {
            const auto& db = Database::instance();
            const object& o = db.get(Database::trigger_types::type1);
            inject(o); // this will change [0] of the buffer to be 0xFF (was 0x00 initially)
        }
    };

}

int main() {
    document  doc;
    doc.emplace_back(0);
    using namespace std::string_literals;
    doc.emplace_back("hello"s);
    doc.emplace_back(2);
    doc.emplace_back(example_object{0});
    print(doc, std::cout,  0);

    TriggableExample te;
    te.init();
    te.do_something();

    return 0;
}
