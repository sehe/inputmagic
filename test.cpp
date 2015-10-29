#include <iostream>
#include <limits>

namespace inputmagic {

    
    template<typename T, typename Prompter>
    struct checked_input {
        Prompter prompter_;
        T& val_;

        checked_input(Prompter&& p, T& v) 
            : prompter_(std::move(p)), val_(v)
        { }

        bool flush_on_error_ = true;
        bool infinite_       = false;
        unsigned retries_    = 3;

        template <typename Char, typename CharT> // , typename T_, typename Prompter_>
        friend std::basic_istream<Char, CharT>& operator>>(std::basic_istream<Char, CharT>& is, checked_input<T, Prompter>&& manip) {
            while (manip.infinite_ || manip.retries_) {
                manip.retries_ -= 1;

                manip.prompter_(std::cout); // TODO inject stream

                if (is >> manip.val_ /* TODO validation */)
                    break;

                if (manip.flush_on_error_) {
                    is.clear();
                    is.ignore(std::numeric_limits<std::streampos>::max(), '\n');
                }
            }

            return is;
        }
    };

    template <typename Msg, typename T>
        auto prompt(Msg&& msg, T& v) {
            auto prompter = [msg=std::forward<Msg>(msg)](std::ostream& os) { os << msg; };
            return checked_input<T, decltype(prompter)> { std::move(prompter), v };
        }
}

int main() {
    using namespace inputmagic;    

    int i;
    std::cin >> prompt("enter an integer: ", i);

    std::cout << "\nRESULT:" << i;
}
