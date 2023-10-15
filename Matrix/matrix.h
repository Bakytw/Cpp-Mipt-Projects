#include <array>
#include "biginteger.h"

template <size_t N, size_t Divisor, bool signal = false>
struct isPrimeHelper {
    static const bool value =
        (N % Divisor != 0) &&
        isPrimeHelper<N, Divisor + 1, Divisor * Divisor >= N>::value;
};

template <size_t N, size_t Divisor>
struct isPrimeHelper<N, Divisor, true> {
    static const bool value = true;
};

template <>
struct isPrimeHelper<1, 2, false> {
    static const bool value = false;
};

template <>
struct isPrimeHelper<2, 2, false> {
    static const bool value = true;
};

template <size_t N>
const bool isPrime = isPrimeHelper<N, 2, false>::value;

template <size_t N>
class Residue {
  public:
    explicit Residue(int n);
    explicit operator int() const;
    Residue() : Residue(0) {}
    ~Residue() = default;
    Residue<N>& operator+=(const Residue<N>& n);
    Residue<N>& operator-=(const Residue<N>& n);
    Residue<N>& operator*=(const Residue<N>& n);
    Residue<N>& operator/=(const Residue<N>& n);
    template <size_t S>
    friend bool operator==(const Residue<S>& n1, const Residue<S>& n2);
    template <size_t S>
    friend std::ostream& operator<<(std::ostream& out, const Residue<S>& n);

  private:
    long long value_;
};

template <size_t M, size_t N, typename Field = Rational>
class Matrix {
  public:
    Matrix() = default;
    ~Matrix() = default;
    Matrix(const Matrix& m);
    template <typename T>
    Matrix(const std::initializer_list<std::initializer_list<T>>& il);
    Matrix<M, N, Field>& operator+=(const Matrix<M, N, Field>& other);
    Matrix<M, N, Field>& operator-=(const Matrix<M, N, Field>& other);
    Matrix<M, N, Field>& operator*=(const Field& other);
    Matrix<M, N, Field>& operator*=(const Matrix<N, N, Field>& other);
    Field det() const;
    Matrix<N, M, Field> transposed() const;
    size_t rank() const;
    Matrix<M, N, Field> inverted() const;
    void invert();
    Field trace() const;
    std::array<Field, N> getRow(size_t row) const;
    std::array<Field, M> getColumn(size_t column) const;
    const std::array<Field, N>& operator[](size_t index) const;
    std::array<Field, N>& operator[](size_t index);
    size_t gaussForwardAndReverse(bool to_revert);
    Matrix<M, N, Field>& sumSub(bool plus, const Matrix<M, N, Field>& other);

  private:
    std::array<std::array<Field, N>, M> matrix_;
};

template <size_t N, typename Field = Rational>
using SquareMatrix = Matrix<N, N, Field>;

template <size_t N>
Residue<N>::Residue(int n) : value_(n % static_cast<long long>(N)) {
    value_ += (value_ < 0 ? N : 0);
}

template <size_t N>
Residue<N>::operator int() const {
    return static_cast<int>(value_);
}

template <size_t N>
Residue<N>& Residue<N>::operator+=(const Residue<N>& n) {
    value_ = (value_ + n.value_) % N;
    return *this;
}

template <size_t N>
Residue<N>& Residue<N>::operator-=(const Residue<N>& n) {
    value_ = (value_ + N - n.value_) % N;
    return *this;
}

template <size_t N>
Residue<N>& Residue<N>::operator*=(const Residue<N>& n) {
    value_ = (value_ * n.value_) % N;
    return *this;
}

template <size_t N>
Residue<N>& Residue<N>::operator/=(const Residue<N>& n) {
    static_assert(isPrime<N>, "Division: it's not a Field");
    Residue<N> base(n);
    ssize_t exponent = N - 2;
    while (exponent != 0) {
        if ((exponent & 1) != 0) {
            *this *= base;
        }
        exponent >>= 1;
        base *= base;
    }
    return *this;
}

template <size_t N>
Residue<N> operator+(const Residue<N>& a, const Residue<N>& b) {
    Residue<N> copy = a;
    copy += b;
    return copy;
}

template <size_t N>
Residue<N> operator-(const Residue<N>& a, const Residue<N>& b) {
    Residue<N> copy = a;
    copy -= b;
    return copy;
}

template <size_t N>
Residue<N> operator*(const Residue<N>& a, const Residue<N>& b) {
    Residue<N> copy = a;
    copy *= b;
    return copy;
}

template <size_t N>
Residue<N> operator/(const Residue<N>& a, const Residue<N>& b) {
    Residue<N> copy = a;
    copy /= b;
    return copy;
}

template <size_t S>
bool operator==(const Residue<S>& n1, const Residue<S>& n2) {
    return n1.value_ == n2.value_;
}

template <size_t S>
bool operator!=(const Residue<S>& n1, const Residue<S>& n2) {
    return !(n1.value_ == n2.value_);
}

template <size_t N>
std::ostream& operator<<(std::ostream& out, const Residue<N>& n) {
    out << n.value_;
    return out;
}

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field>::Matrix(const Matrix& m) : matrix_(m.matrix_) {}

template <size_t M, size_t N, typename Field>
template <typename T>
Matrix<M, N, Field>::Matrix(
    const std::initializer_list<std::initializer_list<T>>& il) {
    size_t i = 0;
    for (auto row = il.begin(); row != il.end(); ++row) {
        size_t j = 0;
        for (auto column = row->begin(); column != row->end(); ++column) {
            matrix_[i][j] = Field(*column);
            ++j;
        }
        ++i;
    }
}

template <size_t M, size_t N, typename Field>
size_t Matrix<M, N, Field>::gaussForwardAndReverse(bool to_revert) {
    size_t count_swaps = 0;
    for (size_t i = 0; i < std::min(M, N); ++i) {
        size_t j = i;
        if (matrix_[i][i] == Field(0)) {
            for (j = i + 1; j < M; ++j) {
                if (matrix_[j][i] != Field(0)) {
                    break;
                }
            }
        }
        if (j == M) {
            continue;
        }
        if (i != j) {
            ++count_swaps;
            std::swap(matrix_[i], matrix_[j]);
        }
        for (size_t k = i + 1; k < M; ++k) {
            if (matrix_[i][i] == Field(0)) {
                continue;
            }
            Field c = matrix_[k][i] / matrix_[i][i];
            for (size_t h = 0; h < N; ++h) {
                matrix_[k][h] -= c * matrix_[i][h];
            }
        }
    }
    if (!to_revert) {
        return count_swaps;
    }
    for (size_t i = M; i > 0; --i) {
        for (size_t j = 0; j < i - 1; ++j) {
            Field c = matrix_[j][i - 1] / matrix_[i - 1][i - 1];
            for (size_t k = 0; k < 2 * M; ++k) {
                matrix_[j][k] -= c * matrix_[i - 1][k];
            }
        }
    }
    return 0;
}

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field>& Matrix<M, N, Field>::sumSub(
    bool plus, const Matrix<M, N, Field>& other) {
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            if (plus) {
                matrix_[i][j] += other.matrix_[i][j];
            } else {
                matrix_[i][j] -= other.matrix_[i][j];
            }
        }
    }
    return *this;
}

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field>& Matrix<M, N, Field>::operator+=(
    const Matrix<M, N, Field>& other) {
    return sumSub(true, other);
}

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field>& Matrix<M, N, Field>::operator-=(
    const Matrix<M, N, Field>& other) {
    return sumSub(false, other);
}

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field>& Matrix<M, N, Field>::operator*=(const Field& other) {
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            matrix_[i][j] *= other;
        }
    }
    return *this;
}

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field> operator*(const Field& a, const Matrix<M, N, Field>& b) {
    Matrix<M, N, Field> copy = b;
    copy *= a;
    return copy;
}

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field>& Matrix<M, N, Field>::operator*=(
    const Matrix<N, N, Field>& other) {
    static_assert(M == N, "Multiplication: matrix_ is not a square");
    return *this = *this * other;
}

template <size_t M, size_t N, size_t K, typename Field>
Matrix<M, K, Field> operator*(const Matrix<M, N, Field>& a,
                              const Matrix<N, K, Field>& b) {
    Matrix<M, K, Field> copy;
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < K; ++j) {
            for (size_t k = 0; k < N; ++k) {
                copy[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return copy;
}

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field> operator+(const Matrix<M, N, Field>& a,
                              const Matrix<M, N, Field>& b) {
    Matrix<M, N, Field> copy = a;
    copy += b;
    return copy;
}

template <size_t N, size_t M, typename Field>
Matrix<M, N, Field> operator-(const Matrix<M, N, Field>& a,
                              const Matrix<M, N, Field>& b) {
    Matrix<M, N, Field> copy = a;
    copy -= b;
    return copy;
}

template <size_t M, size_t N, typename Field>
Field Matrix<M, N, Field>::det() const {
    static_assert(M == N, "Det: matrix_ is not a square");
    Matrix<M, N, Field> copy = *this;
    size_t sign = copy.gaussForwardAndReverse(false);
    Field ans(1);
    if (sign % 2 == 1) {
        ans *= Field(-1);
    }
    for (size_t i = 0; i < M; ++i) {
        ans *= copy[i][i];
    }
    return ans;
}

template <size_t M, size_t N, typename Field>
Matrix<N, M, Field> Matrix<M, N, Field>::transposed() const {
    Matrix<N, M, Field> copy;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            copy[i][j] = matrix_[j][i];
        }
    }
    return copy;
}

template <size_t M, size_t N, typename Field>
size_t Matrix<M, N, Field>::rank() const {
    Matrix copy(*this);
    copy.gaussForwardAndReverse(false);
    size_t i = 0, j = 0;
    while ((i < M) && (j < N)) {
        if (copy[i][j] != Field(0)) {
            ++i;
        }
        ++j;
    }
    return i;
}

template <size_t M, size_t N, typename Field>
Matrix<M, N, Field> Matrix<M, N, Field>::inverted() const {
    Matrix<M, N, Field> copy = *this;
    copy.invert();
    return copy;
}

template <size_t M, size_t N, typename Field>
void Matrix<M, N, Field>::invert() {
    static_assert(M == N, "Invert: matrix_ is not a square");
    Matrix<M, 2 * M, Field> copy;
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < M; ++j) {
            copy[i][j] = matrix_[i][j];
        }
        copy[i][N + i] = Field(1);
    }
    copy.gaussForwardAndReverse(true);
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < M; ++j) {
            matrix_[i][j] = copy[i][j + M] / copy[i][i];
        }
    }
}

template <size_t M, size_t N, typename Field>
Field Matrix<M, N, Field>::trace() const {
    static_assert(M == N, "Trace: matrix_ is not a square");
    Field trace(0);
    for (size_t i = 0; i < M; ++i) {
        trace += matrix_[i][i];
    }
    return trace;
}

template <size_t M, size_t N, typename Field>
std::array<Field, N> Matrix<M, N, Field>::getRow(size_t row) const {
    return matrix_[row];
}

template <size_t M, size_t N, typename Field>
std::array<Field, M> Matrix<M, N, Field>::getColumn(size_t column) const {
    std::array<Field, M> copy;
    for (size_t j = 0; j < M; ++j) {
        copy[j] = matrix_[j][column];
    }
    return copy;
}

template <size_t M, size_t N, typename Field>
const std::array<Field, N>& Matrix<M, N, Field>::operator[](
    size_t index) const {
    return matrix_[index];
}

template <size_t M, size_t N, typename Field>
std::array<Field, N>& Matrix<M, N, Field>::operator[](size_t index) {
    return matrix_[index];
}

template <size_t M, size_t N, typename Field>
bool operator==(const Matrix<M, N, Field>& a, const Matrix<M, N, Field>& b) {
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            if (a[i][j] != b[i][j]) {
                return false;
            }
        }
    }
    return true;
}
