#pragma once
#include <iostream>
#include <cstdlib>

using namespace std;

class Matrix {
private:
  int dy; //행렬의 y축 길이
  int dx; //행렬의 x축 길이
  int **array; //2차 배열(Screen) 포인터
  void alloc(int cy, int cx); //주어진 y축,x축 길이만큼 2차원 배열 동적할당 하는 함수
public:
  int get_dy(); //y축 길이 가져오기
  int get_dx(); //x축 길이 가져오기
  int** get_array(); //2차 배열(Screen) 가져오기
  Matrix(); //생성자
  Matrix(int cy, int cx); //x,y축 설정 생성자
  Matrix(const Matrix *obj); //copy생성자
  Matrix(const Matrix &obj); //copy생성자
  Matrix(int *arr, int col, int row); //블록을 행렬로 저장하기 위한 생성자
  ~Matrix(); //소멸자
  Matrix *clip(int top, int left, int bottom, int right); //행렬 자르기
  void paste(const Matrix *obj, int top, int left); //행렬 붙여넣기
  Matrix *add(const Matrix *obj); //행렬끼리 덧셈
  int sum(); //총 합 계산
  void mulc(int coef); //행렬에 상수배 곱함
  Matrix *binary(); //현재 행렬을 이용해 원소가 0과 1로만 이루어진 행렬을 반환해주는 함수
  bool anyGreaterThan(int val); //행렬에 val값보다 큰 값이 있는지 확인
  void print(); //행렬 출력
  friend ostream& operator<<(ostream& out, const Matrix& obj); //<<연산자 오버로딩
  Matrix& operator=(const Matrix& obj); //=연산자 오버로딩
};
