from flask import Flask

from pistop import *
import time

app = Flask(__name__)

@app.route('/')
def index():
    return 'Hello world'

@app.route('/redlight')
def redlight():
    with PiStop(hwSetup='B+') as myPiStop:
        myPiStop.output(R,ON) #Switch Red lights ON
        time.sleep(2)
        myPiStop.output(R,OFF)
        return 'Luces Roja ON'
                
@app.route('/greenlight')
def greenlight():
    with PiStop(hwSetup="B+") as myPiStop:
        myPiStop.output(G,ON) #Switch Green lights ON
        time.sleep(2)
        myPiStop.output(G,OFF)
        return "Luces Verde ON"

@app.route('/orangelight')
def orangelight():
    with PiStop(hwSetup="B+") as myPiStop:
        myPiStop.output(Y,ON) #Switch Yellow lights ON
        time.sleep(2)
        myPiStop.output(Y,OFF)
        return "Luces Ambar ON"

@app.route('/redlight2')
def redlight2():
    with PiStop(hwSetup='C') as myPiStop:
        myPiStop.output(R,OFF)
	myPiStop.output(G,ON) #Switch Red lights ON
        time.sleep(2)
        myPiStop.output(G,OFF)
        return 'Luces Roja ON'

@app.route('/greenlight2')
def greenlight2():
    with PiStop(hwSetup="C") as myPiStop:
        myPiStop.output(R,ON) #Switch Green lights ON
        time.sleep(2)
        myPiStop.output(R,OFF)
        return "Luces Verde ON"

@app.route('/orangelight2')
def orangelight2():
    with PiStop(hwSetup="C") as myPiStop:
        myPiStop.output(G,OFF)
        myPiStop.output(Y,ON) #Switch Yellow lights ON
        time.sleep(2)
        myPiStop.output(Y,OFF)
        return "Luces Ambar ON"

@app.route('/call')
def call():
    numero = 0
    while numero <= 10:
        with PiStop(hwSetup="B+") as myPiStop:
            myPiStop.output(R,ON)
            time.sleep(0.1)
            myPiStop.output(R,OFF)
            time.sleep(0.1)
            myPiStop.output(Y,ON) #Switch Yellow lights ON
            time.sleep(0.1) 
            myPiStop.output(Y,OFF)
            time.sleep(0.1)        
            myPiStop.output(G,ON)
            time.sleep(0.1) 
            myPiStop.output(G,OFF)
        numero = numero + 1
    return "LLamada"
                        
if __name__ == "__main__":
    app.run(debug=True, host='0.0.0.0')
      
