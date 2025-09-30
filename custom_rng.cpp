#include "custom_rng.h"
#include <ctime>  // Для time(NULL)
#include <cmath>  // Для sqrtf, logf

// Функция соответствует коду на картинке
float* custom_uniform(int n) {
    // Выделяем массив float (new, как на картинке)
    float* r = new float[n];

    // Переменная y как int (32-bit signed)
    int y;

    // Статическая переменная b, инициализируется time(NULL) (как на картинке)
    static int b = static_cast<int>(time(NULL));

    for (int i = 0; i < n; ++i) {
        // Шаг 1: Умножение с переполнением (signed overflow allowed)
        y = b * 1220703125;

        // Шаг 2: Если y < 0 (после overflow), добавить 2 * 1073741824 (2^31)
        if (y < 0) {
            y += 2 * 1073741824;
        }

        // Шаг 3: Нормализация: y * 0.0000000004656613 (4.656613e-10f)
        r[i] = y * 4.656613e-10f;

        // Шаг 4: Обновляем b = y
        b = y;
    }

    return r;
}

// Генератор нормального распределения
float* custom_normal(int n, float mean, float stddev, int num_uniforms) {
    // Выделяем массив для результатов
    float* result = new float[n];

    for (int i = 0; i < n; ++i) {
        // Шаг 1: Генерируем num_uniforms равномерных чисел
        float* uniforms = custom_uniform(num_uniforms);

        // Шаг 2: Суммируем их в a
        float a = 0.0f;
        for (int j = 0; j < num_uniforms; ++j) {
            a += uniforms[j];
        }
        delete[] uniforms;  // Освобождаем память

        // Шаг 3: Вычисляем нормализованное значение: (a - num_uniforms/2) * sqrt(12/num_uniforms)
        float normalized = (a - num_uniforms / 2.0f) * sqrtf(12.0f / num_uniforms);

        // Шаг 4: Масштабируем: mean + stddev * normalized
        result[i] = mean + stddev * normalized;
    }

    return result;
}

// Генератор экспоненциального распределения
float* custom_exponential(int n, float lambda) {
    // Выделяем массив для результатов
    float* result = new float[n];

    // Генерируем n равномерных чисел
    float* uniforms = custom_uniform(n);

    for (int i = 0; i < n; ++i) {
        // Шаг 1: Берем r ( > 0, так как генератор не даёт точно 0)
        float r = uniforms[i];

        // Шаг 2: Вычисляем -log(r) / lambda (для положительного t)
        // Если r близко к 0, log(r) -> -inf, но на практике r > 0
        result[i] = -logf(r) / lambda;
    }

    delete[] uniforms;  // Освобождаем память

    return result;
}
