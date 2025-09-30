#include "custom_rng.h"
#include <iostream>   // Для вывода
#include <vector>
#include <random>     // Для библиотечного генератора
#include <cmath>      // Для pow и т.д.
#include <string>     // Для std::to_string
#include <unordered_map> // Для поиска периода
#include <ctime>      // Для time

int main() {
    setlocale(LC_ALL, "Russian");
    // Параметры
    const int N = 10000;  // Количество генерируемых чисел
    const int BINS = 10;  // Количество интервалов для гистограммы и хи-квадрат (интервалы по 0.1)

    // --- Тестирование равномерного ---
    // Генерация чисел из нашего ГПСЧ
    float* custom_nums_ptr = custom_uniform(N);
    // Копируем в vector<double> для удобства тестов (можно работать с float*, но для совместимости)
    std::vector<double> custom_nums(N);
    for (int i = 0; i < N; ++i) {
        custom_nums[i] = static_cast<double>(custom_nums_ptr[i]);
    }
    delete[] custom_nums_ptr;  // Освобождаем память (важно!)

    // Генерация чисел из библиотечного (std::mt19937 + uniform_real_distribution)
    std::vector<double> lib_nums(N);
    std::mt19937 gen(std::random_device{}());  // Инициализация случайным seed
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    for (int i = 0; i < N; ++i) {
        lib_nums[i] = dis(gen);
    }

    // Тест 1: Визуализация равномерности (вывод частот гистограммы, вместо графика)
    // Шаг: Делим [0,1) на BINS интервалов, считаем частоты и выводим
    std::cout << "Гистограмма (частоты в бинах) для нашего ГПСЧ:" << std::endl;
    std::vector<int> custom_hist(BINS, 0);
    for (double num : custom_nums) {
        int bin = static_cast<int>(num * BINS);
        if (bin < BINS) custom_hist[bin]++;
    }
    for (int i = 0; i < BINS; ++i) {
        std::cout << "Бин " << i << " (" << i * 0.1 << "-" << (i + 1) * 0.1 << "): " << custom_hist[i] << std::endl;
    }

    std::cout << "\nГистограмма (частоты в бинах) для библиотечного:" << std::endl;
    std::vector<int> lib_hist(BINS, 0);
    for (double num : lib_nums) {
        int bin = static_cast<int>(num * BINS);
        if (bin < BINS) lib_hist[bin]++;
    }
    for (int i = 0; i < BINS; ++i) {
        std::cout << "Бин " << i << " (" << i * 0.1 << "-" << (i + 1) * 0.1 << "): " << lib_hist[i] << std::endl;
    }

    // Тест 2: Статистический тест на равномерность (хи-квадрат)
    // Шаг: Считаем observed частоты (из hist), expected = N / BINS, chi2 = sum((obs - exp)^2 / exp)
    auto chi_square_test = [](const std::vector<int>& hist, int n, int bins) -> double {
        double expected = static_cast<double>(n) / bins;
        double chi2 = 0.0;
        for (int count : hist) {
            chi2 += std::pow(count - expected, 2) / expected;
        }
        return chi2;
        };

    double custom_chi2 = chi_square_test(custom_hist, N, BINS);
    double lib_chi2 = chi_square_test(lib_hist, N, BINS);
    std::cout << "\nНаш ГПСЧ: Хи-квадрат = " << custom_chi2 << std::endl;
    std::cout << "Библиотечный: Хи-квадрат = " << lib_chi2 << std::endl;
    // Примечание: Для p-value нужен таблица или аппроксимация, но для простоты только chi2 (меньше ~ значение для BINS-1 df лучше)

    // Тест 3: Независимость (автокорреляция с лагом 1)
    // Шаг: Вычисляем среднее, затем corr = sum((x_i - mean)*(x_{i+1} - mean)) / (var * (n-1))
    auto autocorrelation = [](const std::vector<double>& nums) -> double {
        int n = nums.size();
        double mean = 0.0;
        for (double num : nums) mean += num;
        mean /= n;

        double var = 0.0;
        for (double num : nums) var += std::pow(num - mean, 2);
        var /= n;

        double cov = 0.0;
        for (int i = 0; i < n - 1; ++i) {
            cov += (nums[i] - mean) * (nums[i + 1] - mean);
        }
        cov /= (n - 1);

        return cov / var;
        };

    double custom_ac = autocorrelation(custom_nums);
    double lib_ac = autocorrelation(lib_nums);
    std::cout << "\nНаш ГПСЧ: Автокорреляция (лаг 1) = " << custom_ac << std::endl;
    std::cout << "Библиотечный: Автокорреляция (лаг 1) = " << lib_ac << std::endl;

    // Тест 4: Поиск периода (простая проверка повторений в длинной последовательности)
    // Шаг: Генерируем длинную последовательность (100000), ищем цикл с помощью карты
    auto find_period = [](const std::vector<double>& nums) -> int {
        std::unordered_map<double, int> seen;
        for (int i = 0; i < nums.size(); ++i) {
            if (seen.count(nums[i])) {
                int start = seen[nums[i]];
                bool match = true;
                int len = i - start;
                for (int j = 0; j < len; ++j) {
                    if (i + j >= nums.size() || nums[start + j] != nums[i + j]) {
                        match = false;
                        break;
                    }
                }
                if (match) return len;
            }
            seen[nums[i]] = i;
        }
        return -1;  // Не найден
        };

    // Генерируем длинную последовательность
    float* long_seq_ptr = custom_uniform(100000);
    std::vector<double> long_seq(100000);
    for (int i = 0; i < 100000; ++i) {
        long_seq[i] = static_cast<double>(long_seq_ptr[i]);
    }
    delete[] long_seq_ptr;  // Освобождаем

    int period = find_period(long_seq);
    std::cout << "\nПериод нашего ГПСЧ: " << (period != -1 ? std::to_string(period) : "Не найден в 100000 числах") << std::endl;

    // --- Добавлено: Тестирование нормального распределения N(0,1) ---
    float* custom_normal_ptr = custom_normal(N, 0.0f, 1.0f);  // mean=0, stddev=1, num_uniforms=12
    std::vector<double> custom_normal_nums(N);
    for (int i = 0; i < N; ++i) {
        custom_normal_nums[i] = static_cast<double>(custom_normal_ptr[i]);
    }
    delete[] custom_normal_ptr;

    // Гистограмма для нормального (бины от -3 до 3, шаг 0.6)
    std::cout << "\nГистограмма (частоты в бинах) для нашего нормального N(0,1):" << std::endl;
    std::vector<int> custom_normal_hist(BINS, 0);
    float min_val = -3.0f;
    float bin_width = 6.0f / BINS;  // От -3 до 3
    for (double num : custom_normal_nums) {
        int bin = static_cast<int>((num - min_val) / bin_width);
        if (bin >= 0 && bin < BINS) custom_normal_hist[bin]++;
    }
    for (int i = 0; i < BINS; ++i) {
        float low = min_val + i * bin_width;
        float high = low + bin_width;
        std::cout << "Бин " << i << " (" << low << "-" << high << "): " << custom_normal_hist[i] << std::endl;
    }

    // --- Добавлено: Тестирование экспоненциального с lambda=1 ---
    float* custom_expo_ptr = custom_exponential(N, 1.0f);  // lambda=1 (mean=1)
    std::vector<double> custom_expo_nums(N);
    for (int i = 0; i < N; ++i) {
        custom_expo_nums[i] = static_cast<double>(custom_expo_ptr[i]);
    }
    delete[] custom_expo_ptr;

    // Гистограмма для экспоненциального (бины от 0 до 5, шаг 0.5)
    std::cout << "\nГистограмма (частоты в бинах) для нашего экспоненциального (lambda=1):" << std::endl;
    std::vector<int> custom_expo_hist(BINS, 0);
    float expo_max = 5.0f;
    float expo_bin_width = expo_max / BINS;
    for (double num : custom_expo_nums) {
        if (num >= 0) {  // Экспоненциальное >=0
            int bin = static_cast<int>(num / expo_bin_width);
            if (bin < BINS) custom_expo_hist[bin]++;
        }
    }
    for (int i = 0; i < BINS; ++i) {
        float low = i * expo_bin_width;
        float high = low + expo_bin_width;
        std::cout << "Бин " << i << " (" << low << "-" << high << "): " << custom_expo_hist[i] << std::endl;
    }

    // Можно добавить автокорреляцию для новых, аналогично
    double custom_normal_ac = autocorrelation(custom_normal_nums);
    std::cout << "\nАвтокорреляция (лаг 1) для нормального: " << custom_normal_ac << std::endl;

    double custom_expo_ac = autocorrelation(custom_expo_nums);
    std::cout << "Автокорреляция (лаг 1) для экспоненциального: " << custom_expo_ac << std::endl;

    return 0;
}
