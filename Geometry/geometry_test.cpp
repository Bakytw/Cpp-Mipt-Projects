#include "geometry.h"

#include <cassert>
#include <iostream>

bool equal(long double a, long double b) {
    return (std::abs(a - b) < 1e-8);
}

void test1() {
    // points and lines
    Point a(0.5, 0.5);
    Point b(0.5, 0.5);
    Point c(-0.5, -0.5);
    assert(a == b);
    assert(a != c);
    assert(Point(b) != c);

    Line l1(a, c);
    Line l2(Point(5.5, 5.5), Point(1000.5, 1000.5));
    assert(l1 == l2);
    Line l3(Point(0, 3), Point(0, 0));
    assert(l1 != l3);
    Line l4(1, 0);
    Line l5(a, 1);
    assert(l1 == l4);
    assert(l4 == l5);
}

void test2() {
    // basic polygons
    Point a(0, 0);
    Point b(0, 1);
    Point c(1, 1);
    Point d(1, 0);
    Polygon poly(a, b, c, d);
    assert(poly.verticesCount() == 4);
    assert(poly.getVertices().size() == 4);
    assert(poly.isConvex());

    Polygon poly2(a, b, c, Point(0.1, 0.9));
    assert(!poly2.isConvex());
    assert(equal(poly.perimeter(), 4));
    assert(equal(poly.area(), 1));
}

void test3() {
    // basic ellipses
    Ellipse ellipse(Point(0, 0), Point(0, 1), 2);
    auto pair = ellipse.directrices();
    auto l1 = pair.first;
    ellipse.reflect(l1);
    ellipse.reflect(ellipse.focuses().first);
    auto el2 = ellipse;
    el2.scale(Point(2, 2), 10);
}

void test4() {
    // circles, rectangles, squares, polymorphism
    Circle circle(Point(0, 0), 1);
    Ellipse ellipse(Point(0, 0), Point(0, 0), 2);
    Ellipse& circle2 = circle;
    assert(circle2 == ellipse);
    Shape& sh1 = circle2;
    Shape& sh2 = ellipse;
    assert(sh1 == sh2);
    Square sq(Point(0, 0), Point(1, 1));
    Rectangle rect(Point(1, 0), Point(0, 1), 1);
    Shape& sh3 = sq;
    Shape& sh4 = rect;
    assert(sh3 == sh4);
    assert(sq.inscribedCircle().center() == sq.circumscribedCircle().center());

    Line l1 = rect.diagonals().first;
    Line l2 = rect.diagonals().second;
    assert(l1 != l2);
    sq.rotate(Point(0.5, 0.5), 90);
    assert(sq.isCongruentTo(rect));
    assert(!sq.containsPoint(Point(2, 2)));
    assert(!sq.isSimilarTo(ellipse));
    Square sq2(Point(100, 102), Point(102, 100));
    assert(sq.isSimilarTo(sq2));
}

void test5() {
    // triangles
    Point a(0, 0);
    Point b(0, 1);
    Point c(1, 2);
    Triangle triangle(a, b, c);
    Circle circle = triangle.circumscribedCircle();
    circle = triangle.inscribedCircle();
    circle = triangle.ninePointsCircle();
    Point p = triangle.centroid();
    p = triangle.orthocenter();
    Line l = triangle.EulerLine();
    triangle.reflect(l);
}

int main() {
    test1();
    std::cerr << "Test 1 (points and lines) passed." << std::endl;
    test2();
    std::cerr << "Test 2 (basic polygons) passed." << std::endl;
    test3();
    std::cerr << "Test 3 (basic ellipses) passed." << std::endl;
    test4();
    std::cerr << "Test 4 (circles, rectangles, squares, polymorphism) passed."
              << std::endl;
    test5();
    std::cerr << "Test 5 (triangles) passed." << std::endl;

    std::cout << 0 << std::endl;
}
