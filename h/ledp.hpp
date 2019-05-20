/*
 * ledp.hpp
 *
 *  Created on: May 20, 2019
 *      Author: Administrator
 */

#ifndef LEDP_HPP_
#define LEDP_HPP_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "bspconfig_tc23x.h"

class LED {
public:
	/* number of available LEDs */
	static uint8_t MAX_LED = 4;
	static Ifx_P PORT_LED = MODULE_P13;

	LED(void);
	LED(uint8_t n);
	~LED(void);

	static void init(void);

	void on(void);
	void off(void);
	void toggle();
	uint32_t stat(void);

private:
	bool m_has_init;
	uint8_t m_num;
};

void test_led_cpp(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LEDP_HPP_ */
