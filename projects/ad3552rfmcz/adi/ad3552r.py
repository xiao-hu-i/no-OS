from decimal import Decimal

import numpy as np
from adi.attribute import attribute
from adi.context_manager import context_manager
from adi.rx_tx import tx


class ad3552r(tx, context_manager):
    """ AD3552r ADC """

    _complex_data = False
    _tx_channel_names = ["dac0", "dac1", "dac0_prec", "dac1_prec"]
    _device_name = ""

    def __init__(self, uri="", device_index=0):

        context_manager.__init__(self, uri, self._device_name)
        self._txdac = self._ctx.find_device("ad3552r")
        tx.__init__(self)