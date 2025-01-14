/***************************************************************************//**
 *   @file   adaq8092_fmc.c
 *   @brief  ADAQ8092_FMC Application
 *   @author Antoniu Miclaus (antoniu.miclaus@analog.com)
********************************************************************************
 * Copyright 2022(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "xil_cache.h"
#include "xparameters.h"
#include "axi_adc_core.h"
#include "axi_dmac.h"
#include "adaq8092.h"
#include "no_os_spi.h"
#include "no_os_gpio.h"
#include "spi_extra.h"
#include "parameters.h"
#include "no_os_error.h"
#include "gpio_extra.h"

#include "no_os_print_log.h"

#ifdef IIO_SUPPORT
#include "iio_app.h"
#include "iio_axi_adc.h"
#endif

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
#define ADAQ8092_SAMPLES_PER_CH	1000
#define ADAQ8092_NUM_CH		2

/***************************************************************************//**
* @brief main
*******************************************************************************/
int main(void)
{
	int ret;
	uint16_t adc_buffer[ADAQ8092_SAMPLES_PER_CH * ADAQ8092_NUM_CH]
	__attribute__ ((aligned));

	struct xil_spi_init_param xil_spi_init = {
		.flags = 0,
		.type = SPI_PS
	};

	/* SPI */
	struct no_os_spi_init_param adaq8092_spi_param = {
		.device_id = SPI_DEVICE_ID,
		.max_speed_hz =  1000000u,
		.chip_select = 0,
		.mode = NO_OS_SPI_MODE_0,
		.platform_ops = &xil_spi_ops,
		.extra = &xil_spi_init
	};

	/* GPIO */
	struct xil_gpio_init_param xil_gpio_init = {
		.device_id = GPIO_DEVICE_ID,
		.type = GPIO_PS
	};

	struct no_os_gpio_init_param gpio_par_ser_init_param = {
		.number = GPIO_PAR_SER_NR,
		.platform_ops = &xil_gpio_ops,
		.extra = &xil_gpio_init
	};

	struct no_os_gpio_init_param gpio_adc_pd1_param = {
		.number = GPIO_PD1_NR,
		.platform_ops = &xil_gpio_ops,
		.extra = &xil_gpio_init
	};

	struct no_os_gpio_init_param gpio_adc_pd2_param = {
		.number = GPIO_PD2_NR,
		.platform_ops = &xil_gpio_ops,
		.extra = &xil_gpio_init
	};

	struct no_os_gpio_init_param gpio_en_1v8_param = {
		.number = GPIO_1V8_NR,
		.platform_ops = &xil_gpio_ops,
		.extra = &xil_gpio_init
	};

	/* ADC Core */
	struct axi_adc_init adaq8092_core_param = {
		.name = "adaq8092_core",
		.num_channels = 2,
		.base = RX_CORE_BASEADDR
	};
	struct axi_adc *adaq8092_core;

	/* AXI DMAC */
	struct axi_dmac_init adaq8092_dmac_param = {
		.name = "adaq8092_dmac",
		.base = RX_DMA_BASEADDR,
		.direction = DMA_DEV_TO_MEM,
		.flags = 0
	};
	struct axi_dmac *adaq8092_dmac;

	struct adaq8092_init_param adaq8092_init_param = {
		.spi_init = &adaq8092_spi_param,
		.gpio_adc_pd1_param = &gpio_adc_pd1_param,
		.gpio_adc_pd2_param = &gpio_adc_pd2_param,
		.gpio_en_1p8_param = &gpio_en_1v8_param,
		.gpio_par_ser_param = &gpio_par_ser_init_param,
		.pd_mode = ADAQ8092_NORMAL_OP,
		.clk_pol_mode = ADAQ8092_CLK_POL_INVERTED,
		.clk_phase_mode = ADAQ8092_NO_DELAY,
		.clk_dc_mode = ADAQ8092_CLK_DC_STABILIZER_OFF,
		.lvds_cur_mode = ADAQ8092_3M5A,
		.lvds_term_mode = ADAQ8092_TERM_OFF,
		.dout_en = ADAQ8092_DOUT_ON,
		.dout_mode = ADAQ8092_DOUBLE_RATE_LVDS,
		.test_mode = ADAQ8092_TEST_CHECKERBOARD,
		.alt_bit_pol_en = ADAQ8092_ALT_BIT_POL_OFF,
		.data_rand_en = ADAQ8092_DATA_RAND_OFF,
		.twos_comp = ADAQ8092_TWOS_COMPLEMENT
	};
	struct adaq8092_dev *adaq8092_device;

	ret = adaq8092_init(&adaq8092_device, adaq8092_init_param);
	if (ret) {
		pr_err("ADAQ8092 device initialization failed!");
		return ret;
	}

	ret = axi_adc_init(&adaq8092_core,  &adaq8092_core_param);
	if (ret) {
		pr_err("axi_adc_init() error: %s\n", adaq8092_core->name);
		return ret;
	}

	ret = axi_dmac_init(&adaq8092_dmac, &adaq8092_dmac_param);
	if (ret) {
		pr_err("axi_dmac_init() error: %s\n", adaq8092_dmac->name);
		return ret;
	}

	pr_info("Start Caputre with Test pattern - Checkerboard \n");

	ret = axi_dmac_transfer(adaq8092_dmac, (uintptr_t)adc_buffer,
				sizeof(adc_buffer));
	if (ret) {
		pr_err("axi_dmac_transfer() failed!\n");
		return ret;
	}

	for (int i = 0; i < ADAQ8092_SAMPLES_PER_CH; i+=2)
		pr_info("CH1: %d CH2: %d \n",adc_buffer[i], adc_buffer[i+1]);

	ret = adaq8092_set_test_mode(adaq8092_device, ADAQ8092_TEST_OFF);
	if (ret)
		return ret;

	pr_info("\n Capture done.\n");

#ifdef IIO_SUPPORT
	struct iio_axi_adc_desc *iio_axi_adc_desc;
	struct iio_device *dev_desc;
	struct iio_axi_adc_init_param iio_axi_adc_init_par;
	iio_axi_adc_init_par = (struct iio_axi_adc_init_param) {
		.rx_adc = adaq8092_core,
		.rx_dmac = adaq8092_dmac,
		.dcache_invalidate_range = (void (*)(uint32_t,
						     uint32_t))Xil_DCacheInvalidateRange
	};

	struct iio_data_buffer read_buff = {
		.buff = (void *)adc_buffer,
		.size = 0xFFFFFFFF,
	};

	ret = iio_axi_adc_init(&iio_axi_adc_desc, &iio_axi_adc_init_par);
	if (ret < 0)
		return ret;
	iio_axi_adc_get_dev_descriptor(iio_axi_adc_desc, &dev_desc);

	struct iio_app_device devices[] = {
		IIO_APP_DEVICE("adaq8092_dev", iio_axi_adc_desc, dev_desc,
			       &read_buff, NULL),
	};

	return iio_app_run(devices, NO_OS_ARRAY_SIZE(devices));
#endif

	return 0;
}
