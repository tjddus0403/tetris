class MatrixError(Exception):
    pass
#어떻게 보면 x,y가 행,열 일 수 있고 어떻게 보면 열,행 일 수 있음 (나는 열,행으로 볼 거임)
class Matrix:
    nAlloc = 0
    nFree = 0
    
    #Matrix의 할당된 ? 갯수
    def get_nAlloc(self):
        return Matrix.nAlloc
    
    #Matrix의 해제된 ? 갯수
    def get_nFree(self):
        return Matrix.nFree
    
    #블록의 y좌표(행좌표) 받기
    def get_dy(self):
        return self._dy
    
    #블록의 x좌표(열좌표) 받기
    def get_dx(self):
        return self._dx
    
    #블록의 배열 받기
    def get_array(self):
        return self._array
    
    #Matrix ? 해제
    def __del__(self):
        Matrix.nFree += 1
    
    #블록을 Matrix에 할당(붙여넣기) 
    def __alloc(self, cy, cx):
        if cy < 0 or cx < 0:
            raise MatrixError("wrong matrix size")
        self._dy = cy
        self._dx = cx
        self._array = [[0]*self._dx for i in range(self._dy)]
        #print(self.__array)
        Matrix.nAlloc += 1
        
    #Matrix 생성자      
    def __init__(self, arg):
        if isinstance(arg, list):
            array = arg
            cy = len(array)
            cx = len(array[0])
            self.__alloc(cy, cx)
            for y in range(cy):
                for x in range(cx):
                    self._array[y][x] = array[y][x]
            return
        elif isinstance(arg, Matrix):
            other = arg
            cy = other._dy
            cx = other._dx
            self.__alloc(cy, cx)
            for y in range(cy):
                for x in range(cx):
                    self._array[y][x] = other._array[y][x]
            return
        else:
            self.__alloc(0, 0)
            return
    
    def __str__(self):
        return 'Matrix(%d, %d)' % (self._dy, self._dx)

    #Matrix(self) 출력
    def print(self):
        print('[', end=' ')
        for y in range(self._dy-1):
            print('[', end=' ')
            for x in range(self._dx-1):
                print(self._array[y][x], end=', ')
            print(self._array[y][self._dx-1], end=' ')
            print('],', end=' ')
        print('[', end=' ')
        for x in range(self._dx-1):
            print(self._array[self._dy-1][x], end=', ')
        print(self._array[self._dy-1][self._dx-1], end=' ')
        print(']', end=' ')
        print(']')        
        return

    #Matrix(self)의 일정부분 잘라서 복사
    def clip(self, top, left, bottom, right):
        cy = bottom - top
        cx = right - left
        temp = [[0]*cx for i in range(cy)]       
        for y in range(cy):
            for x in range(cx):
                if (top+y >= 0) and (left+x >= 0) \
                   and (top+y < self._dy) and (left+x < self._dx):
                    temp[y][x] = self._array[top+y][left+x]
                else:
                    raise MatrixError("invalid matrix range")
        return Matrix(temp)
   
    #Matrix(self)에 other 붙여넣기
    def paste(self, other, top, left):
        for y in range(other._dy):
            for x in range(other._dx):
                if (top+y >= 0) and (left+x >= 0) \
                   and (top+y < self._dy) and (left+x < self._dx):
                    self._array[top+y][left+x] = other._array[y][x]
                else:
                    raise MatrixError("invalid matrix range")

    
    #Matrix 덧셈 (self+other)
    def __add__(self, other):
        if (self._dx != other._dx) or (self._dy != other._dy):
            raise MatrixError("matrix sizes mismatch")
        temp = [[0]*self._dx for i in range(self._dy)]
        for y in range(self._dy):
            for x in range(self._dx):
                temp[y][x] = self._array[y][x] + other._array[y][x]                
        return Matrix(temp)
    
    #Matrix 각 원소의 총 합
    def sum(self):
        total = 0
        for y in range(self._dy):
            for x in range(self._dx):
                total += self._array[y][x]
        return total
    
    #Matrix에 상수 곱하기 (self*coef)
    def mulc(self, coef):
        for y in range(self._dy):
            for x in range(self._dx):
                self._array[y][x] *= coef
    
    #Matrix의 각 원소가 특정 값보다 큰지 확인하기 (self>val=?)
    def anyGreaterThan(self, val):
        for y in range(self._dy):
            temp = [v for v in self._array[y] if v > val]
            if len(temp) > 0:
                return True
        return False
    
    #Matrix의 각 원소 값을 이진수로 나타내기 (0,1)
        temp = Matrix(self)
        for y in range(self._dy):
            for x in range(self._dx):
                if temp._array[y][x] != 0:
                    temp._array[y][x] = 1
        return temp

### end of Matrix class

