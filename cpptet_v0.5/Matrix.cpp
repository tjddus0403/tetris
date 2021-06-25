#include "Matrix.h"

void Matrix::alloc(int cy, int cx) { //2차원 배열 동적할당하는 함수
  if ((cy < 0) || (cx < 0)) return;
  dy = cy;
  dx = cx;
  array = new int*[dy]; //y축 동적할당
  for (int y = 0; y < dy; y++) //반복문 통해 x축 동적할당
    array[y] = new int[dx]; 
  for (int y = 0; y < dy; y++) //2차원 배열 모든 값 0으로 초기화
    for (int x = 0; x < dx; x++)
      array[y][x] = 0;
}

int Matrix::get_dy() { return dy; } 

int Matrix::get_dx() { return dx; }

int **Matrix::get_array() { return array; }

Matrix::Matrix() { alloc(0, 0); }

Matrix::~Matrix() { 
  for (int y = 0; y < dy; y++)
    delete array[y];
  delete array;
}

Matrix::Matrix(int cy, int cx) {
  alloc(cy, cx);
  for (int y = 0; y < dy; y++) //이건 alloc에도 있어서 안해도 될듯?
    for (int x = 0; x < dx; x++)
      array[y][x] = 0;
}

Matrix::Matrix(const Matrix *obj) { //포인터 통한 copy constructor
  alloc(obj->dy, obj->dx);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = obj->array[y][x];
}

Matrix::Matrix(const Matrix &obj) { //참조자 통한 copy constructor
  alloc(obj.dy, obj.dx);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = obj.array[y][x];
}

Matrix::Matrix(int *arr, int col, int row) { //1차원 배열로 저장되어있는 블록 2차원 행렬로 생성
  alloc(col, row);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = arr[y * dx + x];
}

Matrix *Matrix::clip(int top, int left, int bottom, int right) { //행렬을 부분적으로 자르는 함수
  int cy = bottom - top;
  int cx = right - left;
  Matrix *temp = new Matrix(cy, cx);
  for (int y = 0; y < cy; y++) {
    for (int x = 0; x < cx; x++) {
      if ((top + y >= 0) && (left + x >= 0) &&
	  (top + y < dy) && (left + x < dx))
	temp->array[y][x] = array[top + y][left + x];
      else {
	cerr << "invalid matrix range";
	return NULL;
      }
    }
  }
  return temp; //자른 행렬 반환
}

void Matrix::paste(const Matrix *obj, int top, int left) { //다른 행렬을 현재 행렬에 붙여주는 함수
  for (int y = 0; y < obj->dy; y++)
    for (int x = 0; x < obj->dx; x++) {
      if ((top + y >= 0) && (left + x >= 0) &&
	  (top + y < dy) && (left + x < dx))
	array[y + top][x + left] = obj->array[y][x];
      else {
	cerr << "invalid matrix range";
	return NULL;
      }
    }
}

Matrix *Matrix::add(const Matrix *obj) { //현재행렬에 다른 행렬의 값을 더하는 함수
  if ((dx != obj->dx) || (dy != obj->dy)) return NULL;
  Matrix *temp = new Matrix(dy, dx);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      temp->array[y][x] = array[y][x] + obj->array[y][x];
  return temp; //더한 후의 행렬 반환
}

int Matrix::sum() { //행렬에 있는 모든 원소의 합 반환해주는 함수
  int total = 0;
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      total += array[y][x];
  return total;
}

void Matrix::mulc(int coef) { //현재 행렬의 모든 원소에 coef만큼 상수배 곱해주는 함수
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = coef * array[y][x];
}

Matrix *Matrix::binary() { //현재 행렬을 이용해 원소가 0과 1로만 이루어진 행렬을 반환해주는 함수
  Matrix *temp = new Matrix(dy, dx);
  int **t_array = temp->get_array();
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      t_array[y][x] = (array[y][x] != 0 ? 1 : 0); //0이면 0, 0이 아니면 1
  
  return temp;
}

bool Matrix::anyGreaterThan(int val) { //행렬에 val보다 큰 값이 있는지 확인하는 함수
  for (int y = 0; y < dy; y++) {
    for (int x = 0; x < dx; x++) {
      if (array[y][x] > val)
	return true; //있으면 true
    }
  }
  return false; //없으면 false를 반환
}

void Matrix::print() { //행렬 출력 함수
  cout << "Matrix(" << dy << "," << dx << ")" << endl;
  for (int y = 0; y < dy; y++) {
    for (int x = 0; x < dx; x++)
      cout << array[y][x] << " ";
    cout << endl;
  }
}


ostream& operator<<(ostream& out, const Matrix& obj){ //<<연산자 오버로딩
  out << "Matrix(" << obj.dy << "," << obj.dx << ")" << endl;
  for(int y = 0; y < obj.dy; y++){
    for(int x = 0; x < obj.dx; x++)
      out << obj.array[y][x] << " ";
    out << endl;
  }
  out << endl; //어차피 friend 함수니까 obj.print()쓰면 안되는건가..?
  return out;
}

Matrix& Matrix::operator=(const Matrix& obj) //=연산자 오버로딩
{
  if (this == &obj) return *this; 
  if ((dx != obj.dx) || (dy != obj.dy)) 
    alloc(obj.dy, obj.dx);

  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = obj.array[y][x];
  return *this;
}
