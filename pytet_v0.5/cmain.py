from ctetris import *
from random import *
from color_print import *

import os
import sys
import tty
import termios
import signal
import imp

def clearScreen(numlines=100): #Tetris와 동일
	if os.name == 'posix':
		os.system('clear')
	elif os.name in ['nt', 'dos', 'ce']:
		os.system('CLS')
	else:
		print('\n' * numlines)
	return

def _printScreen(board): #Tetris ver. printScreen
	clearScreen()
	array = board.oScreen.get_array()

	for y in range(board.oScreen.get_dy()-Tetris.iScreenDw):
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

def printScreen(board): #CTetris ver. printScreen  #화면 출력 함수
	clearScreen() #화면 지우기
	array = board.oCScreen.get_array() #출력하려는 CTetris객체(board)의 oScreen배열을 array에 저장
	#Tetris ver. printScreen과 마찬가지로 반복문을 통해 원하는 범위의 화면 출력
	for y in range(board.oScreen.get_dy()-Tetris.iScreenDw):
		line = ''
		for x in range(Tetris.iScreenDw, board.oScreen.get_dx()-Tetris.iScreenDw):
			if array[y][x] == 0: #해당 위치 값이 0이면 흰색 빈칸 출력
				line += TextColor().white+'□' 
			elif array[y][x] == 1:#해당 위치 값이 1이면 빨간색 칸 출력
				line += TextColor().red+'■'
			elif array[y][x] == 2:#해당 위치 값이 2이면 초록색 칸 출력
				line += TextColor().green+'■'
			elif array[y][x] == 3:#해당 위치 값이 3이면 노란색 칸 출력
				line += TextColor().yellow+'■'
			elif array[y][x] == 4:#해당 위치 값이 4이면 파란색 칸 출력
				line += TextColor().blue+'■'
			elif array[y][x] == 5:#해당 위치 값이 5이면 보라색 칸 출력
				line += TextColor().purple +'■'
			elif array[y][x] == 6:#해당 위치 값이 6이면 cyan색 칸 출력
				line += TextColor().cyan+'■'
			elif array[y][x] == 7:#해당 위치 값이 7이면 핑크색 칸 출력
				line += TextColor().pink+'■'
			else: #모두 아니면 'XX' 출력
				line += 'XX'
		print(line)

	print()
	return

def unregisterAlarm(): #Tetris와 동일
	signal.alarm(0)
	return

def registerAlarm(handler, seconds): #Tetris와 동일
	unregisterAlarm()
	signal.signal(signal.SIGALRM, handler)
	signal.alarm(seconds)
	return

def timeout_handler(signum, frame): #Tetris와 동일
	#print("timeout!")
	raise RuntimeError ### we have to raise error to wake up any blocking function
	return

def getChar(): #Tetris와 동일
	fd = sys.stdin.fileno()
	old_settings = termios.tcgetattr(fd)
	try:
		tty.setraw(sys.stdin.fileno())
		ch = sys.stdin.read(1)
		unregisterAlarm()
	finally:
		termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
	return ch
 
def readKey(): #Tetris와 동일
	c1 = getChar()
	if ord(c1) != 0x1b: ### ESC character
		return c1
	c2 = getChar()
	if ord(c2) != 0x5b: ### '[' character
		return c1
	c3 = getChar()
	return chr(0x10 + ord(c3) - 65)

def readKeyWithTimeOut(): #Tetris와 동일
	registerAlarm(timeout_handler, 1)
	try:
		key = readKey()
		unregisterAlarm()
		return key
	except RuntimeError as e:
		pass # print('readkey() interrupted!')

	return
 
def rotate(m_array): #Tetris와 동일
    size = len(m_array)
    r_array = [[0] * size for _ in range(size)]

    for y in range(size):
        for x in range(size):
            r_array[x][size-1-y] = m_array[y][x]

    return r_array

def initSetOfBlockArrays(): #Tetris와 동일
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

    nBlocks = len(arrayBlks)
    setOfBlockArrays = [[0] * 4 for _ in range(nBlocks)]

    for idxBlockType in range(nBlocks):
        temp_array = arrayBlks[idxBlockType]
        setOfBlockArrays[idxBlockType][0] = temp_array
        for idxBlockDegree in range(1,4):
            temp_array = rotate(temp_array)
            setOfBlockArrays[idxBlockType][idxBlockDegree] = temp_array

    return setOfBlockArrays
#주어진 key값을 작동시켜 CTetris 게임 현재 진행 상태 반환하는 함수
def processKey(board, key): 
	global nBlocks #블록 종류 갯수

	state = board.accept(key) 
	#key값 전해주고 이에 따른 CTetris 게임 진행상태 반환받기
	printScreen(board)#CTetris 게임 화면 출력
          
	if state != TetrisState.NewBlock:
	#새로운 블록이 생성되어야 할 상태 아니면 현재 상태 반환
		return state

	key = getKey(False) ### log or replay
	#새 블록 생성되어야 할 상태면, getKey함수 이용해 새 블록에 해당되는 key값 받기
	state = board.accept(key) #key값 전해주고 이에 따른 CTetris 게임 진행상태 받기
	printScreen(board) #CTetris 게임 화면 출력

	if state != TetrisState.Finished:
		return state
	return state#현재 CTetris 게임 진행상태 반환

def getKey(is_keystroke_needed): #현재 모드와 상황에 따른 key값 반환하는 함수
	global is_log_mode #전역변수 is_log_mode 사용(현재 모드가 log모드인지)
	global is_replay_mode #전역변수 is_replay_mode 사용(현재 모드가 replay모드인지)

	if is_replay_mode: #만약 replay모드가 켜져있으면 get_key_from_log()실행해 key값 받기
		key = get_key_from_log()
	#replay모드가 아닌 경우	
	elif is_keystroke_needed: #만약 사용자로부터 키값을 받는 것을 필요로 한다면
		key = readKeyWithTimeOut() #사용자로부터 키 값 받기
		if not key: #받은 key값이 없으면 아래로 한 칸('s')
			key = 's'
	else: #replay모드도 아닌데 사용자로부터 키 값을 받을 필요없다면(새 블록 생성)
		idxBlockType = randint(0, nBlocks-1) #현재 블록의 종류는 0~6까지의 랜덤 정수로 결정
		key = '0' + str(idxBlockType) #key값을 '0현재 블록 종류'로 설정
 
	if is_log_mode: #만약 log모드가 켜져있으면 log_key(key)함수 실행(keylog.py파일에 기록)
		log_key(key)
	return key #설정된 key값 반환

###################replay용 함수##########################
def get_key_from_log(): 
#replay모드에서 사용하는 함수(keylog.py에 기록되어있는 keys 리스트로부터 key값 하나씩 가져오기) 
	global keys #전역변수 keys(key값들이 기록된 리스트) 사용
	global key_idx #전역변수 key_idx 사용

	key = keys[key_idx] #key=keys리스트에 있는 원소로 설정
	key_idx += 1 
	return key
####################log용 함수#############################
def log_start(): #log시작 함수/key값 기록할 파일을 쓰기모드로 열어서 초기화
	fp = open('keylog.py', 'w')
	fp.write('keys = [\n')
	fp.close()
	return

def log_end(): #log종료 함수/key값 기록한 파일을 쓰기모드로 열어서 ]\n 덧붙여서 저장
	fp = open('keylog.py', 'a')
	fp.write(']\n')
	fp.close()
	return

def log_key(key): #log함수/key값 기록하는 파일을 쓰기모드로 열어서 key리스트에 있는 내용 맨 뒤에서부터
	#차례대로 파일에 덧붙여서 쓰고 저장하기
	fp = open('keylog.py', 'a') 
	fp.write('\'%c\',\n' % key[-1])
	fp.close()
	return

if __name__ == "__main__":
	global is_log_mode #log모드 여부 확인 전역변수
	global is_replay_mode #replay모드 여부 확인 전역변수
	global keys #key값들 기록을 담은 리스트 전역변수
	global key_idx #keys리스트에 있는 마지막 key의 인덱스 값을 가진 전역변수

	is_log_mode = False #log모드 꺼진 상태로 초기화
	if len(sys.argv) == 2 and sys.argv[1] == 'log': 
	#python3 cmain.py log 실행 시, keylog.py에 기록 남음
		is_log_mode = True #log모드 켜기
		log_start() #log시작함수 실행

	is_replay_mode = False #replay모드 꺼진 상태로 초기화
	if len(sys.argv) == 2 and sys.argv[1] == 'replay': 
		#python3 cmain.py replay 실행 시, keylog.py에 적힌 기록 보여줌
		is_replay_mode = True #replay모드 켜기
		key_idx = 0 #keys리스트에 있는 마지막 key인덱스값 초기화(0)
		from keylog import * #keylog.py 불러오기
		print(keys) #keys리스트 내용 출력
		input('Press any key to continue:')

	setOfBlockArrays = initSetOfBlockArrays() #블록 세트 생성

	CTetris.init(setOfBlockArrays) #해당 블록 세트 사용하는 CTetris 클래스 초기 설정
	board = CTetris(20, 15) #설정된 CTetris 클래스의 객체 board 생성->해당 설정을 가진 CTetris 게임 시작
	key = getKey(False) ### log or replay
	#사용자에게 key값 받을 필요 없음(시작할 때는 새로운 블록 먼저 등장해야함)
	state = board.accept(key) #현재 게임 진행 상태는 CTetris 게임(board)이 key값을 받아 반환해준 값으로 설정
	printScreen(board) #CTetris게임(board) 화면에 출력

	while True:
		key = getKey(True) ### log or replay
		#사용자에게 key값 받도록 하여 key값 설정
		if key == 'q': #만약 key값이 'q'이면 현재 게임 진행 상태 Finished로 설정하고 반복문 탈출
			state = TetrisState.Finished
			print('Game aborted...')
			break

		state = processKey(board, key) #processKey함수 통해 현재 게임 진행 상태 받아오기
		if state == TetrisState.Finished: #현재 게임 진행 상태가 Finished면 반복문 탈출
			print('Game Over!!!')
			break
    
	unregisterAlarm() #알람 초기화 후 
	print('Program terminated...')
	if is_log_mode: #만약 log모드였으면 log종료
		log_end()

### end of main.py
