#include "biginteger.h"

long long BigInteger::toNumber(sign_type sign) {
    if (sign == POSITIVE) {
        return 1;
    }
    if (sign == ZERO) {
        return 0;
    }
    return -1;
}

BigInteger::sign_type BigInteger::toSign(long long number) {
    if (number > 0) {
        return POSITIVE;
    }
    if (number == 0) {
        return ZERO;
    }
    return NEGATIVE;
}

BigInteger::sign_type BigInteger::sign() const {
    return type_;
}

void BigInteger::changeSign() {
    type_ = toSign(-1 * toNumber(sign()));
}

BigInteger::BigInteger() = default;

BigInteger::BigInteger(const std::string& s)
    : type_(s.empty() || s == "0" || s == "-0"
                ? ZERO
                : (s[0] == '-' ? NEGATIVE : POSITIVE)) {
    if (sign() == ZERO) {
        digits_.push_back(0);
        return;
    }
    size_t i;
    size_t shift = 0;
    if (s[0] == '-') {
        shift = 1;
    }
    for (i = s.size(); i >= BASE_LENGTH_ + shift; i -= BASE_LENGTH_) {
        digits_.push_back(std::stoll(s.substr(i - BASE_LENGTH_, BASE_LENGTH_)));
    }
    if (i > shift) {
        long long temp = std::stoll(s.substr(shift, i - shift));
        digits_.push_back(temp);
    }
}

BigInteger::BigInteger(long long n) : BigInteger(std::to_string(n)) {}

std::string BigInteger::toString() const {
    std::string s;
    if (sign() == NEGATIVE) {
        s.push_back('-');
    }
    s += std::to_string(digits_.back());
    for (size_t i = digits_.size() - 1; i > 0; --i) {
        std::string temp = std::to_string(digits_[i - 1]);
        size_t tmp = temp.size();
        while (tmp < BASE_LENGTH_) {
            s.push_back('0');
            ++tmp;
        }
        s += temp;
    }
    return s;
}

BigInteger::operator bool() const {
    return sign() != ZERO;
}

BigInteger& BigInteger::unsignedSubtraction(const BigInteger& b) {
    long long carry = 0;
    for (size_t i = 0; i < b.digits_.size() || carry != 0; ++i) {
        digits_[i] -= carry + (i < b.digits_.size() ? b.digits_[i] : 0);
        carry = digits_[i] < 0 ? 1 : 0;
        if (carry != 0) {
            digits_[i] += BASE_;
        }
    }
    while (digits_.size() > 1 && digits_.back() == 0) {
        digits_.pop_back();
    }
    if (digits_.size() == 1 && digits_[0] == 0) {
        type_ = ZERO;
    }
    return *this;
}

BigInteger& BigInteger::unsignedSum(const BigInteger& b) {
    long long carry = 0;
    for (size_t i = 0;
         i < std::max(digits_.size(), b.digits_.size()) || carry != 0; ++i) {
        if (i == digits_.size()) {
            digits_.push_back(0);
        }
        digits_[i] += carry + (i < b.digits_.size() ? b.digits_[i] : 0);
        carry = digits_[i] >= BASE_ ? 1 : 0;
        if (carry != 0) {
            digits_[i] -= BASE_;
        }
    }
    return *this;
}

bool BigInteger::unsignedOrder(const BigInteger& b) const {
    if (digits_.size() != b.digits_.size()) {
        return (digits_.size() < b.digits_.size());
    }
    for (size_t i = digits_.size(); i > 0; --i) {
        if (digits_[i - 1] != b.digits_[i - 1]) {
            return (digits_[i - 1] < b.digits_[i - 1]);
        }
    }
    return false;
}

BigInteger& BigInteger::operation(const BigInteger& b, bool type) {
    if (b.sign() == BigInteger::ZERO) {
        return *this;
    }
    if (sign() == BigInteger::ZERO) {
        return *this = type ? -b : b;
    }
    if ((sign() == b.sign()) ^ type) {
        return unsignedSum(b);
    }
    if (unsignedOrder(b)) {
        return *this = type ? (-b + *this) : (b + *this);
    }
    return unsignedSubtraction(b);
}

BigInteger& BigInteger::operator+=(const BigInteger& b) {
    return operation(b, false);
}

BigInteger& BigInteger::operator-=(const BigInteger& b) {
    return operation(b, true);
}

BigInteger& BigInteger::operator*=(const BigInteger& b) {
    BigInteger c;
    if (sign() == ZERO || b.sign() == ZERO) {
        c.type_ = ZERO;
    } else if (sign() != b.sign()) {
        c.type_ = NEGATIVE;
    } else {
        c.type_ = POSITIVE;
    }
    c.digits_.resize(digits_.size() + b.digits_.size());
    for (size_t i = 0; i < digits_.size(); ++i) {
        long long carry = 0;
        for (size_t j = 0; j < b.digits_.size() || carry != 0; ++j) {
            long long cur =
                c.digits_[i + j] +
                digits_[i] * (j < b.digits_.size() ? b.digits_[j] : 0) + carry;
            c.digits_[i + j] = cur % BASE_;
            carry = cur / BASE_;
        }
    }
    while (c.digits_.size() > 1 && c.digits_.back() == 0) {
        c.digits_.pop_back();
    }
    return *this = c;
}

BigInteger BigInteger::operator-() const {
    BigInteger copy(*this);
    copy.changeSign();
    return copy;
}

BigInteger& BigInteger::operator++() {
    return *this += 1;
}

BigInteger BigInteger::operator++(int) {
    BigInteger copy = *this;
    ++(*this);
    return copy;
}

BigInteger& BigInteger::operator--() {
    return *this -= 1;
}

BigInteger BigInteger::operator--(int) {
    BigInteger copy = *this;
    --(*this);
    return copy;
}

bool operator<(const BigInteger& b1, const BigInteger& b2) {
    if (b1.sign() != b2.sign()) {
        return BigInteger::toNumber(b1.sign()) <
               BigInteger::toNumber(b2.sign());
    }
    return b1.sign() == BigInteger::NEGATIVE ? b2.unsignedOrder(b1)
                                             : b1.unsignedOrder(b2);
}

bool operator>(const BigInteger& b1, const BigInteger& b2) {
    return b2 < b1;
}

bool operator==(const BigInteger& b1, const BigInteger& b2) {
    if (b1.sign() != b2.sign()) {
        return false;
    }
    if (b1.digits_.size() != b2.digits_.size()) {
        return false;
    }
    for (size_t i = 0; i < b1.digits_.size(); ++i) {
        if (b1.digits_[i] != b2.digits_[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(const BigInteger& b1, const BigInteger& b2) {
    return !(b1 == b2);
}

bool operator>=(const BigInteger& b1, const BigInteger& b2) {
    return !(b1 < b2);
}

bool operator<=(const BigInteger& b1, const BigInteger& b2) {
    return !(b1 > b2);
}

BigInteger operator+(const BigInteger& a, const BigInteger& b) {
    BigInteger copy = a;
    copy += b;
    return copy;
}

BigInteger operator-(const BigInteger& a, const BigInteger& b) {
    BigInteger copy = a;
    copy -= b;
    return copy;
}

BigInteger operator*(const BigInteger& a, const BigInteger& b) {
    BigInteger copy = a;
    copy *= b;
    return copy;
}

BigInteger& BigInteger::divMod(const BigInteger& b, bool divmod) {
    BigInteger c, temp;
    BigInteger copy = b;
    if (b.sign() == NEGATIVE) {
        copy = -b;
    }
    for (size_t i = digits_.size(); i > 0; --i) {
        long long l = 0, r = BASE_, m;
        temp = temp * BASE_ + digits_[i - 1];
        while (l + 1 < r) {
            m = (r + l) >> 1;
            if (copy * m > temp) {
                r = m;
            } else {
                l = m;
            }
        }
        c.digits_.push_back(l);
        temp = temp - l * copy;
    }
    if (divmod) {
        std::reverse(c.digits_.begin(), c.digits_.end());
        digits_ = c.digits_;
    } else {
        digits_ = temp.digits_;
    }
    while (digits_.size() > 1 && digits_.back() == 0) {
        digits_.pop_back();
    }
    if (sign() == ZERO || (digits_.size() == 1 && digits_.back() == 0)) {
        type_ = ZERO;
    } else if (sign() != b.sign()) {
        type_ = NEGATIVE;
    } else {
        type_ = POSITIVE;
    }
    return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& b) {
    return divMod(b, true);
}

BigInteger& BigInteger::operator%=(const BigInteger& b) {
    return divMod(b, false);
}

BigInteger operator/(const BigInteger& a, const BigInteger& b) {
    BigInteger copy = a;
    copy /= b;
    return copy;
}

BigInteger operator%(const BigInteger& a, const BigInteger& b) {
    BigInteger copy = a;
    copy %= b;
    return copy;
}

std::istream& operator>>(std::istream& in, BigInteger& b) {
    std::string s;
    in >> s;
    b = BigInteger(s);
    return in;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& b) {
    std::string s = b.toString();
    out << s;
    return out;
}

BigInteger operator""_bi(unsigned long long n) {
    return {static_cast<long long>(n)};
}

BigInteger operator""_bi(const char* s, size_t size) {
    if (size == 0) {
        return 0;
    }
    return {static_cast<std::string>(s)};
}

bool BigInteger::isEven() {
    return digits_[0] % 2 == 0;
}

void BigInteger::halve() {
    long long carry = 0;
    for (size_t i = digits_.size(); i > 0; --i) {
        long long cur = digits_[i - 1] + carry * BASE_;
        digits_[i - 1] = cur / 2;
        carry = cur % 2;
    }
    while (digits_.size() > 1 && digits_.back() == 0) {
        digits_.pop_back();
    }
}

Rational::Rational() : Rational(0){};

Rational::Rational(const BigInteger& b) : denominator_(1), numerator_(b) {}

Rational::Rational(int n) : denominator_(1), numerator_(n) {}

Rational Rational::operator-() const {
    Rational copy(*this);
    copy.numerator_ = -numerator_;
    return copy;
}

Rational& Rational::operator+=(const Rational& b) {
    numerator_ *= b.denominator_;
    numerator_ += (b.numerator_ * denominator_);
    denominator_ *= b.denominator_;
    make_rational();
    return *this;
}

Rational& Rational::operator-=(const Rational& b) {
    numerator_ *= b.denominator_;
    numerator_ -= b.numerator_ * denominator_;
    denominator_ *= b.denominator_;
    make_rational();
    return *this;
}

Rational& Rational::operator*=(const Rational& b) {
    denominator_ *= b.denominator_;
    numerator_ *= b.numerator_;
    make_rational();
    return *this;
}

Rational& Rational::operator/=(const Rational& b) {
    denominator_ *= b.numerator_;
    numerator_ *= b.denominator_;
    make_rational();
    return *this;
}

void Rational::make_rational() {
    if (numerator_.sign() == BigInteger::ZERO) {
        denominator_ = 1;
        return;
    }
    if (denominator_.sign() == BigInteger::NEGATIVE) {
        denominator_.changeSign();
        numerator_.changeSign();
    }
    while (numerator_.isEven() && denominator_.isEven()) {
        numerator_.halve();
        denominator_.halve();
    }
    BigInteger a = (numerator_ > 0 ? numerator_ : -numerator_);
    BigInteger b = denominator_;
    if (a > b) {
        std::swap(a, b);
    }
    while (a) {
        while (a.isEven()) {
            a.halve();
        }
        while (b.isEven()) {
            b.halve();
        }
        if (a > b) {
            std::swap(a, b);
        }
        b -= a;
        if (a > b) {
            std::swap(a, b);
        }
    }
    numerator_ /= b;
    denominator_ /= b;
}

std::string Rational::toString() const {
    if (denominator_ == 1) {
        return (numerator_ / denominator_).toString();
    }
    return numerator_.toString() + '/' + denominator_.toString();
}

std::string Rational::asDecimal(size_t precision) const {
    if (precision == 0) {
        return (numerator_ / denominator_).toString();
    }
    std::string s;
    if (numerator_.sign() == BigInteger::NEGATIVE) {
        s.push_back('-');
    }
    BigInteger copy_numerator = (numerator_ > 0 ? numerator_ : -numerator_);
    BigInteger copy_denominator = denominator_;
    BigInteger a = 1;
    for (size_t i = 0; i < precision; ++i) {
        a *= 10;
    }
    std::string s2 = ((copy_numerator * a) / copy_denominator).toString();
    if (s2.size() > precision) {
        s += s2.substr(0, s2.size() - precision);
        s2 = s2.substr(s2.size() - precision, precision);
    } else {
        s += "0";
    }
    return s + '.' +
           (precision > s2.size() ? std::string(precision - s2.size(), '0')
                                  : "") +
           s2;
}

Rational::operator double() const {
    return std::stod(asDecimal(15));
}

Rational operator+(const Rational& a, const Rational& b) {
    Rational copy = a;
    copy += b;
    return copy;
}

Rational operator-(const Rational& a, const Rational& b) {
    Rational copy = a;
    copy -= b;
    return copy;
}

Rational operator*(const Rational& a, const Rational& b) {
    Rational copy = a;
    copy *= b;
    return copy;
}

Rational operator/(const Rational& a, const Rational& b) {
    Rational copy = a;
    copy /= b;
    return copy;
}

bool operator<(const Rational& r1, const Rational& r2) {
    return r1.numerator_ * r2.denominator_ < r2.numerator_ * r1.denominator_;
}

bool operator>(const Rational& r1, const Rational& r2) {
    return r2 < r1;
}

bool operator==(const Rational& r1, const Rational& r2) {
    return (r1 - r2).numerator_ == 0;
}

bool operator!=(const Rational& r1, const Rational& r2) {
    return !(r1 == r2);
}

bool operator>=(const Rational& r1, const Rational& r2) {
    return !(r1 < r2);
}

bool operator<=(const Rational& r1, const Rational& r2) {
    return !(r1 > r2);
}

std::ostream& operator<<(std::ostream& out, const Rational& r) {
    out << r.toString();
    return out;
}

std::istream& operator>>(std::istream& in, Rational& r) {
    BigInteger b;
    in >> b;
    r = Rational(b);
    return in;
}
