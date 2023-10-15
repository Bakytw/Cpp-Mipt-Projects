#include <cstring>
#include <iostream>

class String {
  private:
    size_t size_;
    size_t capacity_;
    char* buffer_;
    void add_zero();

  public:
    String(const char* s);
    String(size_t n, char c);
    String();
    String(const String& s);
    ~String();
    String& operator=(String s);
    void swap(String& s);
    const char& operator[](size_t index) const;
    char& operator[](size_t index);
    size_t length() const;
    size_t size() const;
    size_t capacity() const;
    char* data();
    const char* data() const;
    void reserve(size_t new_capacity);
    void shrink_to_fit();
    void push_back(char c);
    void pop_back();
    const char& front() const;
    char& front();
    const char& back() const;
    char& back();
    String& operator+=(const String& s);
    String& operator+=(char c);
    bool empty() const;
    void clear();
    size_t find(const String& s) const;
    String substr(size_t start, size_t count) const;
    size_t rfind(const String& s) const;
};

bool operator<(const String& s1, const String& s2);

bool operator>(const String& s1, const String& s2);

bool operator==(const String& s1, const String& s2);

bool operator!=(const String& s1, const String& s2);

bool operator>=(const String& s1, const String& s2);

bool operator<=(const String& s1, const String& s2);

String operator+(const String& s1, const String& s2);

String operator+(char s1, const String& s2);

String operator+(const String& s1, char s2);

std::ostream& operator<<(std::ostream& out, const String& s);

std::istream& operator>>(std::istream& in, String& s);
