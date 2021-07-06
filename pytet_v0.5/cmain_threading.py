from ctetris import *
from random import *
from color_print import *

import os
import sys
import tty
import termios
import signal

import threading
import time

##############################################################
### Data model related code
##############################################################
def rotate(m_array): #cmain.py와 동일
    size = len(m_array)
    r_array = [[0] * size for _ in range(size)]

    for y in range(size):
        for x in range(size):
            r_array[x][size-1-y] = m_array[y][x]

    return r_array

def initSetOfBlockArrays(): #cmain.py와 동일
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
    
def processKey(board, key): #cmain.py와 동일
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

##############################################################
### UI code
##############################################################

def clearScreen(numlines=100): #cmain.py와 동일
	if os.name == 'posix':
		os.system('clear')
	elif os.name in ['nt', 'dos', 'ce']:
		os.system('CLS')
	else:
		print('\n' * numlines)
	return

def _printScreen(board): #main.py의 printScreen
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

def printScreen(board): #cmain.py와 동일
	#clearScreen()
	array = board.oCScreen.get_array()

	for y in range(board.oScreen.get_dy()-Tetris.iScreenDw):
		line = ''
		for x in range(Tetris.iScreenDw, board.oScreen.get_dx()-Tetris.iScreenDw):
			if array[y][x] == 0:
				line += TextColor().white+'□'
			elif array[y][x] == 1:
				line += TextColor().red+'■'
			elif array[y][x] == 2:
				line += TextColor().green+'■'
			elif array[y][x] == 3:
				line += TextColor().yellow+'■'
			elif array[y][x] == 4:
				line += TextColor().blue+'■'
			elif array[y][x] == 5:
				line += TextColor().purple +'■'
			elif array[y][x] == 6:
				line += TextColor().cyan+'■'
			elif array[y][x] == 7:
				line += TextColor().pink+'■'
			else:
				line += 'XX'
		print(line)

	print()
	return

queue = list() #공유변수 (Critical Section)
cv = threading.Condition() # Condition variable
isGameDone = False #공유변수

def getChar(): #사용자로부터 문자 하나 입력 받아 반환하는 함수
	ch = sys.stdin.read(1)
	return ch

class KeyProducer(threading.Thread): #Thread 클래스 상속받은 생산자 thread
	def run(self):
		global isGameDone
		print("KeyProducer Start!!!!!!!!!!!!!!")
		while not isGameDone:
			try:
				key=getChar()
			except:
				isGameDone = True
				print('getChar() wakes up!!')
				break
			############
			cv.acquire()
			print("KeyProducer Lock!!!!!")
			queue.append(key)
			cv.notify()
			cv.release()
			############
			print("KeyProducer release!!!!!")
			if key == 'q':
				isGameDone = True
				break
		return

class TimeOutProducer(threading.Thread): #Thread 클래스 상속받은 생산자 thread
	def run(self):
		print("TimeOut Start!!!!!!!!!!!")
		while not isGameDone:
			print("TimeOutProducer!!!!!!!!!!!!!!!!!!!!!")
			time.sleep(1)
			############
			cv.acquire()
			print("TimeOutProducer Lock!!!!!")
			queue.append('s')
			cv.notify()
			cv.release()
			############
			print("TimeOutProducer release!!!!!")
		return

class Consumer(threading.Thread): #Thread 클래스 상속받은 소비자 thread
	def run(self):
		global isGameDone
		print("Consumer Start!!!!!!")

		setOfBlockArrays = initSetOfBlockArrays()

		CTetris.init(setOfBlockArrays)
		board = CTetris(20, 15)

		idxBlockType = randint(0, nBlocks-1)
		key = '0' + str(idxBlockType)
		state = board.accept(key)
		printScreen(board)

		while not isGameDone:
			print("Consumer!!!!!!!!!!!!!!")
			############
			cv.acquire()
			print("Consumer Lock!!!!!")
			while len(queue) < 1:
				cv.wait()
			key = queue.pop(0)
			cv.release()
			############
			print("Consumer release!!!!!")

			if key == 'q':
				state = TetrisState.Finished
				print('Game aborted...')
				break

			state = processKey(board, key)
			if state == TetrisState.Finished:
				isGameDone = True
				print('Game Over!!!')
				os.kill(os.getpid(), signal.SIGINT)
				break
		return

def signal_handler(num, stack):
	print('signal_handler called!!')
	raise RuntimeError

if __name__ == "__main__":
	global fd
	global old_settings

	signal.signal(signal.SIGINT, signal_handler)

	threads = list()
	threads.append(Consumer())
	threads.append(KeyProducer())
	threads.append(TimeOutProducer())

	fd = sys.stdin.fileno()
	old_settings = termios.tcgetattr(fd)
	tty.setcbreak(sys.stdin.fileno())

	for th in threads:
		th.start()
		print("Start")
	
	for th in threads:
		try:
			th.join()
			print("join()!!!!!!!!")
		except:
			print('th.join() wakes up!!')
	
	termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
	print('Program terminated...')

### end of main.py
