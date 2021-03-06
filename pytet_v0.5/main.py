from tetris import * 
from random import *

import os #운영체제 제어 모듈
import sys #파이썬 인터프리터 제어 모듈
import tty #터미널 제어 모듈
import termios #저수준 터미널 제어 인터페이스
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
	for y in range(board.oScreen.get_dy()-Tetris.iScreenDw): 
	#출력하려는 화면 y축 범위 : 객체(board)의 oScreen의 dy에서 블록의 최대 폭인 iScreenDw를 뺀 범위 
		line = ''
		for x in range(Tetris.iScreenDw, board.oScreen.get_dx()-Tetris.iScreenDw):
		#출력하려는 화면 x축 범위 : 객체(board)의 oScreen의 dx에서 블록의 최대 폭인 iScreenDw를 뺀 범위
			if array[y][x] == 0: #만약 해당 위치 값이 0이면 빈 칸 출력
				line += '□'
			elif array[y][x] == 1: #만약 해당 위치 값이 1이면 칠해진 칸 출력
				line += '■'
			else: #모두 아니면 'XX' 출력
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
#Blocking함수를 깨워주기 위한 목적
def registerAlarm(handler, seconds):#알람 예약 함수
	unregisterAlarm() #예약된 알람이 없는 상태 설정
	signal.signal(signal.SIGALRM, handler) #SIGALRM 처리기를 함수handler로 설정
	signal.alarm(seconds) #seconds초 alarm 예약->여기선 1초->반환값 1
	#1초 지나면 handler에 있는 런타임오류 강제발생 시켜줌
	return

def timeout_handler(signum, frame): 
	print("timeout!")
	raise RuntimeError ### we have to raise error to wake up any blocking function
	#오류 강제 발생
	return

def getChar(): #문자 받아오는 함수
	fd = sys.stdin.fileno() #파일 디스크립터 0이 표준 입력(stdin)을 나타냄. 따라서 fd=0 
	old_settings = termios.tcgetattr(fd) 
	#termios.tcgetattr() : fd에 대한 tty attribute(속성)을 포함하는 리스트 반환
	try: #예외 발생 가능성이 있는 코드
		tty.setraw(sys.stdin.fileno()) #fd모드를 raw로 변경 
		#when생략되었기에 기본값 termios.TCSAFLUSH로 termios.tcsetattr()로 전달됨
		#따라서 모드가 raw로 변경된 fd에 대한 tty attribute를 attribute로 설정
		ch = sys.stdin.read(1) #사용자로부터 문자열 입력받음(자동 엔터->sys.stdin.read()특성)
		unregisterAlarm() #알람 초기화
	finally: #예외 발생과 관련없이 무조건 실행할 코드
		termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
		#다시 사용자로부터 입력을 받을 원래 상태로 복귀
	return ch #사용자로부터 입력받은 문자 반환
 
def readKey(): #getChar() 이용해 key읽고 반환하는 함수
	c1 = getChar() #첫번째 문자를 사용자로부터 받음
	if ord(c1) != 0x1b: ### ESC character #첫번째 문자가 'ESC'가 아니면 해당 문자 반환
		return c1 
	c2 = getChar() #두번째 문자를 사용자로부터 받음
	if ord(c2) != 0x5b: ### '[' character #두번째 문자가 '['가 아니면 첫번째 문자 반환
		return c1
	c3 = getChar() #세번째 문자를 사용자로부터 받음
	return chr(0x10 + ord(c3) - 65) #아스키코드로 주어진 수식을 계산해 나오는 문자를 반환

def readKeyWithTimeOut(): #시간 흐름에 따른 key값 바꿔주는 함수
	registerAlarm(timeout_handler, 1) #1초마다 런타임 에러 발생
	try: #에러발생 가능한 코드
		key = readKey() #key 읽어오기
		unregisterAlarm() #알람 초기화
		return key #키 값 반환
	except RuntimeError as e: #런타임 에러에 대한 예외 설정
		pass # print('readkey() interrupted!')

	return
 
def rotate(m_array): #블록 회전 함수
    size = len(m_array)
    r_array = [[0] * size for _ in range(size)] #회전된 블록을 담을 r_array생성

    for y in range(size): #반복문 통해 r_array에 회전된 블록 담기
        for x in range(size):
            r_array[x][size-1-y] = m_array[y][x]

    return r_array #회전된 블록 반환

def initSetOfBlockArrays(): #블록세트 생성 후 반환하는 함수(모든 상태의 블록을 담고 있음(블록 한 종류당 회전에 따라 4가지 상태 존재))
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
    setOfBlockArrays = [[0] * 4 for _ in range(nBlocks)] #한 종류당 4가지 상태가 들어갈 수 있는 크기의 setOfBlockArrays 배열 생성
		
    for idxBlockType in range(nBlocks): #반복문을 통해 setOfBlockArrays 배열에 모든 종류, 상태의 블록 넣기
        temp_array = arrayBlks[idxBlockType]
        setOfBlockArrays[idxBlockType][0] = temp_array
        for idxBlockDegree in range(1,4):
            temp_array = rotate(temp_array)
            setOfBlockArrays[idxBlockType][idxBlockDegree] = temp_array
    return setOfBlockArrays #생성된 setOfBlockArrays 배열 반환
    
def processKey(board, key): #주어진 key값을 작동시켜 Tetris 게임 현재 진행 상태 반환하는 함수
	global nBlocks #블록 종류 갯수

	state = board.accept(key) #key값 전해주고 이에 따른 Tetris 게임 진행상태 반환받기
	printScreen(board) #Tetris 게임 화면 출력
          
	if state != TetrisState.NewBlock: #새로운 블록이 생성되어야 할 상태가 아니라면 현재 상태 반환
		return state

	idxBlockType = randint(0, nBlocks-1) #새로운 블록이 생성되어야 할 상태라면 
	key = '0' + str(idxBlockType) #key값을 '0블록종류'로 설정
	state = board.accept(key) #key값 전해주고 이에 따른 Tetris 게임 진행상태 반환받기
	printScreen(board) #Tetris 게임 화면 출력

	if state != TetrisState.Finished: 
		return state 
	return state #현재 Tetris 게임 진행상태 반환

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

		state = processKey(board, key) #processKey함수 통해 현재 게임 진행 상태 받아오기
		if state == TetrisState.Finished: #현재 게임 진행 상태가 Finished면 반복문 탈출(게임 종료)
			print('Game Over!!!')
			break
    
	unregisterAlarm() #알람 초기화 후 종료
	print('Program terminated...')

### end of main.py


