#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

class BigInteger {
  public:
    enum sign_type { POSITIVE, ZERO, NEGATIVE };
    BigInteger();
    BigInteger(const std::string& s);
    BigInteger(long long n);
    sign_type sign() const;
    void changeSign();
    std::string toString() const;
    explicit operator bool() const;
    BigInteger& operator+=(const BigInteger& b);
    BigInteger& operator-=(const BigInteger& b);
    BigInteger& operator*=(const BigInteger& b);
    BigInteger& operator/=(const BigInteger& b);
    BigInteger& operator%=(const BigInteger& b);
    BigInteger operator-() const;
    BigInteger& operator++();
    BigInteger operator++(int);
    BigInteger& operator--();
    BigInteger operator--(int);
    friend bool operator<(const BigInteger& b1, const BigInteger& b2);
    friend bool operator==(const BigInteger& b1, const BigInteger& b2);

  private:
    BigInteger& operation(const BigInteger& b, bool type);
    BigInteger& unsignedSubtraction(const BigInteger& b);
    BigInteger& unsignedSum(const BigInteger& b);
    bool unsignedOrder(const BigInteger& b) const;
    BigInteger& divMod(const BigInteger& b, bool divmod);
    static long long toNumber(sign_type sign);
    static sign_type toSign(long long number);
    sign_type type_ = ZERO;
    std::vector<long long> digits_;
    static const long long BASE_ = 1000000000;
    static const long long BASE_LENGTH_ = 9;
};

bool operator<(const BigInteger& b1, const BigInteger& b2);
bool operator>(const BigInteger& b1, const BigInteger& b2);
bool operator==(const BigInteger& b1, const BigInteger& b2);
bool operator!=(const BigInteger& b1, const BigInteger& b2);
bool operator>=(const BigInteger& b1, const BigInteger& b2);
bool operator<=(const BigInteger& b1, const BigInteger& b2);
BigInteger operator+(const BigInteger& a, const BigInteger& b);
BigInteger operator-(const BigInteger& a, const BigInteger& b);
BigInteger operator*(const BigInteger& a, const BigInteger& b);
BigInteger operator/(const BigInteger& a, const BigInteger& b);
BigInteger operator%(const BigInteger& a, const BigInteger& b);
std::istream& operator>>(std::istream& in, BigInteger& b);
std::ostream& operator<<(std::ostream& out, const BigInteger& b);
BigInteger operator""_bi(unsigned long long n);
BigInteger operator""_bi(const char* s, size_t size);

class Rational {
  private:
    BigInteger denominator_;
    BigInteger numerator_;
    void make_rational();

  public:
    Rational();
    Rational(const BigInteger& b);
    Rational(int n);
    Rational& operator+=(const Rational& b);
    Rational& operator-=(const Rational& b);
    Rational& operator*=(const Rational& b);
    Rational& operator/=(const Rational& b);
    Rational operator-() const;
    std::string toString() const;
    std::string asDecimal(size_t precision = 0) const;
    explicit operator double() const;
    friend bool operator==(const Rational& b1, const Rational& b2);
    friend bool operator<(const Rational& b1, const Rational& b2);
};

Rational operator+(const Rational& a, const Rational& b);
Rational operator-(const Rational& a, const Rational& b);
Rational operator*(const Rational& a, const Rational& b);
Rational operator/(const Rational& a, const Rational& b);
bool operator<(const Rational& b1, const Rational& b2);
bool operator>(const Rational& b1, const Rational& b2);
bool operator==(const Rational& b1, const Rational& b2);
bool operator!=(const Rational& b1, const Rational& b2);
bool operator>=(const Rational& b1, const Rational& b2);
bool operator<=(const Rational& r1, const Rational& b2);
