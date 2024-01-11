from pynq.overlays.base import BaseOverlay
from pynq.lib import Pmod_IO
from pynq.lib.pmod import PMODB


base = BaseOverlay("base.bit")
pIO = Pmod_IO(PMODB, 3, 'in')

#set the pin as input mode
prev = 0

while (1):
    value = pIO.read()
    if (value != prev):
        prev = value
        print(value)