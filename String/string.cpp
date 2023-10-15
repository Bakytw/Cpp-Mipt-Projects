#include "string.h"

void String::add_zero() {
    buffer_[size_] = '\0';
}

String::String(const char* s)
    : size_(strlen(s)), capacity_(size_), buffer_(new char[capacity_ + 1]) {
    memcpy(buffer_, s, size_);
    add_zero();
}

String::String(size_t n, char c)
    : size_(n), capacity_(size_), buffer_(new char[capacity_ + 1]) {
    memset(buffer_, c, n);
    add_zero();
}

String::String() : String(0, '\0') {}

String::String(const String& s) : String(s.size_, '\0') {
    memcpy(buffer_, s.buffer_, size_ + 1);
}

String::~String() {
    delete[] buffer_;
}

String& String::operator=(String s) {
    swap(s);
    return *this;
}

void String::swap(String& s) {
    std::swap(size_, s.size_);
    std::swap(capacity_, s.capacity_);
    std::swap(buffer_, s.buffer_);
}

const char& String::operator[](size_t index) const {
    return buffer_[index];
}

char& String::operator[](size_t index) {
    return buffer_[index];
}

size_t String::length() const {
    return size_;
}

size_t String::size() const {
    return size_;
}

size_t String::capacity() const {
    return capacity_;
}

char* String::data() {
    return buffer_;
}

const char* String::data() const {
    return buffer_;
}

void String::reserve(size_t new_capacity) {
    capacity_ = new_capacity;
    char* temp = new char[capacity_ + 1];
    memcpy(temp, buffer_, size_);
    delete[] buffer_;
    buffer_ = temp;
}

void String::shrink_to_fit() {
    if (capacity_ != size_) {
        reserve(size_);
        add_zero();
    }
}

void String::push_back(char c) {
    if (++size_ >= capacity_) {
        reserve(2 * size_ + 1);
    }
    buffer_[size_ - 1] = c;
    add_zero();
}

void String::pop_back() {
    buffer_[--size_] = '\0';
}

const char& String::front() const {
    return buffer_[0];
}

char& String::front() {
    return buffer_[0];
}

const char& String::back() const {
    return buffer_[size_ - 1];
}

char& String::back() {
    return buffer_[size_ - 1];
}

String& String::operator+=(const String& s) {
    if (capacity_ < s.size_ + size_) {
        reserve(2 * (s.size_ + size_));
    }
    memcpy(buffer_ + size_, s.buffer_, s.size_);
    size_ += s.size_;
    add_zero();
    return *this;
}

String& String::operator+=(char c) {
    push_back(c);
    return *this;
}

bool String::empty() const {
    return size_ == 0;
}

void String::clear() {
    size_ = 0;
    add_zero();
}

size_t String::find(const String& s) const {
    char* temp = strstr(buffer_, s.buffer_);
    if (temp != nullptr) {
        return temp - buffer_;
    }
    return length();
}

String String::substr(size_t start, size_t count) const {
    String temp(count, '\0');
    memcpy(temp.buffer_, buffer_ + start, count);
    return temp;
}

size_t String::rfind(const String& s) const {
    if (size_ < s.size_) {
        return size_;
    }
    for (size_t i = size_ - s.size_ + 1; i > 0; --i) {
        bool flag = true;
        for (size_t j = s.size_; j > 0; --j) {
            if (buffer_[j + i - 1] != s.buffer_[j - 1]) {
                flag = false;
            }
        }
        if (flag) {
            return i;
        }
    }
    return size_;
}

bool operator<(const String& s1, const String& s2) {
    return strcmp(s1.data(), s2.data()) < 0;
}

bool operator>(const String& s1, const String& s2) {
    return s2 < s1;
}

bool operator==(const String& s1, const String& s2) {
    if (s1.size() != s2.size()) {
        return false;
    }
    return strcmp(s1.data(), s2.data()) == 0;
}

bool operator!=(const String& s1, const String& s2) {
    return !(s1 == s2);
}

bool operator>=(const String& s1, const String& s2) {
    return !(s1 < s2);
}

bool operator<=(const String& s1, const String& s2) {
    return !(s1 > s2);
}

String operator+(const String& s1, const String& s2) {
    String copy = s1;
    copy += s2;
    return copy;
}

String operator+(char s1, const String& s2) {
    String copy(1, s1);
    copy += s2;
    return copy;
}

String operator+(const String& s1, char s2) {
    String copy = s1;
    copy.push_back(s2);
    return copy;
}

std::ostream& operator<<(std::ostream& out, const String& s) {
    out << s.data();
    return out;
}

std::istream& operator>>(std::istream& in, String& s) {
    char c;
    s.clear();
    while (in.get(c) && std::isspace(c) == 0) {
        s.push_back(c);
    }
    return in;
}
