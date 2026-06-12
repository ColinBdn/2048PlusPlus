#include "../include/matrix.h"
#include "../../../base/include/useful.h"
#include "iostream"
#include <string>


Matrix::Matrix(const int row, const int col, const bool random)
{
	m_row = row;
	m_col = col;
	m_data.reserve(row);
	for (size_t i = 0; i < row; i++) {
		if (random)
		{
			m_data.emplace_back(col);
			for (size_t j = 0; j < col; j++)
			{
				m_data[i][j] = genRandomFloat(-1, 1);
			}
		}
		else
		{
			m_data.emplace_back(col, 0);
		}
	}
}

void Matrix::print(int x, int y) {
	for (const auto& row : m_data) {
		std::string row_str="";
		for (const float& ele : row) {
			row_str+=std::to_string(ele)+" ";
		}
		printAtPos(row_str, x, y);
		y++;
	}
}

Matrix Matrix::dot(const Matrix& a, const Matrix& b)
{
	int row = a.row();
	int col = b.col();
	Matrix res(row, col, false);

	for (int i = 0; i < row; i++) {
		for (int k = 0; k < b.row(); k++) {
			for (int j = 0; j < col; j++) {
				res.m_data[i][j] += a(i, k) * b(k, j);
			}
		}
	}

	return res;
}
void Matrix::dot(const Matrix& a, const Matrix& b, Matrix& c)
{
	int row = a.row();
	int col = b.col();
	if (c.m_row != row || c.m_col != col)
	{
		std::cout << "can't dot product, dim doesn't match !";
		exit(1);
	}

	for (auto& row : c.m_data) {
		std::fill(row.begin(), row.end(), 0);
	}

	for (int i = 0; i < row; i++) {
		for (int k = 0; k < b.row(); k++) {
			for (int j = 0; j < col; j++) {
				c.m_data[i][j] += a(i, k) * b(k, j);
			}
		}
	}

	//for (int i = 0; i < row; i++)
	//{
	//	for (int j = 0; j < col; j++)
	//	{
	//		for (int k = 0; k < b.row(); k++)
	//			c.m_data[i][j] += a(i, k) * b(k, j);
	//	}
	//}

}