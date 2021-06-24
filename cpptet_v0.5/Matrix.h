#pragma once
#include <iostream>
#include <cstdlib>

using namespace std;

class Matrix {
private:
  int dy; //행렬의 y축 길이
  int dx; //행렬의 x축 길이
  int **array; //2차 배열
  void alloc(int cy, int cx); 
public:
  int get_dy();
  int get_dx();
  int** get_array();
  Matrix();
  Matrix(int cy, int cx);
  Matrix(const Matrix *obj);
  Matrix(const Matrix &obj);
  Matrix(int *arr, int col, int row);
  ~Matrix();
  Matrix *clip(int top, int left, int bottom, int right);
  void paste(const Matrix *obj, int top, int left);
  Matrix *add(const Matrix *obj);
  int sum();
  void mulc(int coef);
  Matrix *binary();
  bool anyGreaterThan(int val);
  void print();
  friend ostream& operator<<(ostream& out, const Matrix& obj);
  Matrix& operator=(const Matrix& obj);
};
