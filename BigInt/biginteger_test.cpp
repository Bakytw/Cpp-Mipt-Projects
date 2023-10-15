#include "biginteger.h"

#include <cassert>
#include <iostream>

void test1() {
    BigInteger from_empty;

    BigInteger from_int(123'456);
    BigInteger from_neg_int(-123'456);

    BigInteger from_string("12345678901234567890");
    BigInteger from_neg_string("-123123123123123123123123132132132131231");

    BigInteger copy_constr(from_string);
}

void test2() {
    assert(BigInteger("123456789123456789") ==
           BigInteger("123456789123456789"));
    assert(BigInteger("-12345678901234567890") ==
           BigInteger("-12345678901234567890"));
    assert(BigInteger("123456789123456789") != BigInteger("58943759834759"));
}

void test3() {
    assert(BigInteger(1) < BigInteger(10));
    assert(BigInteger(-100) < 5);
    assert(!(BigInteger(1) > BigInteger(10)));
    assert(!(BigInteger(-100) > BigInteger(5)));
    assert(BigInteger("123456789012345678901234567890") <
           BigInteger("123456789012545678901234567890"));
    assert(!(BigInteger("123456789012345678901234567890") <
             BigInteger("123456789012345678901234567890")));
}

void test4() {
    {
        BigInteger a("-1234567890123456789");
        BigInteger b("1234567890123456789");

        assert((a * b).toString() == "-1524157875323883675019051998750190521");
    }

    {
        BigInteger a(
            "12345123456789012345678923456789123534645723452363465473643423");
        BigInteger b("12568432423758325345984738557347237543");

        assert((a / b).toString() == "982232552203790490610772");
    }

    {
        BigInteger a(
            "12345123456789012345678923456789123534645723452363465473643423");
        BigInteger b("12568432423758325345984738557347237543");

        assert((a % b).toString() == "7378391778761293146339181012435030227");
    }
}

void test5() {
    if (BigInteger(0)) {
        assert(false);
    }
    bool called = false;
    if (BigInteger(1)) {
        called = true;
    }
    assert(called);
}

void test6() {
    Rational pi = 0;
    Rational sign = 1;

    for (BigInteger n = 1; n < 100; ++n) {
        pi += sign * (Rational(4) / (Rational(2) * n - Rational(1)));
        sign = -sign;
    }

    std::cerr << static_cast<double>(pi) << std::endl;
    assert(std::abs(static_cast<double>(pi) - 3.14) < 0.1);
    assert(Rational(3) < pi);
    assert(pi < Rational(4));
    assert(Rational(1) / Rational(2) ==
           Rational("100000000000000000000000000000"_bi) /
               Rational("200000000000000000000000000000"_bi));
}

int main() {
    test1();
    std::cerr << "Test 1 passed." << std::endl;
    test2();
    std::cerr << "Test 2 passed." << std::endl;
    test3();
    std::cerr << "Test 3 passed." << std::endl;
    test4();
    std::cerr << "Test 4 passed." << std::endl;
    test5();
    std::cerr << "Test 5 passed." << std::endl;
    test6();
    std::cerr << "Test 6 passed." << std::endl;
}
