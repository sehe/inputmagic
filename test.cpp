#include <iostream>
#include <functional>
#include <vector>
#include <tuple>
#include <experimental/tuple>

namespace inputmagic {

    template<typename T, typename Prompter, typename ReadPolicy>
    struct checked_input {
        using Validator = std::function<char const*(T const& v)>;
        using Output    = std::reference_wrapper<std::ostream>;

        checked_input(Prompter&& p, T& v, ReadPolicy&& r)
            : prompter_(std::move(p)), 
              reader_(std::move(r)),
              value_(v)
        { }

        template <typename Char, typename CharT>
        friend std::basic_istream<Char, CharT>& operator>>(std::basic_istream<Char, CharT>& is, checked_input&& manip) {
            return manip.do_manip(is);
        }

        template <typename Char, typename CharT>
        friend std::basic_istream<Char, CharT>& operator>>(std::basic_istream<Char, CharT>& is, checked_input& manip) {
            return manip.do_manip(is);
        }

        checked_input& flush_on_error(bool newvalue = true) { flush_on_error_ = newvalue;    return *this; } 
        checked_input& retries(unsigned newvalue = true)    { retries_ = newvalue;           return *this; } 
        checked_input& format_error(char const* newvalue)   { format_error_ = newvalue;      return *this; } 
        checked_input& output(std::ostream& newvalue)       { out_ = newvalue;               return *this; } 
        checked_input& multiple_diagnostics()               { multiple_diagnostics_ = true;  return *this; } 
        checked_input& single_diagnostics()                 { multiple_diagnostics_ = false; return *this; } 

        template <typename F>
        checked_input& validate(F&& check, char const* msg = "Value not valid") {
            validators_.emplace_back([f=std::forward<F>(check),msg](T const& v) { return f(v)? nullptr : msg; });
            return *this;
        }

      private:
        Prompter   prompter_;
        ReadPolicy reader_;
        T&         value_;

        enum { INF = -1 };

        std::vector<Validator> validators_ = {};
        bool flush_on_error_               = true;
        int retries_                       = INF;
        char const* format_error_          = "Invalid input format";
        Output out_                        = std::cout;
        bool multiple_diagnostics_         = true;

        bool infinite() const { return INF == retries_; }

        std::istream& do_manip(std::istream& is) {
            auto attempt = [this] { return infinite() || retries_ > 0; };

            while (attempt()) {
                if (!infinite())
                    retries_ -= 1;

                prompter_(out_);

                if (reader_(is, value_)) {
                    if (!run_validators(out_))
                        is.setstate(is.rdstate() | std::ios::failbit);
                    else
                        break;
                } else {
                    out_.get() << format_error_ << "\n";
                }

                if (attempt()) {
                    if (is.eof())
                        break;
                    is.clear();
                    if (flush_on_error_)
                        is.ignore(1024, '\n');
                }
            }

            return is;
        }

        bool run_validators(std::ostream& out) {
            bool ok = true;
            for (auto& validator : validators_) {
                if (auto* msg = validator(value_)) {
                    out << msg << "\n";
                    ok = false;

                    if (!multiple_diagnostics_)
                        break;
                }
            }

            return ok;
        }

    };

    struct stream_to {
        std::ostream& os_;
        template <typename V> void operator()(V const& v) const { os_ << v; }
        template <typename V, typename... Vs> void operator()(V const& v, Vs const&... vs) const { os_ << v; (*this)(vs...); }
    };

    template <typename T, typename... Msg>
        auto prompt(T& v, Msg&&... msg) {
            auto prompter = [stored=std::make_tuple(std::forward<Msg>(msg)...)](std::ostream& os) { 
                std::experimental::apply(stream_to{os}, stored);
            };
            auto reader   = [](std::istream& is, T& v) -> auto& { return is >> v; };

            return checked_input<T, decltype(prompter), decltype(reader)> {
                std::move(prompter),
                v,
                std::move(reader) 
            };
        }

    template <typename T, typename... Msg>
        auto read_line(T& v, Msg&&... msg) {
            auto prompter = [stored=std::make_tuple(std::forward<Msg>(msg)...)](std::ostream& os) { 
                std::experimental::apply(stream_to{os}, stored);
            };
            auto reader   = [](std::istream& is, std::string& v) -> auto& { return std::getline(is, v); };

            return checked_input<std::string, decltype(prompter), decltype(reader)> {
                std::move(prompter),
                v,
                std::move(reader) 
            }.flush_on_error(false);
        }
}

#include <algorithm>
using namespace std;

bool alpha_only(string const& s) {
    return all_of(s.begin(), s.end(), [](unsigned char ch) { return isalpha(ch); });
}

// can't have subsequent spaces and can't start with a space
bool correct_space(string const& s) {
    return (" " + s).find("  ") == string::npos; 
}

int main() {
    using namespace inputmagic;

    std::string lname;

    if (cin >> read_line(lname, "Input customer's lastname (previous: '", std::ref(lname), "'): ")
            .validate(alpha_only,                         "You can only input alpha here!")
            .validate([](auto& s) { return !s.empty(); }, "Last name can not be empty")
            .validate(correct_space,                      "Cannot have spaces there"))
    {
        cout << "Success: " << lname << "\n";
    }
}
