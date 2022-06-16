#pragma once
#include <codecvt>
#include <locale>
#include <string>
#include <vector>
std::wstring s2ws(const std::string &str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring &wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
}
template <typename T>
double mean_v(const T &data)
{
    double ret = 0;
    for (const auto &i : data)
    {
        ret += i.value;
    }
    return ret / data.size();
}
template <typename T>
double deviation(const T &data)
{
    double mean = 0, ret = 0;
    for (const auto &i : data)
    {
        mean += i.value;
    }
    mean /= data.size();
    for (const auto &i : data)
    {
        ret += pow(i.value - mean, 2);
    }
    ret /= (data.size() - 1);
    return sqrt(ret);
}

template <typename T>
double deviation_m(const T &data, double mean)
{
    double ret = 0;
    for (const auto &i : data)
    {
        ret += pow(i.value - mean, 2);
    }
    ret /= (data.size() - 1);
    return sqrt(ret);
}