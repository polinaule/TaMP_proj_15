#include <gtest/gtest.h>
#include <cmath>
#include <cstring>
#include <functional>

// Прототипы функций (объявления)
double chord_method(double a, double b, double eps, std::function<double(double)> f);
void des_encrypt_block(const uint8_t input[8], uint8_t output[8], const uint8_t key[8]);
void des_decrypt_block(const uint8_t input[8], uint8_t output[8], const uint8_t key[8]);
std::string sha384_hash(const std::string& input);

// Тест метода хорд
TEST(ChordTest, RootOfSquareMinusFour) {
    auto f = [](double x) { return x * x - 4; };
    double root = chord_method(1.0, 3.0, 1e-6, f);
    EXPECT_NEAR(root, 2.0, 1e-5);
}

TEST(ChordTest, NoRootOnSegment) {
    auto f = [](double x) { return x * x + 1; };
    double root = chord_method(0.0, 2.0, 1e-6, f);
    EXPECT_TRUE(std::isnan(root));
}

// Тест DES
TEST(DESTest, EncryptDecrypt) {
    uint8_t plain[8] = { 1,2,3,4,5,6,7,8 };
    uint8_t key[8] = { 8,7,6,5,4,3,2,1 };
    uint8_t cipher[8], decrypted[8];
    des_encrypt_block(plain, cipher, key);
    des_decrypt_block(cipher, decrypted, key);
    EXPECT_EQ(memcmp(plain, decrypted, 8), 0);
}

// Тест SHA-384
TEST(SHA384Test, KnownHash) {
    std::string input = "hello";
    std::string expected = "59e1748777448c69de6b800d7a33bbfb9ff1b463e44354c3553bcdb9c666fa90125a3c79f90397bdf5f6a13de828684f";
    std::string result = sha384_hash(input);
    EXPECT_EQ(result, expected);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}