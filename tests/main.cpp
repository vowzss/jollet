
#include <iostream>

#include "jollet/utils/string.h"

using namespace jollet;

int main() {
    std::string original = "\t\"Hello \n World\"  \t  !\b\f\rTest";
    std::cout << "Original string:\n[" << original << "]\n\n";

    std::string s1 = original;
    utils::string::sanitize(s1);
    std::cout << "sanitize:\n[" << s1 << "]\n";

    std::string s2 = original;
    utils::string::remove_quotes(s2);
    std::cout << "remove_quotes:\n[ " << s2 << "]\n";

    std::string s3 = original;
    utils::string::remove_whitespace(s3);
    std::cout << "remove_whitespace:\n[" << s3 << "]\n";

    return 0;
}