from tetris import * 
from random import *

import os #운영체제 제어 모듈
import sys #파이썬 인터프리터 제어 모듈
import tty #터미널 제어 모듈
import termios #
import signal #시그널 처리 모듈

#화면 지우는 함수
def clearScreen(numlines=100): 
	if os.name == 'posix': #os.name의 리턴 posix는 우분투 등의 리눅스 운영체제를 의미함 (cf. os.name의 리턴 nt는 윈도우 운영체제 의미)
		os.system('clear') #os모듈의 system()함수는 python이 컴퓨터의 운영체제에게 명령어를 전달하기 위한 일종의 번역기능을 함. 
		#즉, 여기서는 우분투 환경에서 clear실행하라는거임
	elif os.name in ['nt', 'dos', 'ce']: #os.name의 리턴이 윈도우 운영체제 의미하면,
		os.system('CLS') #윈도우 환경에서 CLS실행 (clear의미)
	else:
		print('\n' * numlines) #둘다 아니면 공백 100줄
	return

#화면 출력 함수
def printScreen(board):
	clearScreen() #화면 지우기 
	array = board.oScreen.get_array() #출력하려는 Tetris객체(board)의 oScreen 배열을 array에 저장
	for y in range(board.oScreen.get_dy()-Tetris.iScreenDw): #출력하려는 Tetris객체(board)의 oScreen의 y좌표에서 
		line = ''
		for x in range(Tetris.iScreenDw, board.oScreen.get_dx()-Tetris.iScreenDw):
			if array[y][x] == 0:
				line += '□'
			elif array[y][x] == 1:
				line += '■'
			else:
				line += 'XX'
		print(line)

	print()
	return

def unregisterAlarm(): #알람 초기화 함수(예약된 알람이 아무것도 없는 상태)
	signal.alarm(0) 
	#alarm은 SIGALRM신호가 초 단위로 호출 프로세스에 전달되도록 정렬함
	#초가 0인 경우, 알람 취소됨(알람이 없는거나 마찬가지)
	#예약된 알람이 없는 경우 0반환/ alarm은 이전에 예약된 알람이 전달될 때까지 남은 시간(초) 반환
	return 

def registerAlarm(handler, seconds):#알람 예약 함수(몇 초마다 handler의 강제오류 발생시킬건지..?)
	unregisterAlarm() #예약된 알람이 없는 상태 설정
	signal.signal(signal.SIGALRM, handler) #SIGALRM 처리기를 함수handler로 설정
	signal.alarm(seconds) #seconds초 alarm 예약->여기선 1초->반환값 1
	#1초 지나면 handler에 있는 런타임오류 강제발생 시켜주는건가...?이게 맞나...?
	return

def timeout_handler(signum, frame): 
	#print("timeout!")
	raise RuntimeError ### we have to raise error to wake up any blocking function
	#오류 강제 발생
	return

def getChar(): #문자열 받아오는 함수
	fd = sys.stdin.fileno() #파일 디스크립터 0이 표준 입력(stdin)을 나타냄. 따라서 fd=0 
	old_settings = termios.tcgetattr(fd) 
	try: #예외 발생 가능성이 있는 코드
		tty.setraw(sys.stdin.fileno())
		ch = sys.stdin.read(1)
		unregisterAlarm()
	finally: #예외 발생과 관련없이 무조건 실행할 코드
		termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
	return ch 
 
def readKey():
	c1 = getChar()
	if ord(c1) != 0x1b: ### ESC character
		return c1
	c2 = getChar()
	if ord(c2) != 0x5b: ### '[' character
		return c1
	c3 = getChar()
	return chr(0x10 + ord(c3) - 65)

def readKeyWithTimeOut(): #시간 흐름에 따른 key값 바꿔주는 함수
	registerAlarm(timeout_handler, 1) #1초마다 런타임 에러 발생
	try: #에러발생 가능한 코드
		key = readKey() 
		unregisterAlarm() #알람 초기화
		return key #키 값 반환
	except RuntimeError as e: #런타임 에러에 대한 예외 설정
		pass # print('readkey() interrupted!')

	return
 
def rotate(m_array): #블록 회전 함수
    size = len(m_array)
    r_array = [[0] * size for _ in range(size)]

    for y in range(size):
        for x in range(size):
            r_array[x][size-1-y] = m_array[y][x]

    return r_array

def initSetOfBlockArrays(): #블록세트 생성자(모든 상태의 블록을 담고 있음(블록 한 종류당 회전에 따라 4가지 상태 존재))
    global nBlocks

    arrayBlks = [ [ [ 0, 0, 1, 0 ],     # I shape
                    [ 0, 0, 1, 0 ],     
                    [ 0, 0, 1, 0 ],     
                    [ 0, 0, 1, 0 ] ],   
                  [ [1, 0, 0],          # J shape
                    [1, 1, 1],          
                    [0, 0, 0] ],
                  [ [0, 0, 1],          # L shape
                    [1, 1, 1],          
                    [0, 0, 0] ],        
                  [ [1, 1],             # O shape
                    [1, 1] ],           
                  [ [0, 1, 1],          # S shape
                    [1, 1, 0],          
                    [0, 0, 0] ],
                  [ [0, 1, 0],          # T shape    
                    [1, 1, 1],          
                    [0, 0, 0] ],
                  [ [1, 1, 0],          # Z shape
                    [0, 1, 1],          
                    [0, 0, 0] ]         
                ]

    nBlocks = len(arrayBlks) #블록 종류 수
    setOfBlockArrays = [[0] * 4 for _ in range(nBlocks)] 

    for idxBlockType in range(nBlocks):
        temp_array = arrayBlks[idxBlockType]
        setOfBlockArrays[idxBlockType][0] = temp_array
        for idxBlockDegree in range(1,4):
            temp_array = rotate(temp_array)
            setOfBlockArrays[idxBlockType][idxBlockDegree] = temp_array

    return setOfBlockArrays
    
def processKey(board, key):
	global nBlocks 

	state = board.accept(key)
	printScreen(board)
          
	if state != TetrisState.NewBlock:
		return state

	idxBlockType = randint(0, nBlocks-1)
	key = '0' + str(idxBlockType)
	state = board.accept(key)
	printScreen(board)

	if state != TetrisState.Finished:
		return state

	return state

if __name__ == "__main__": #직접 실행된 모듈이라면, 
#__name__=현재 모듈의 이름을 담고 있는 내장변수
#직접 실행된 모듈의 경우 __main__이라는 값, 직접 실행되지 않은 import된 모듈은 모듈의 이름(파일명) 가지게 됨
	setOfBlockArrays = initSetOfBlockArrays() #블록 세트 생성

	Tetris.init(setOfBlockArrays) #해당 블록 세트를 사용하는 Tetris 클래스 초기 설정 
	board = Tetris(20, 15) #설정된 Tetris 클래스의 객체 board 생성->해당 설정을 가진 Tetris 게임 시작

	idxBlockType = randint(0, nBlocks-1) #현재 블록의 종류는 0~6까지의 랜덤 정수로 결정
	key = '0' + str(idxBlockType) #현재 key값은 '0현재 블록 종류'로 설정 
	state = board.accept(key) #현재 게임 진행 상태는 Tetris 게임(board)이 key값을 받아 반환해준 값으로 설정
	printScreen(board) #Tetris게임(board) 화면에 출력

	while True: 
		key = readKeyWithTimeOut() #시간 흐름에 따른 키 값을 읽어와서 key 값 설정
		if not key: #key값이 0이면 key='s'(블록 한 칸 아래로 내려감)
			key = 's'
		#print(repr(key))
        
		if key == 'q': #key값이 'q'이면 현재 게임 진행 상태 Finished로 설정하고 반복문 탈출(게임 종료)
			state = TetrisState.Finished
			print('Game aborted...')
			break

		state = processKey(board, key) #
		if state == TetrisState.Finished: #현재 게임 진행 상태가 Finished면 반복문 탈출(게임 종료)
			print('Game Over!!!')
			break
    
	unregisterAlarm() #알람 초기화 후 종료
	print('Program terminated...')

### end of main.py


