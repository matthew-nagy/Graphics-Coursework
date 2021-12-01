#pragma once

#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>


template<class T>
std::vector<T> operator+(const std::vector<T>& left, const std::vector<T>& right){
    std::vector<T> ret;
    ret.reserve(left.size());
    for(size_t i = 0; i < left.size(); i++)
        ret.emplace_back(left[i] + right[i]);
    return ret;
}
template<class T>
std::vector<T> operator-(const std::vector<T>& left, const std::vector<T>& right){
    std::vector<T> ret;
    ret.reserve(left.size());
    for(size_t i = 0; i < left.size(); i++)
        ret.emplace_back(left[i] - right[i]);
    return ret;
}

template<class T>
std::vector<T> operator/(const std::vector<T>& left, float right){
    std::vector<T> ret;
    ret.reserve(left.size());
    for(size_t i = 0; i < left.size(); i++)
        ret.emplace_back(left[i] / right);
    return ret;
}

template<class T>
std::vector<std::vector<T>> interpolate(const std::vector<T>& from, const std::vector<T>& to, int numberOfValues){
    numberOfValues = std::abs(numberOfValues);

    if(numberOfValues > 10000)
        return std::vector<std::vector<T>>{from};

    std::vector<std::vector<T>> values;
    values.reserve(numberOfValues);

    auto delta = (to - from) / (numberOfValues - 1);
    std::vector<T> step = from;
    for(size_t i = 0; i < numberOfValues; i++){
        values.emplace_back(step);
        step = step + delta;
    }

    return values;
}

template<class T>
void xAdjustLine(std::vector<std::vector<T>>& values){
    if(values.size() <= 1)return;

    std::vector<std::vector<T>> newVals;

    values[0][0] = std::round(values[0][0]);
    for(size_t i = 1; i < values.size(); i++){
        values[i][0] = std::round(values[i][0]);
        if(values[i][0] > values[i-1][0]){
            if((values[i-1][0] + 1) != values[i][0]){
                auto valCopy = values[i-1];
                valCopy[0] += 1;
                newVals.emplace_back(valCopy);
            }

        }
    }

    if(newVals.size() != 0)
        values.insert(values.end(), newVals.begin(), newVals.end());
}

typedef std::vector<float> vf;
typedef std::vector<int> vi;