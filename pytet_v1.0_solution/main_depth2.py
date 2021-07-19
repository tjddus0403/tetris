from tetris import *
from random import *

import os
import sys
import tty
import termios
import select

import threading
import time
import curses
import copy

##############################################################
### Data model related code
##############################################################
def rotate(m_array):
    size = len(m_array)
    r_array = [[0] * size for _ in range(size)]

    for y in range(size):
        for x in range(size):
            r_array[x][size-1-y] = m_array[y][x]

    return r_array

def initSetOfBlockArrays():
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
    

##############################################################
### UI code
##############################################################

def printMsg(msg): 
	window = win0

	window.clear()
	window.addstr(0, 0, msg) #addstr(지정된 문자열을 현재 위치에 출력)

	lock.acquire()
	window.refresh() #화면 갱신
	lock.release()
	return

def arrayToString(array):
	line = ''
	for x in array:
		if x == 0:
			line += '□'
		elif x == 1:
			line += '■'
		else:
			line += 'XX'

	return line

def printWindow(window, screen):
	array = screen.get_array()
	window.clear()

	for y in range(screen.get_dy()-Tetris.iScreenDw):
		line = arrayToString(array[y][Tetris.iScreenDw:-Tetris.iScreenDw])
		window.addstr(y, 0, line)

	lock.acquire()
	window.refresh()
	lock.release()
	return

def getChar(): ### non-blocking mode
	if select.select([sys.stdin], [], [], 0) == ([sys.stdin], [], []):
		return sys.stdin.read(1)
	raise error('No character')

##############################################################
### Threading code (Observer pattern)
##############################################################

from abc import * #abc=abstract base class의 약자

isGameDone = False

class Publisher(metaclass = ABCMeta): 
	@abstractmethod
	def addObserver(self, observer):
		pass

	@abstractmethod
	def notifyObservers(self, obj):
		pass

class Observer(metaclass = ABCMeta):
	@abstractmethod
	def update(self, obj):
		pass

class KeyController(threading.Thread, Publisher):
	def __init__(self, *args, **kwargs):
		super(KeyController, self).__init__(*args[1:], **kwargs)
		self.name = args[0]
		self.observers = list()
		return

	def addObserver(self, observer):
		self.observers.append(observer)
		return
	
	def notifyObservers(self, obj):
		for observer in self.observers:
			observer.update(obj)
		return

	def run(self):
		global isGameDone

		while not isGameDone:
			try:
				key = getChar() ### non-blocking mode
				self.notifyObservers(key)
			except:
				pass

		printMsg('%s terminated... Press any key to continue' % self.name)
		time.sleep(5)
		self.notifyObservers('')
		return

class TimeController(threading.Thread, Publisher):
	def __init__(self, *args, **kwargs):
		super(TimeController, self).__init__(*args[1:], **kwargs)
		self.name = args[0]
		self.observers = list()
		return

	def addObserver(self, observer):
		self.observers.append(observer)
		return
	
	def notifyObservers(self, obj):
		for observer in self.observers:
			observer.update(obj)
		return

	def run(self):
		global isGameDone

		while not isGameDone:
			time.sleep(1)
			self.notifyObservers('y')

		printMsg('%s terminated... Press any key to continue' % self.name)
		time.sleep(5)
		self.notifyObservers('')
		return

class Model(threading.Thread, Observer, Publisher):
	def __init__(self, *args, **kwargs):
		super(Model, self).__init__(*args[1:], **kwargs)
		self.name = args[0]
		self.queue = list()
		self.cv = threading.Condition()
		self.observers = list()
		return

	def addObserver(self, observer):
		self.observers.append(observer)
		return
	
	def notifyObservers(self, obj):
		for observer in self.observers:
			observer.update(obj)
		return

	def update(self, obj):
		self.cv.acquire()
		self.queue.append(obj)
		self.cv.notify()
		self.cv.release()
		return
	
	def read(self):
		self.cv.acquire()
		while len(self.queue) < 1:
			self.cv.wait()
		obj = self.queue.pop(0)
		self.cv.release()
		return obj
	
	def addKeypad(self, keypad):
		self.keypad = keypad
		return

	def processKey(self, board, key):
		global nBlocks 

		state = board.accept(key)
		self.notifyObservers(copy.deepcopy(board.getScreen()))
          
		if state != TetrisState.NewBlock:
			return state

		idxBlockType = randint(0, nBlocks-1)
		key = str(idxBlockType)
		state = board.accept(key)
		self.notifyObservers(copy.deepcopy(board.getScreen()))

		if state != TetrisState.Finished:
			return state

		return state

	def run(self):
		global isGameDone

		setOfBlockArrays = initSetOfBlockArrays()

		Tetris.init(setOfBlockArrays)
		board = Tetris(20, 15)

		idxBlockType = randint(0, nBlocks-1)
		key = str(idxBlockType)
		state = board.accept(key)
		self.notifyObservers(copy.deepcopy(board.getScreen()))

		while not isGameDone:
			key = self.read()
			if key == '':
				break
			if key not in self.keypad:
				continue

			key = self.keypad[key]
			if key == 'q':
				state = TetrisState.Finished
			else:
				state = self.processKey(board, key)

			if state == TetrisState.Finished:
				isGameDone = True
				printMsg('%s IS DEAD!!!' % self.name)
				time.sleep(2)
				break

		printMsg('%s terminated... Press any key to continue' % self.name)
		time.sleep(5)
		self.notifyObservers('')
		return

class View(threading.Thread, Observer):
	def __init__(self, *args, **kwargs):
		super(View, self).__init__(*args[1:], **kwargs)
		self.name = args[0]
		self.queue = list()
		self.cv = threading.Condition()
		return

	def update(self, obj):
		self.cv.acquire()
		self.queue.append(obj)
		self.cv.notify()
		self.cv.release()
		return
	
	def read(self):
		self.cv.acquire()
		while len(self.queue) < 1:
			self.cv.wait()
		obj = self.queue.pop(0)
		self.cv.release()
		return obj
	
	def addWindow(self, window):
		self.window = window
		return

	def run(self):
		global isGameDone

		while not isGameDone:
			obj = self.read()
			if obj == '':
				break
			printWindow(self.window, obj)

		printMsg('%s terminated... Press any key to continue' % self.name)
		time.sleep(5)
		return

##############################################################
### Main code
##############################################################

def main(args):
	global lock
	global win0 

	lock = threading.Lock() 

	screen = curses.initscr() #curses 초기화
	screen.clear() #현재 화면 지우기

	curses.echo() #입력된 문자를 화면상에 출력
	curses.start_color() 
	#8개의 기본 색상(검정, 빨강, 녹색, 노랑, 파랑, 마젠타, 시안 및 흰색)과 
	#터미널이 지원할 수 있는 색상과 색상 쌍의 최댓값인 curses 모듈의 
	#2개의 전역 변수 COLORS와 COLOR_PAIRS를 초기화합니다. 
	#또한 터미널의 전원을 켰을 때의 값으로 터미널의 색상을 복원합니다.
	curses.use_default_colors() #터미널에서 기본색 사용하도록 허용

	win1 = curses.newwin(20, 30, 0, 0) #높이 20, 너비 30인 screen을 (0,0) 좌표에 생성
	win2 = curses.newwin(20, 30, 0, 40) #높이 20, 너비 30인 screen을 (0,40) 좌표에 생성
	win0 = curses.newwin(3, 70, 21, 0) #높이 3, 너비 70인 screen을 (21,0) 좌표에 생성

	th_view1 = View('view1')
	th_view1.addWindow(win1)
	
	th_view2 = View('view2')
	th_view2.addWindow(win2)

	keypad1 = { 'q': 'q', 'w': 'w', 'a': 'a', 's': 'y', 'd': 'd', ' ': ' ', 'y': 'y' }
	th_model1 = Model('model1')
	th_model1.addKeypad(keypad1)
	th_model1.addObserver(th_view1)

	keypad2 = { 'u': 'q', 'i': 'w', 'j': 'a', 'k': 'y', 'l': 'd', '\r': ' ', 'y': 'y' }
	th_model2 = Model('model2')
	th_model2.addKeypad(keypad2)
	th_model2.addObserver(th_view2)

	th_cont1 = KeyController('kcont')
	th_cont1.addObserver(th_model1)
	th_cont1.addObserver(th_model2)

	th_cont2 = TimeController('tcont')
	th_cont2.addObserver(th_model1)
	th_cont2.addObserver(th_model2)

	threads = list()
	threads.append(th_view1)
	threads.append(th_view2)
	threads.append(th_model1)
	threads.append(th_model2)
	threads.append(th_cont1)
	threads.append(th_cont2)

	exited = list()

	fd = sys.stdin.fileno()
	old_settings = termios.tcgetattr(fd)
	tty.setcbreak(sys.stdin.fileno())

	for th in threads:
		th.start()
	
	for th in threads:
		th.join()
		exited.append(th.name)
	
	string = ''
	for name in exited:
		string += ':%s' % name
	string += ' terminated!!!'

	printMsg(string)
	time.sleep(2)
	printMsg('Program terminated...')
	time.sleep(1)

curses.wrapper(main)

### end of main.py

