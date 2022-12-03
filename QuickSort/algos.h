#pragma once

#include <algorithm>
#include <vector>
#include <tbb/tbb.h>

void quicksortSeq(std::vector<int>::iterator first, std::vector<int>::iterator last)
{
    if (first == last) return;

    auto pivot = *std::next(first, std::distance(first, last) / 2);

    auto middle1 = std::partition(first, last, [pivot](const auto& em) {
        return em < pivot;
        });
    auto middle2 = std::partition(middle1, last, [pivot](const auto& em) {
        return !(pivot < em);
        });

    quicksortSeq(first, middle1);
    quicksortSeq(middle2, last);
}

void quicksort(std::vector<int>::iterator first, std::vector<int>::iterator last)
{
    if (first == last) return;

    auto pivot = *std::next(first, std::distance(first, last) / 2);

    auto middle1 = std::partition(first, last, [pivot](const auto& em) {
        return em < pivot;
        });
    auto middle2 = std::partition(middle1, last, [pivot](const auto& em) {
        return !(pivot < em);
        });
    tbb::task_group g;
    if (std::distance(first, last) > 10000) {
        g.run([&] {quicksort(first, middle1); });
        g.run([&] {quicksort(middle2, last); });

        g.wait();
    }
    else {
        quicksortSeq(first, middle1);
        quicksortSeq(middle2, last);
    }
}