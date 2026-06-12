#pragma once
#include <vector>

class Matrix
{
public:
    Matrix(const int row, const int col, bool random=false);

    inline int row() const {
        return m_row;
    }
    inline int col() const {
        return m_col;
    }

    inline void setData(const std::vector<std::vector<float>>& data)
    {
        m_data = data;
    }
    inline float& operator()(size_t row, size_t col) {
        return m_data[row][col];
    }
    inline float operator()(size_t row, size_t col) const {
        return m_data[row][col];
    }
    inline std::vector<float>& operator()(size_t row) {
        return m_data[row];
    }
    inline std::vector<float> operator()(size_t row) const {
        return m_data[row];
    }

    Matrix operator+(Matrix b);
    Matrix operator-(Matrix b);
    
    template<typename T> static Matrix vec1dToMat(const std::vector<T>& data)
    {
        int size = data.size();
        Matrix mat(size, 1, false);
        for (int i = 0; i < size; i++)
        {
            mat.m_data[i][0] = data[i];
        }

        return std::move(mat);
    }

    template<typename T> static Matrix vec2dToMat(const std::vector<std::vector<T>>& data)
    {
        Matrix mat(data.size(), data[0].size());
        mat.m_data = data;
        return std::move(mat);
    }

    template<typename T> static std::vector<float> vec2dToVec1d(const std::vector<std::vector<T>>& data)
    {
        std::vector<float> res;
        for (const std::vector<T>& row : data)
        {
            for (float ele : row)
            {
                res.push_back(ele);
            }
        }
        return res;
    }


    static Matrix dot(const Matrix& a, const Matrix& b);
    static void dot(const Matrix& a, const Matrix& b, Matrix& c);

    void print(int x=0, int y=0);

private:
    int m_row;
    int m_col;
    std::vector<std::vector<float>> m_data;
};