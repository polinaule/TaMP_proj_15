#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <openssl/sha.h>

// Вычисляет SHA-384 хеш строки, возвращает шестнадцатеричную строку
std::string sha384_hash(const std::string& input) {
    unsigned char hash[SHA384_DIGEST_LENGTH];
    SHA384_CTX ctx;
    SHA384_Init(&ctx);
    SHA384_Update(&ctx, input.c_str(), input.size());
    SHA384_Final(hash, &ctx);

    std::stringstream ss;
    for (int i = 0; i < SHA384_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

// Unit-тест: хеш "hello" должен совпадать с эталоном
int main() {
    std::string test = "hello";
    std::string expected = "59e1748777448c69de6b800d7a33bbfb9ff1b463e44354c3553bcdb9c666fa90125a3c79f90397bdf5f6a13de828684f";
    std::string result = sha384_hash(test);
    assert(result == expected);
    std::cout << "SHA-384 test passed.\n";
    return 0;
}