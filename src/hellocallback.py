import ctypes
from ctypes import *

def logwrapper(msg):
    print("{} {}".format("this from the logwrapper called by C", msg))
    return 1;
   

callback_type = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_char_p)

callback_func = callback_type(logwrapper)

c_logfilter = "../Debug/logfilter"
logfilter_c = CDLL(c_logfilter)
rundemos = logfilter_c.run_demos
#runmain.restype = POINTER(XMLValue)
#getVals.restype = POINTER(XMLValue)

rundemos(callback_func)
#runmain(callback_func)

#https://pgi-jcns.fz-juelich.de/portal/pages/using-c-from-python.html
#typedef int (*callback_type)(float, float);
#typedef int (*callback_type)(int, char*);