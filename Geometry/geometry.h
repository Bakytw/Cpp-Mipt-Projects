#include <cmath>
#include <iostream>
#include <vector>

static const double EPS = 1e-6;

struct Point {
    double x, y;
    Point() = default;
    Point(double x, double y);
    void rotate(const Point& center, double angle);
    void reflect(const Point& center);
    void scale(const Point& center, double coefficient);
};

double Distance(const Point& p1, const Point& p2);
bool operator==(const Point& p1, const Point& p2);
bool operator!=(const Point& p1, const Point& p2);

class Line {
  public:
    double a, b, c;  // ax + by + c = 0
    Line(double a, double b, double c);
    Line(double k, double b);
    Line(const Point& p, double k);
    Line(const Point& p1, const Point& p2);
    bool operator==(const Line& another) const;
    bool operator!=(const Line& another) const;
    void reflect(Point& p) const;
};

class Shape {
  public:
    virtual double perimeter() const = 0;
    virtual double area() const = 0;
    virtual bool isCongruentTo(const Shape& another) const = 0;
    virtual bool isSimilarTo(const Shape& another) const = 0;
    virtual bool containsPoint(const Point& point) const = 0;
    virtual void rotate(const Point& center, double angle) = 0;
    virtual void reflect(const Point& center) = 0;
    virtual void reflect(const Line& axis) = 0;
    virtual void scale(const Point& center, double coefficient) = 0;
    virtual ~Shape() = default;
};

bool operator==(const Shape& sh1, const Shape& sh2);

class Polygon : public Shape {
  protected:
    std::vector<Point> points_;

  public:
    template <typename... Args>
    Polygon(const Args&... args) : points_{args...} {}
    friend bool operator==(const Polygon& p1, const Polygon& p2);
    size_t verticesCount() const;
    const std::vector<Point>& getVertices() const;
    bool isConvex() const;
    double perimeter() const override;
    double area() const override;
    bool isCongruentTo(const Shape& another) const override;
    bool isSimilarTo(const Shape& another) const override;
    bool containsPoint(const Point& point) const override;
    void rotate(const Point& center, double angle) override;
    void reflect(const Point& center) override;
    void reflect(const Line& axis) override;
    void scale(const Point& center, double coefficient) override;
    ~Polygon() override = default;
};

class Ellipse : public Shape {
  protected:
    Point f1, f2;
    double a, b;

  public:
    Ellipse(const Point& f1, const Point& f2, double sum);
    friend bool operator==(const Ellipse& el1, const Ellipse& el2);
    std::pair<Point, Point> focuses() const;
    double eccentricity() const;
    std::pair<Line, Line> directrices() const;
    Point center() const;
    double perimeter() const override;
    double area() const override;
    bool isCongruentTo(const Shape& another) const override;
    bool isSimilarTo(const Shape& another) const override;
    bool containsPoint(const Point& point) const override;
    void rotate(const Point& center, double angle) override;
    void reflect(const Point& center) override;
    void reflect(const Line& axis) override;
    void scale(const Point& center, double coefficient) override;
    ~Ellipse() override = default;
};

class Circle : public Ellipse {
  public:
    Circle(const Point& center, double r);
    double radius() const;
    double perimeter() const override;
    double area() const override;
    ~Circle() override = default;
};

class Rectangle : public Polygon {
  public:
    Rectangle(const Point& p1, const Point& p2, double coefficient);
    Point center() const;
    std::pair<Line, Line> diagonals() const;
    double perimeter() const override;
    double area() const override;
    ~Rectangle() override = default;
};

class Square : public Rectangle {
  public:
    Square(const Point& p1, const Point& p2);
    Circle circumscribedCircle() const;
    Circle inscribedCircle() const;
    ~Square() override = default;
};

class Triangle : public Polygon {
  public:
    Triangle(const Point& p1, const Point& p2, const Point& p3);
    Circle circumscribedCircle() const;
    Circle inscribedCircle() const;
    Point centroid() const;
    Point orthocenter() const;
    Line EulerLine() const;
    Circle ninePointsCircle() const;
    ~Triangle() override = default;
};
