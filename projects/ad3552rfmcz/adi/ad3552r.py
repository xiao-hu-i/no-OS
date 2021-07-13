from decimal import Decimal

import numpy as np
from adi.attribute import attribute
from adi.context_manager import context_manager
from adi.rx_tx import tx


class ad3552r(tx, context_manager, attribute):
    """ AD3552r ADC """

    _complex_data = False
    _tx_channel_names = ["dac0", "dac1", "dac0_prec", "dac1_prec"]
    _device_name = ""
    # Litell endian, unsinged, 2 bytes
    _tx_data_type = '>u2'
    # big endian, unsinged, 2 bytes
    #_tx_data_type = '<u2'

    def __init__(self, uri="", device_index=0):

        context_manager.__init__(self, uri, self._device_name)
        self._txdac = self._ctx.find_device("ad3552r")
        self._ctrl = self._txdac
        tx.__init__(self)

    @property
    def ldac_period_us(self):
        """ldac_period_us"""
        # Only need to consider one channel, all others follow
        return self._get_iio_dev_attr("ldac_period_us")

    @ldac_period_us.setter
    def ldac_period_us(self, value):
        self._set_iio_dev_attr_str("ldac_period_us", value)
    
    @property
    def update_mode(self):
        """update_mode"""
        # Only need to consider one channel, all others follow
        return self._get_iio_dev_attr("update_mode")

    @update_mode.setter
    def update_mode(self, value):
        self._set_iio_dev_attr_str("update_mode", value)