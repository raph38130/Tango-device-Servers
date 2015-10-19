from time import time
from numpy.random import random_sample

from PyTango import AttrQuality, AttrWriteType, DispLevel
from PyTango.server import Device, DeviceMeta, attribute, command, run
from PyTango.server import class_property, device_property

class PowerSupply(PyTango.hlapi.Device):
    __metaclass__ = DeviceMeta

    voltage = attribute()

    current = attribute(label="Current", dtype=float,
                        display_level=DispLevel.EXPERT,
                        access=AttrWriteType.READ_WRITE,
                        unit="A", format="8.4f",
                        min_value=0.0, max_value=8.5,
                        min_alarm=0.1, max_alarm=8.4,
                        min_warning=0.5, max_warning=8.0,
                        fget="get_current", fset="set_current",
                        doc="the power supply current")

    noise = attribute(label="Noise", dtype=((float,),),
                      max_dim_x=1024, max_dim_y=1024,
                      fget="get_noise")

    host = device_property(dtype=str)
    port = class_property(dtype=int, default_value=9788)

    def read_voltage(self):
        self.info_stream("get voltage(%s, %d)" % (self.host, self.port))
        return 10.0

    def get_current(self):
        return 2.3456, time(), AttrQuality.ATTR_WARNING

    def set_current(self, current):
        print("Current set to %f" % current)

    def get_noise(self):
        return random_sample((1024, 1024))

    @command(dtype_in=float)
    def ramp(self, value):
        print("Ramping up...")

if __name__ == "__main__":
    run((PowerSupply,))

