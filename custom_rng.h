#pragma once
#ifndef CUSTOM_RNG_H
#define CUSTOM_RNG_H

#include <vector>  // Не нужен теперь, но оставлю для совместимости, если тест использует vector

// Функция для равномерного распределения [0, 1)
float* custom_uniform(int n);

// Функция для нормального распределения N(mean, stddev^2)
float* custom_normal(int n, float mean, float stddev, int num_uniforms = 12);

// Функция для экспоненциального распределения с параметром lambda
float* custom_exponential(int n, float lambda);

#endif
