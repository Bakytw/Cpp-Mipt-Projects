#include "geometry.h"

Point::Point(double x, double y) : x(x), y(y) {}

void Point::rotate(const Point& center, double angle) {
    angle *= M_PI / 180;
    double new_x = x - center.x;
    double new_y = y - center.y;
    x = center.x + new_x * cos(angle) - new_y * sin(angle);
    y = center.y + new_x * sin(angle) + new_y * cos(angle);
}

void Point::reflect(const Point& center) {
    x = 2 * center.x - x;
    y = 2 * center.y - y;
}

void Point::scale(const Point& center, double coefficient) {
    x = center.x + coefficient * (x - center.x);
    y = center.y + coefficient * (y - center.y);
}

double Distance(const Point& p1, const Point& p2) {
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

bool operator==(const Point& p1, const Point& p2) {
    return Distance(p1, p2) < EPS;
}

bool operator!=(const Point& p1, const Point& p2) {
    return !(p1 == p2);
}

Line::Line(double a, double b, double c) : a(a), b(b), c(c) {}

Line::Line(double k, double b) : a(-k), b(1), c(-b) {}

Line::Line(const Point& p, double k) : a(-k), b(1), c(k * p.x - p.y) {}

Line::Line(const Point& p1, const Point& p2)
    : a(p1.y - p2.y), b(p2.x - p1.x), c(p1.x * p2.y - p2.x * p1.y) {}

bool Line::operator==(const Line& another) const {
    if (a * another.b - b * another.a >= EPS) {
        return false;
    }
    if (a * another.c - c * another.a >= EPS) {
        return false;
    }
    return true;
}

bool Line::operator!=(const Line& another) const {
    return !(*this == another);
}

void Line::reflect(Point& p) const {
    Point temp = p;
    p.x += -2 * a * (a * temp.x + b * temp.y + c) / (a * a + b * b);
    p.y += -2 * b * (a * temp.x + b * temp.y + c) / (a * a + b * b);
}

bool operator==(const Shape& sh1, const Shape& sh2) {
    const auto* sh1Polygon = dynamic_cast<const Polygon*>(&sh1);
    if (sh1Polygon == nullptr) {
        const auto* sh1Ellipse = dynamic_cast<const Ellipse*>(&sh1);
        const auto* sh2Polygon = dynamic_cast<const Polygon*>(&sh2);
        if (sh2Polygon == nullptr) {
            const auto* sh2Ellipse = dynamic_cast<const Ellipse*>(&sh2);
            return *sh1Ellipse == *sh2Ellipse;
        } else {
            return false;
        }
    } else {
        const auto* sh2Polygon = dynamic_cast<const Polygon*>(&sh2);
        if (sh2Polygon == nullptr) {
            return false;
        } else {
            return *sh1Polygon == *sh2Polygon;
        }
    }
}

bool operator==(const Polygon& p1, const Polygon& p2) {
    if (p1.points_.size() != p2.points_.size()) {
        return false;
    }
    size_t sz = p1.points_.size();
    for (size_t to_reflect = 0; to_reflect < 2; ++to_reflect) {
        for (size_t shift = 0; shift < sz; ++shift) {
            bool flag = true;
            for (size_t j = 0; j < sz; ++j) {
                if (p1.points_[to_reflect == 1 ? sz - j - 1 : j] !=
                    p2.points_[(shift + j) % sz]) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                return true;
            }
        }
    }
    return false;
}

size_t Polygon::verticesCount() const {
    return points_.size();
}

const std::vector<Point>& Polygon::getVertices() const {
    return points_;
}

bool Polygon::isConvex() const {
    bool flag1 = false;
    bool flag2 = false;
    for (size_t i = 0; i < points_.size(); ++i) {
        double sign = (points_[i].x - points_[(i + 1) % points_.size()].x) *
                          (points_[(i + 2) % points_.size()].y -
                           points_[(i + 1) % points_.size()].y) -
                      (points_[i].y - points_[(i + 1) % points_.size()].y) *
                          (points_[(i + 2) % points_.size()].x -
                           points_[(i + 1) % points_.size()].x);
        if (sign > 0) {
            flag1 = true;
        } else {
            flag2 = true;
        }
        if (flag1 && flag2) {
            return false;
        }
    }
    return true;
}

double Polygon::perimeter() const {
    double temp = 0;
    for (size_t i = 0; i < points_.size(); ++i) {
        temp += Distance(points_[i], points_[(i + 1) % points_.size()]);
    }
    return temp;
}

double Polygon::area() const {
    double temp = 0;
    for (size_t i = 0; i < points_.size(); ++i) {
        temp += (points_[(i + 1) % points_.size()].x - points_[i].x) *
                (points_[(i + 1) % points_.size()].y + points_[i].y);
    }
    return std::abs(temp / 2);
}

bool Polygon::isCongruentTo(const Shape& another) const {
    return isSimilarTo(another) &&
           std::abs(perimeter() - another.perimeter()) < EPS;
}

bool Polygon::isSimilarTo(const Shape& another) const {
    const auto* temp = dynamic_cast<const Polygon*>(&another);
    if (temp == nullptr) {
        return false;
    }
    if (points_.size() != temp->points_.size()) {
        return false;
    }
    double k = perimeter() / temp->perimeter();
    size_t sz = points_.size();
    for (size_t to_reflect = 0; to_reflect < 2; ++to_reflect) {
        for (size_t shift = 0; shift < sz; ++shift) {
            bool flag = true;
            for (size_t j = 0; j < sz; ++j) {
                if (std::abs(
                        k -
                        Distance(
                            points_[to_reflect == 1 ? (sz - j - 1) % sz : j],
                            points_[to_reflect == 1 ? (sz - j) % sz
                                                    : (j + 1) % sz]) /
                            Distance(temp->points_[(shift + j) % sz],
                                     temp->points_[(shift + j + 1) % sz])) >=
                    EPS) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                return true;
            }
        }
    }
    return false;
}

bool Polygon::containsPoint(const Point& point) const {
    bool flag = false;
    size_t j = points_.size() - 1;
    for (size_t i = 0; i < points_.size(); ++i) {
        if (((points_[i].x + (point.y - points_[i].y) /
                                 (points_[j].y - points_[i].y) *
                                 (points_[j].x - points_[i].x)) < point.x) &&
            ((points_[j].y < point.y && points_[i].y >= point.y) ||
             (points_[i].y < point.y && points_[j].y >= point.y))) {
            flag = !flag;
        }
        j = i;
    }
    return flag;
}
void Polygon::rotate(const Point& center, double angle) {
    for (size_t i = 0; i < points_.size(); ++i) {
        points_[i].rotate(center, angle);
    }
}
void Polygon::reflect(const Point& center) {
    for (size_t i = 0; i < points_.size(); ++i) {
        points_[i].reflect(center);
    }
}
void Polygon::reflect(const Line& axis) {
    for (size_t i = 0; i < points_.size(); ++i) {
        axis.reflect(points_[i]);
    }
}
void Polygon::scale(const Point& center, double coefficient) {
    for (size_t i = 0; i < points_.size(); ++i) {
        points_[i].scale(center, coefficient);
    }
}

bool operator==(const Ellipse& el1, const Ellipse& el2) {
    if ((el1.f1 == el2.f1 && el1.f2 == el2.f2) ||
        (el1.f1 == el2.f2 && el1.f2 == el2.f1)) {
        return std::abs(el1.a - el2.a) < EPS && std::abs(el1.b - el2.b) < EPS;
    }
    return false;
}

Ellipse::Ellipse(const Point& f1, const Point& f2, double sum)
    : f1(f1),
      f2(f2),
      a(sum / 2),
      b(sqrt(
          std::abs(Distance(center(), f1) * Distance(center(), f1) - a * a))) {}

std::pair<Point, Point> Ellipse::focuses() const {
    return {f1, f2};
}

double Ellipse::eccentricity() const {
    return std::abs(Distance(center(), f1) / a);
}

std::pair<Line, Line> Ellipse::directrices() const {
    Line l1(f1, f2);
    double c = a * sqrt(l1.a * l1.a + l1.b * l1.b) / eccentricity();
    return {Line(-l1.b, l1.a, c), Line(-l1.b, l1.a, -c)};
}

Point Ellipse::center() const {
    return {0.5 * (f1.x + f2.x), 0.5 * (f1.y + f2.y)};
}

double Ellipse::perimeter() const {
    return M_PI * (3 * (a + b) - sqrt((3 * a + b) * (a + 3 * b)));
}
double Ellipse::area() const {
    return M_PI * a * b;
}

bool Ellipse::isCongruentTo(const Shape& another) const {
    const auto* temp = dynamic_cast<const Ellipse*>(&another);
    if (temp == nullptr) {
        return false;
    }
    return isSimilarTo(*temp) && std::abs(a - temp->a) < EPS;
}

bool Ellipse::isSimilarTo(const Shape& another) const {
    const auto* temp = dynamic_cast<const Ellipse*>(&another);
    return std::abs(eccentricity() - temp->eccentricity()) < EPS;
}

bool Ellipse::containsPoint(const Point& point) const {
    return Distance(f1, point) + Distance(f2, point) - 2 * a < EPS;
}

void Ellipse::rotate(const Point& center, double angle) {
    f1.rotate(center, angle);
    f2.rotate(center, angle);
}

void Ellipse::reflect(const Point& center) {
    f1.reflect(center);
    f2.reflect(center);
}

void Ellipse::reflect(const Line& axis) {
    axis.reflect(f1);
    axis.reflect(f2);
}

void Ellipse::scale(const Point& center, double coefficient) {
    a *= coefficient;
    b *= coefficient;
    f1.scale(center, coefficient);
    f2.scale(center, coefficient);
}

Circle::Circle(const Point& center, double r)
    : Ellipse(center, center, r * 2) {}

double Circle::radius() const {
    return a;
}

double Circle::perimeter() const {
    return 2 * M_PI * radius();
}

double Circle::area() const {
    return M_PI * radius() * radius();
}

Rectangle::Rectangle(const Point& p1, const Point& p2, double coefficient) {
    points_.resize(4, Point());
    points_[0] = p1;
    points_[2] = p2;
    double diag = Distance(p1, p2);
    coefficient = std::max(coefficient, 1 / coefficient);
    double alphasmall = 180 * atan(coefficient) / M_PI;
    double alphabig = alphasmall - 90;
    double small = sqrt(diag * diag / (coefficient * coefficient + 1));
    double big = small * coefficient;
    points_[1].x = (p2.x - p1.x) * (small / diag);
    points_[1].y = (p2.y - p1.y) * (small / diag);
    points_[1].rotate(Point(0, 0), alphasmall);
    points_[1].x += p1.x;
    points_[1].y += p1.y;
    points_[3].x = (p2.x - p1.x) * (big / diag);
    points_[3].y = (p2.y - p1.y) * (big / diag);
    points_[3].rotate(Point(0, 0), alphabig);
    points_[3].x += p1.x;
    points_[3].y += p1.y;
}

Point Rectangle::center() const {
    return {0.5 * (points_[0].x + points_[2].x),
            0.5 * (points_[0].y + points_[2].y)};
}

std::pair<Line, Line> Rectangle::diagonals() const {
    return {Line(points_[0], points_[2]), Line(points_[1], points_[3])};
}

double Rectangle::perimeter() const {
    return 2 * (Distance(points_[0], points_[1]) +
                Distance(points_[1], points_[2]));
}

double Rectangle::area() const {
    return Distance(points_[0], points_[1]) * Distance(points_[1], points_[2]);
}

Square::Square(const Point& p1, const Point& p2) : Rectangle(p1, p2, 1) {}

Circle Square::circumscribedCircle() const {
    return {center(), Distance(points_[0], points_[2]) * sqrt(2) / 2};
}

Circle Square::inscribedCircle() const {
    return {center(), Distance(points_[0], points_[1]) / 2};
}

Triangle::Triangle(const Point& p1, const Point& p2, const Point& p3)
    : Polygon(p1, p2, p3) {}

Circle Triangle::circumscribedCircle() const {
    Point a = points_[0];
    Point b = points_[1];
    Point c = points_[2];
    Point center;
    double d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
    center.x = ((a.x * a.x + a.y * a.y) * (b.y - c.y) +
                (b.x * b.x + b.y * b.y) * (c.y - a.y) +
                (c.x * c.x + c.y * c.y) * (a.y - b.y)) /
               d;
    center.y = ((a.x * a.x + a.y * a.y) * (c.x - b.x) +
                (b.x * b.x + b.y * b.y) * (a.x - c.x) +
                (c.x * c.x + c.y * c.y) * (b.x - a.x)) /
               d;
    return {center, Distance(center, a)};
}

Circle Triangle::inscribedCircle() const {
    double a = Distance(points_[1], points_[2]);
    double b = Distance(points_[0], points_[2]);
    double c = Distance(points_[0], points_[1]);
    Point center;
    center.x =
        (a * points_[0].x + b * points_[1].x + c * points_[2].x) / (a + b + c);
    center.y =
        (a * points_[0].y + b * points_[1].y + c * points_[2].y) / (a + b + c);
    return {center, 2 * area() / perimeter()};
}

Point Triangle::centroid() const {
    return {(points_[0].x + points_[1].x + points_[2].x) / 3,
            (points_[0].y + points_[1].y + points_[2].y) / 3};
}

Point Triangle::orthocenter() const {
    Point a = points_[0];
    Point b = points_[1];
    Point c = points_[2];
    Point center;
    center.x = a.y * a.y * (c.y - b.y) + b.x * c.x * (c.y - b.y) +
               b.y * b.y * (a.y - c.y) + a.x * c.x * (a.y - c.y) +
               c.y * c.y * (b.y - a.y) + a.x * b.x * (b.y - a.y);
    center.x /= a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y);
    center.y = a.x * a.x * (b.x - c.x) + b.y * c.y * (b.x - c.x) +
               b.x * b.x * (c.x - a.x) + a.y * c.y * (c.x - a.x) +
               c.x * c.x * (a.x - b.x) + a.y * b.y * (a.x - b.x);
    center.y /= a.y * (c.x - b.x) + b.y * (a.x - c.x) + c.y * (b.x - a.x);
    return center;
}

Line Triangle::EulerLine() const {
    return {centroid(), orthocenter()};
}

Circle Triangle::ninePointsCircle() const {
    Point p = orthocenter();
    Circle c = circumscribedCircle();
    p.x = 0.5 * (p.x + c.center().x);
    p.y = 0.5 * (p.y + c.center().y);
    return {p, c.radius() / 2};
}
