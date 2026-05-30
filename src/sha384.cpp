// sha384.cpp
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <string>
#include <openssl/evp.h>

std::string sha384_hash(const std::string& input) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha384(), nullptr);
    EVP_DigestUpdate(ctx, input.c_str(), input.size());
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

#ifdef SHA384_TEST
int main() {
    std::string test = "hello";
    std::string expected = "59e1748777448c69de6b800d7a33bbfb9ff1b463e44354c3553bcdb9c666fa90125a3c79f90397bdf5f6a13de828684f";
    std::string result = sha384_hash(test);
    assert(result == expected);
    std::cout << "Тест SHA-384 пройден.\n";
    return 0;
}
#endif
