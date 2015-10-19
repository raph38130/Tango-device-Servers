import tkinter
import threading

class MyTkApp(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        self.root=tkinter.Tk()
        self.c=tkinter.Canvas(width=800,height=600)
        self.c.pack()
        self.root.mainloop()


