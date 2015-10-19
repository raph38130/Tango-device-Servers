# -*- coding: utf-8 -*-
#
# This file is part of the Sense_Hat project
#
# 
#
# Distributed under the terms of the GPL license.
# See LICENSE.txt for more info.

""" astro pi / sense hat for the raspberry pi 2

"""

__all__ = ["Sense_Hat", "main"]

# PyTango imports
import PyTango
from PyTango import DebugIt
from PyTango.server import run
from PyTango.server import Device, DeviceMeta
from PyTango.server import attribute, command
from PyTango.server import class_property, device_property
from PyTango import AttrQuality, AttrWriteType, DispLevel, DevState
# Additional import
# PROTECTED REGION ID(Sense_Hat.additionnal_import) ENABLED START #
# PROTECTED REGION END #    //  Sense_Hat.additionnal_import


class Sense_Hat(Device):
    """
    """
    __metaclass__ = DeviceMeta
    # PROTECTED REGION ID(Sense_Hat.class_variable) ENABLED START #
    # PROTECTED REGION END #    //  Sense_Hat.class_variable
    # ----------------
    # Class Properties
    # ----------------

    # -----------------
    # Device Properties
    # -----------------

    # ----------
    # Attributes
    # ----------

    Temperature = attribute(
        dtype='double',
    )

    Pressure = attribute(
        dtype='double',
    )

    Compass = attribute(
        dtype=('double',),
        max_dim_x=3,
    )

    Gyro = attribute(
        dtype=('double',),
        max_dim_x=3,
    )

    LedMatrix = attribute(
        dtype=(('int',),),
        access=AttrWriteType.WRITE,
        max_dim_x=8, max_dim_y=8,
    )

    # ---------------
    # General methods
    # ---------------

    def init_device(self):
        Device.init_device(self)
        # PROTECTED REGION ID(Sense_Hat.init_device) ENABLED START #
        # PROTECTED REGION END #    //  Sense_Hat.init_device

    def always_executed_hook(self):
        # PROTECTED REGION ID(Sense_Hat.always_executed_hook) ENABLED START #
        pass
        # PROTECTED REGION END #    //  Sense_Hat.always_executed_hook

    def delete_device(self):
        # PROTECTED REGION ID(Sense_Hat.delete_device) ENABLED START #
        pass
        # PROTECTED REGION END #    //  Sense_Hat.delete_device

    # ------------------
    # Attributes methods
    # ------------------

    def read_Temperature(self):
        # PROTECTED REGION ID(Sense_Hat.Temperature_read) ENABLED START #
        return 0.0
        # PROTECTED REGION END #    //  Sense_Hat.Temperature_read

    def read_Pressure(self):
        # PROTECTED REGION ID(Sense_Hat.Pressure_read) ENABLED START #
        return 0.0
        # PROTECTED REGION END #    //  Sense_Hat.Pressure_read

    def read_Compass(self):
        # PROTECTED REGION ID(Sense_Hat.Compass_read) ENABLED START #
        return [0.0]
        # PROTECTED REGION END #    //  Sense_Hat.Compass_read

    def read_Gyro(self):
        # PROTECTED REGION ID(Sense_Hat.Gyro_read) ENABLED START #
        return [0.0]
        # PROTECTED REGION END #    //  Sense_Hat.Gyro_read

    def write_LedMatrix(self, value):
        # PROTECTED REGION ID(Sense_Hat.LedMatrix_write) ENABLED START #
        pass
        # PROTECTED REGION END #    //  Sense_Hat.LedMatrix_write

    # --------
    # Commands
    # --------

    @command
    @DebugIt()
    def SwitchOff(self):
        # PROTECTED REGION ID(Sense_Hat.SwitchOff) ENABLED START #
        pass
        # PROTECTED REGION END #    //  Sense_Hat.SwitchOff

# ----------
# Run server
# ----------


def main(args=None, **kwargs):
    from PyTango.server import run
    return run((PyHL,), args=args, **kwargs)

if __name__ == '__main__':
    main()
