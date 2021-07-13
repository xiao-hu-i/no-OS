  
# Copyright (C) 2019 Analog Devices, Inc.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#     - Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     - Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#     - Neither the name of Analog Devices, Inc. nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#     - The use of this software may or may not infringe the patent rights
#       of one or more patent holders.  This license does not release you
#       from the requirement that you obtain separate licenses from these
#       patent holders to use this software.
#     - Use of the software either in source or binary form, must be run
#       on or directly connected to an Analog Devices Inc. component.
#
# THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED.
#
# IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
# RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
# THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from ad3552r import ad3552r
import matplotlib.pyplot as plt
import numpy as np

NB_CH = 1
AMP = 60000
SAMPLES = 10000
PERIODS = 100
#MODE = 0 #DAC
MODE = 2 #input
LDAC_PERIOD_NS = 100000
#TRIGGER_MODE = 1 #dac delay trigger
TRIGGER_MODE = "software" #internal ldac
NB_TIMES = 10
PRINT_SIN = 0

def create_sin():
        N = SAMPLES
        fc = PERIODS
        ts = 1 / float(N)
        t = np.arange(0, N * ts, ts)
        d1 = (np.sin(2 * np.pi * t * fc) + 1) / 2 * AMP
        d2 = (np.cos(2 * np.pi * t * fc) + 1) / 2 * AMP
        #print(len(d1))
        if NB_CH == 1:
                return (t, d1)
        else:
                return (t, [d1, d2])

def ramp(t):
        return t % 20000

def create_ramp():
        N = SAMPLES
        MAX = AMP
        t = np.arange(0, N * 2)
        d = np.arange(0, MAX, MAX / N)
        d2 = np.arange(MAX, 0, -MAX / N)
        d3 = np.append(d, d2)
        dd3 = d3 / 2
        #return (t, ramp(t))
        if NB_CH == 1:
                return (t, d3)
        else:
                return (t, [d3, dd3])

def print_sin(t, data):
        #plt.plot(t, np.transpose(outs)) 
        plt.plot(t, np.transpose(data))
        plt.title("numpy.sin()") 
        plt.xlabel("X") 
        plt.ylabel("Y")
        plt.show()

(t, _sin) = create_sin()
#(t, _sin) = create_ramp()
if PRINT_SIN == 1:
        print_sin(t, _sin)

# Set up AD7124
dev = ad3552r(uri="serial:COM19")
#dev = ad3552r(uri="ip:192.168.100.100")
print('Conected')

if NB_CH == 1:
        dev.tx_enabled_channels = [0]
        #dev.tx_enabled_channels = [1]
else:
        dev.tx_enabled_channels = [0, 1]

MAX_SIZE = 10000
if NB_TIMES == 0:
        inc = 0
else:
        inc = 1

i = 0
while i <= NB_TIMES:
        j = 0
        sz = len(_sin)
        while j < sz:
                tmp = min(sz - j, MAX_SIZE)
                new_data = _sin[j : j + tmp]
                #print(new_data)
                dev.tx(new_data)
                k = 0
                while k < tmp:

                        k = k + 1

                j = j + tmp
        if i % 1000 == 0:
                print("Sent %d" % i)
        i = i + inc
