// chord.cpp
#include <iostream>
#include <cmath>
#include <cassert>
#include <functional>

// Метод хорд для нахождения корня уравнения f(x)=0 на отрезке [a, b]
// Параметры:
// a, b - границы отрезка, на котором корень отделен (f(a)*f(b) < 0)
// eps - требуемая точность
// f - функция, корень которой ищем
// Возвращает приближенное значение корня (NaN, если корень не отделен)
double chord_method(double a, double b, double eps, std::function<double(double)> f) {
    double fa = f(a), fb = f(b);
    // Проверка: на концах отрезка функция должна иметь разные знаки
    if (fa * fb >= 0) {
        std::cerr << "Корень не отделен на заданном отрезке.\n";
        return NAN;
    }

    double x_prev = a;      // предыдущее приближение
    double x_curr = b;      // текущее приближение
    int iter = 0;           // счетчик итераций (ограничение 1000)

    // Продолжаем, пока изменение x больше точности и не превышен лимит итераций
    while (fabs(x_curr - x_prev) > eps && iter < 1000) {
        double f_prev = f(x_prev);
        double f_curr = f(x_curr);

        // Формула метода хорд: следующее приближение
        double x_next = x_curr - f_curr * (x_curr - x_prev) / (f_curr - f_prev);

        // Сдвигаем окно: предыдущее приближение становится текущим,
        // текущее – новым вычисленным значением
        x_prev = x_curr;
        x_curr = x_next;
        ++iter;
    }
    return x_curr;
}

// Unit-тест: ищем корень уравнения x^2 - 4 = 0 на отрезке [1, 3] (корень x=2)
#ifdef CHORD_TEST
int main() {
    // Определяем функцию f(x) = x*x - 4
    auto f = [](double x) { return x * x - 4; };

    // Вызываем метод хорд с точностью 1e-6
    double root = chord_method(1.0, 3.0, 1e-6, f);

    // Проверяем, что найденный корень близок к 2
    assert(fabs(root - 2.0) < 1e-5);
    std::cout << "Тест метода хорд пройден. Корень = " << root << "\n";
    return 0;
}
#endif