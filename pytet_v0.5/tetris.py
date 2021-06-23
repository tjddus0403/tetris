from matrix import *
from enum import Enum

#Enum을 상속받는 클래스, tetris 게임의 현재 상황 나타냄
class TetrisState(Enum):
    Running = 0
    NewBlock = 1
    Finished = 2
### end of class TetrisState():

#tetris 게임 클래스
class Tetris():
    nBlockTypes = 0
    nBlockDegrees = 0
    setOfBlockObjects = 0
    iScreenDw = 0   # larget enough to cover the largest block

    @classmethod
    def init(cls, setOfBlockArrays):
        Tetris.nBlockTypes = len(setOfBlockArrays)
        Tetris.nBlockDegrees = len(setOfBlockArrays[0])
        Tetris.setOfBlockObjects = [[0] * Tetris.nBlockDegrees for _ in range(Tetris.nBlockTypes)]
        arrayBlk_maxSize = 0
        for i in range(Tetris.nBlockTypes):
            if arrayBlk_maxSize <= len(setOfBlockArrays[i][0]):
                arrayBlk_maxSize = len(setOfBlockArrays[i][0])
        Tetris.iScreenDw = arrayBlk_maxSize     # larget enough to cover the largest block

        for i in range(Tetris.nBlockTypes):
            for j in range(Tetris.nBlockDegrees):
                Tetris.setOfBlockObjects[i][j] = Matrix(setOfBlockArrays[i][j])
        return

    def createArrayScreen(self):
        self.arrayScreenDx = Tetris.iScreenDw * 2 + self.iScreenDx #블록의 maximum size 고려해서 Screen의 x축의 크기 설정
        self.arrayScreenDy = self.iScreenDy + Tetris.iScreenDw #블록의 maximum size 고려해서 Screen의 y축의 크기 설정
        self.arrayScreen = [[0] * self.arrayScreenDx for _ in range(self.arrayScreenDy)] 
        for y in range(self.iScreenDy):
            for x in range(Tetris.iScreenDw): #Screen의 왼쪽 벽은 숫자 1로 설정(채워져있음을 의미)
                self.arrayScreen[y][x] = 1
            for x in range(self.iScreenDx): #게임 진행되는 Screen은 0으로 설정(비어있음을 의미)
                self.arrayScreen[y][Tetris.iScreenDw + x] = 0
            for x in range(Tetris.iScreenDw): #Screen의 오른쪽 벽은 숫자 1로 설정(채워져있음을 의미)
                self.arrayScreen[y][Tetris.iScreenDw + self.iScreenDx + x] = 1

        for y in range(Tetris.iScreenDw): #Screen의 아래쪽 벽은 숫자 1로 설정(채워져있음을 의미)
            for x in range(self.arrayScreenDx):
                self.arrayScreen[self.iScreenDy + y][x] = 1

        return self.arrayScreen 

    def __init__(self, iScreenDy, iScreenDx):  #Tetris 생성자
        self.iScreenDy = iScreenDy #인자로 받은 y축 크기를 Tetris Screen(보여지는거)의 y축 크기로 설정 
        self.iScreenDx = iScreenDx #인자로 받은 x축 크기를 Tetris Screen(보여지는거)의 x축 크기로 설정
        self.idxBlockDegree = 0 #블록은 원래 형태(0->원상태, 1->90도 회전, 2->180도 회전, 3->360도 회전)
        arrayScreen = self.createArrayScreen() #Tetris의 Screen생성
        self.iScreen = Matrix(arrayScreen) #Matrix type의 input Screen 생성
        self.oScreen = Matrix(self.iScreen) #Matrix type의 output Screen 생성
        self.justStarted = True #justStarted=True로 초기화->막 시작한 상태(=블록이 생성되어야하는 상태)
        return

    def accept(self, key): #key값 받기 (블록이 새로 생성될 때의 key=블록 모양 결정 단, key값은 '01','02','03','04','05','06'중 하나
                                      #블록 조절할 때의 key=블록 위치, 회전 설정)
        self.state = TetrisState.Running #Tetris 게임 진행 상태 = 진행 중

        if key >= '0' and key <= '6': #블록이 새로 생성될 때!!! key값이 '0'보다 크거나 같고 '6'보다 작거나 같으면 
            if self.justStarted == False: #justStarted가 False값 가지면 deleteFullLines실행
                self.deleteFullLines()
            self.iScreen = Matrix(self.oScreen) #현재의 output Screen을 Matrix type으로 받아 input Screen 설정
            self.idxBlockType = int(key) #key값을 정수형으로 바꾸어 현재 블록의 종류 결정 (key=0,1,2,3,4,5,6 중 하나)
            self.idxBlockDegree = 0 #현재 블록의 회전 상태 0으로 설정
            self.currBlk = Tetris.setOfBlockObjects[self.idxBlockType][self.idxBlockDegree] 
            #위에서 설정한 현재 블록의 종류와 회전 상태로 현재 블록 설정
            self.top = 0 #현재 블록의 위치 top을 0으로 초기화(제일 위에 있는 상태->지금 막 생성된 상태)
            self.left = Tetris.iScreenDw + self.iScreenDx//2 - self.currBlk.get_dx()//2 
            #현재 블록의 위치 left를 Screen의 중앙으로 초기화(제일 가운데 있는 상태->지금 막 생성된 상태)
            self.tempBlk = self.iScreen.clip(self.top, self.left, self.top+self.currBlk.get_dy(), self.left+self.currBlk.get_dx())
            #현재 input Screen에서 현재 블록의 위치와 크기만큼 잘라 임시블록 생성
            self.tempBlk = self.tempBlk + self.currBlk #임시블록에 현재 블록 더하기
            self.justStarted = False #justStarted=False로 설정->블록 새로 생성될 필요 없는 상태
            print() 

            if self.tempBlk.anyGreaterThan(1): #임시블록에 1보다 큰수가 존재하면 Tetris 게임 종료
                self.state = TetrisState.Finished
            self.oScreen = Matrix(self.iScreen) #현재 input Screen을 Matrix type으로 받아 output Screen 설정
            self.oScreen.paste(self.tempBlk, self.top, self.left) #현재 output Screen에 임시블록 붙여넣기
            return self.state #현재 Tetris 게임 진행 상태 반환
        
        elif key == 'q': #key='q'이면 pass(main에서 Tetris 게임 종료시킴)
            pass
        elif key == 'a': # move left
            self.left -= 1
        elif key == 'd': # move right
            self.left += 1
        elif key == 's': # move down
            self.top += 1
        elif key == 'w': # rotate the block clockwise
            self.idxBlockDegree = (self.idxBlockDegree + 1) % Tetris.nBlockDegrees
            self.currBlk = Tetris.setOfBlockObjects[self.idxBlockType][self.idxBlockDegree]
        elif key == ' ': # drop the block
            while not self.tempBlk.anyGreaterThan(1): #Screen 맨 밑까지 내려야하니까
                    self.top += 1
                    self.tempBlk = self.iScreen.clip(self.top, self.left, self.top+self.currBlk.get_dy(), self.left+self.currBlk.get_dx())
                    self.tempBlk = self.tempBlk + self.currBlk
        else:
            print('Wrong key!!!')
            
        self.tempBlk = self.iScreen.clip(self.top, self.left, self.top+self.currBlk.get_dy(), self.left+self.currBlk.get_dx())
        self.tempBlk = self.tempBlk + self.currBlk

        if self.tempBlk.anyGreaterThan(1):   ## 벽 충돌시 undo 수행
            if key == 'a': # undo: move right
                self.left += 1
            elif key == 'd': # undo: move left
                self.left -= 1
            elif key == 's': # undo: move up
                self.top -= 1
                self.state = TetrisState.NewBlock #Screen 맨 밑까지 내려갔으니까 새로운 블록 필요 (Tetris 게임 상태 NewBlock으로 변경)
            elif key == 'w': # undo: rotate the block counter-clockwise
                self.idxBlockDegree = (self.idxBlockDegree - 1) % Tetris.nBlockDegrees
                self.currBlk = Tetris.setOfBlockObjects[self.idxBlockType][self.idxBlockDegree]
            elif key == ' ': # undo: move up
                self.top -= 1
                self.state = TetrisState.NewBlock #Screen 맨 밑까지 내려갔으니까 새로운 블록 필요 (Tetris 게임 상태 NewBlock으로 변경)
            
            self.tempBlk = self.iScreen.clip(self.top, self.left, self.top+self.currBlk.get_dy(), self.left+self.currBlk.get_dx())
            self.tempBlk = self.tempBlk + self.currBlk 

        self.oScreen = Matrix(self.iScreen) #현재 input Screen을 Matrix type으로 받아 output Screen설정
        self.oScreen.paste(self.tempBlk, self.top, self.left) #현재 output Screen에 임시블록 붙여넣기

        return self.state #현재 Tetris 게임 진행 상태 반환

    def deleteFullLines(self):
        nDeleted = 0
        nScanned = self.currBlk.get_dy()

        if self.top + self.currBlk.get_dy() - 1 >= self.iScreenDy:
            nScanned -= (self.top + self.currBlk.get_dy() - self.iScreenDy)

        zero = Matrix([[ 0 for x in range(0, (self.iScreenDx - 2*Tetris.iScreenDw))]])
        for y in range(nScanned - 1, -1, -1):
            cy = self.top + y + nDeleted
            line = self.oScreen.clip(cy, 0, cy+1, self.oScreen.get_dx())
            if line.sum() == self.oScreen.get_dx():
                temp = self.oScreen.clip(0, 0, cy, self.oScreen.get_dx())
                self.oScreen.paste(temp, 1, 0)
                self.oScreen.paste(zero, 0, Tetris.iScreenDw)
                nDeleted += 1

        return nScanned

### end of class Tetris():
    
