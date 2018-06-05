/**
 * Wooting RGB control DLL loader.
 */

#include "pdll.h"

typedef unsigned int uint;

class WootingRGB : public PDLL {
	DECLARE_CLASS(WootingRGB);
	
	DECLARE_FUNCTION0(bool, wooting_rgb_kbd_connected);
	DECLARE_FUNCTION0(bool, wooting_rgb_reset);
	DECLARE_FUNCTION5(bool, wooting_rgb_direct_set_key, uint, uint, uint, uint, uint);
	DECLARE_FUNCTION2(bool, wooting_rgb_direct_reset_key, uint, uint);
	DECLARE_FUNCTION0(bool, wooting_rgb_array_update_keyboard);
	DECLARE_FUNCTION1(bool, wooting_rgb_array_auto_update, bool);
	DECLARE_FUNCTION5(bool, wooting_rgb_array_set_single, uint, uint, uint, uint, uint);
};
