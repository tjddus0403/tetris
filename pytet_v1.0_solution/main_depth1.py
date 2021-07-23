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
    
def processKey(window, board, key):
	global nBlocks 

	state = board.accept(key)
	printWindow(window, board.getScreen())
          
	if state != TetrisState.NewBlock:
		return state

	idxBlockType = randint(0, nBlocks-1)
	key = str(idxBlockType)
	state = board.accept(key)
	printWindow(window, board.getScreen())

	if state != TetrisState.Finished:
		return state

	return state

##############################################################
### UI code
##############################################################
def printMsg(msg): #전달받은 문자열을 win0에 출력해주는 함수
	window = win0 #win0을 window로 지정

	window.clear() #win0 지우기
	window.addstr(0, 0, msg) #win0의 (0,0) 좌표에 msg출력하기

	lock.acquire() #락걸기
	window.refresh() #win0 갱신 (갱신을 해야 지금까지 win0에 적용한 것들이 실제로 보임)
	lock.release() #락해제
	return 

def arrayToString(array): #배열을 문자열로 바꾸는 함수
	line = ''
	for x in array:
		if x == 0:
			line += '□'
		elif x == 1:
			line += '■'
		else:
			line += 'XX'

	return line

def printWindow(window, screen): #전달받은 창에 전달받은 화면(행렬)을 출력해주는 함수
	array = screen.get_array() #array에 Matrix객체인 screen의 2차원 배열형태 저장
	window.clear() #전달받은 창 지우기

	for y in range(screen.get_dy()-Tetris.iScreenDw):
		line = arrayToString(array[y][Tetris.iScreenDw:-Tetris.iScreenDw])
		#매 라인마다 arrayToString통해 배열을 문자열로 바꿔서 line에 저장
		window.addstr(y, 0, line)
		#전달받은 창의 (0, y)좌표에 line 출력

	lock.acquire() #락 걸기
	window.refresh() #전달받은 창 갱신
	lock.release() #락 해제
	return

def getChar(): ### non-blocking mode
	if select.select([sys.stdin], [], [], 0) == ([sys.stdin], [], []):
		return sys.stdin.read(1)
	raise error('No character')

##############################################################
### Threading code (Observer pattern)
##############################################################
from abc import *

isGameDone = False

class Publisher(metaclass = ABCMeta): #Publisher 클래스
	@abstractmethod
	def addObserver(self, observer): #Observer를 추가하는 함수
		pass

	@abstractmethod
	def notifyObservers(self, key): 
	#Observer에게 Publisher의 정보(key값)를 알리는 함수
		pass

class KeyController(threading.Thread, Publisher): #KeyController 클래스
	#Publisher 클래스를 상속하며 Publisher 역할을 함
	def __init__(self, *args, **kwargs): #KeyController 객체 생성자
		super(KeyController, self).__init__(*args[1:], **kwargs)
		self.name = args[0] #받은 인자 중 첫번째 것으로 객체의 이름 설정
		self.observers = list() #객체의 observer를 관리하기 위해 observers리스트 생성
		return

	def addObserver(self, observer): #객체의 observer를 추가하는 함수
		self.observers.append(observer) #observers리스트에 전달받은 observer 추가
		#c++에서 구현하려면 포인터로 주어야 할 듯 (복사해서 갖고있는건 의미가 없음)
		return
	
	def notifyObservers(self, key): #객체의 observer들에게 key값을 전해주는 함수
		for observer in self.observers: #observers리스트에 있는 각 observer객체들은
			observer.update(key) #각자 자신의 keys리스트에 전달받은 key값을 update해줌
		return

	def run(self): #스레드 함수
		global isGameDone

		while not isGameDone: #isGameDone이 false이면 
			try: #에러가 발생할 수 있는 코드 (key값이 입력되지 않는 경우)
				key = getChar() ### non-blocking mode
				#getChar을 통해 사용자로부터 key값 받아오기
				self.notifyObservers(key)
				#받아온 key값을 notifyObservers통해 객체의 observer들에게 전달
			except: #keyr값이 입력되지 않았으면 (런타임 에러)
				pass #그냥 넘어가기

		printMsg('%s terminated... Press any key to continue' % self.name)
		#printMsg통해 종료 메시지 출력
		time.sleep(1) #1초 쉬기
		self.notifyObservers('') #객체의 observer들에게 빈 key값을 전달
		return

class TimeController(threading.Thread, Publisher): #TimeController 클래스
	def __init__(self, *args, **kwargs): #TimeController 객체 생성자
		super(TimeController, self).__init__(*args[1:], **kwargs)
		self.name = args[0] #받은 인자 중 첫번째 것으로 객체의 이름 설정
		self.observers = list() #객체의 observer를 관리하기 위해 observers리스트 생성
		return

	def addObserver(self, observer): #객체의 observer를 추가하는 함수
		self.observers.append(observer) #observers리스트에 전달받은 observer 추가
		return
	
	def notifyObservers(self, key): #객체의 observer들에게 key값을 전해주는 함수
		for observer in self.observers: #observers리스트에 있는 각 observer객체들은
			observer.update(key) #각자 자신의 keys리스트에 전달받은 key값을 update해줌
		return

	def run(self): #스레드 함수
		while not isGameDone:
			time.sleep(1)
			self.notifyObservers('y') #객체의 observer들에게 key값 'y'를 전달

		printMsg('%s terminated... Press any key to continue' % self.name)
		#printMsg통해 종료 메시지 출력
		time.sleep(1) #1초 쉬기
		self.notifyObservers('') #객체의 observer들에게 빈 key값을 전달
		return

class Observer(metaclass = ABCMeta): #Observer 클래스
	@abstractmethod
	def update(self, key): #Publisher의 정보(key값)를 Observer가 전달받는 함수
		pass

class ModelView(threading.Thread, Observer): #ModelView 클래스
	def __init__(self, *args, **kwargs):
		super(ModelView, self).__init__(*args[1:], **kwargs)
		self.name = args[0]
		self.queue = list() #key값을 저장할 keys 리스트 생성
		self.cv = threading.Condition() 
		#key값을 읽어오는 과정에서 실행 순서를 지정해줄 조건변수 cv 생성
		#update가 되고 난 다음에 read를 해야함
		#아무것도 없는데 읽고 있을 수는 없으니까
		return

	def update(self, key): #Publisher에서 받은 key값을 keys리스트에 저장하는 함수
		self.cv.acquire() #락 걸기
		self.queue.append(key) #Publisher로부터 전달받은 key값 keys리스트에 추가
		self.cv.notify() #조건변수 이용해 
		self.cv.release() #락 해제
		return
	
	def read(self):
		self.cv.acquire() #락 걸기
		while len(self.queue) < 1:
			self.cv.wait()
		key = self.queue.pop(0)
		self.cv.release() #락 해제
		return key
	
	def addKeypad(self, keypad): #키패드 설정하는 함수
		self.keypad = keypad #전달받은 키패드를 객체의 키패드로 설정
		return

	def addWindow(self, window): #창 설정하는 함수
		self.window = window #전달받은 창을 객체의 창으로 설정
		return

	def run(self): #스레드 함수
		global isGameDone

		setOfBlockArrays = initSetOfBlockArrays()

		Tetris.init(setOfBlockArrays) 
		board = Tetris(20, 15)

		idxBlockType = randint(0, nBlocks-1)
		key = str(idxBlockType)
		state = board.accept(key)
		printWindow(self.window, board.getScreen())

		while not isGameDone:
			key = self.read() #read함수를 통해 key값을 읽어오고
			if key == '':
				break
			if key not in self.keypad: #읽어온 key값이 객체의 키패드에 없으면
				continue #그냥 넘기기

			key = self.keypad[key] #있으면 키패드에 해당하는 값으로 key값 재설정
			if key == 'q':
				state = TetrisState.Finished
			else:
				state = processKey(self.window, board, key)

			if state == TetrisState.Finished:
				isGameDone = True
				printMsg('%s IS DEAD!!!' % self.name)
				time.sleep(2)
				break

		printMsg('%s terminated... Press any key to continue' % self.name)
		time.sleep(1)
		return

##############################################################
### Main code
##############################################################

def main(args):
	global lock
	global win0

	lock = threading.Lock()

	screen = curses.initscr()
	screen.clear()

	curses.echo()
	curses.start_color()
	curses.use_default_colors()

	win1 = curses.newwin(20, 30, 0, 0)
	win2 = curses.newwin(20, 30, 0, 40)
	win0 = curses.newwin(3, 70, 21, 0)

	keypad1 = { 'q': 'q', 'w': 'w', 'a': 'a', 's': 'y', 'd': 'd', ' ': ' ', 'y': 'y' }
	th_model1 = ModelView('model1')
	th_model1.addKeypad(keypad1)
	th_model1.addWindow(win1)

	keypad2 = { 'u': 'q', 'i': 'w', 'j': 'a', 'k': 'y', 'l': 'd', '\r': ' ', 'y': 'y' }
	th_model2 = ModelView('model2')
	th_model2.addKeypad(keypad2)
	th_model2.addWindow(win2)

	th_cont1 = KeyController('kcont')
	th_cont1.addObserver(th_model1)
	th_cont1.addObserver(th_model2)

	th_cont2 = TimeController('tcont')
	th_cont2.addObserver(th_model1)
	th_cont2.addObserver(th_model2)

	threads = list()
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
