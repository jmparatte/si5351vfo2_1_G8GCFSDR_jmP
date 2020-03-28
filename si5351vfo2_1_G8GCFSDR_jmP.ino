//SI5351_vfo2.1 + LCD + Output Clk0/Clk2

#include "si5351.h"
#include "Wire.h"
#include "EEPROM.h"

Si5351 si5351;

struct eeprom_s {
	uint8_t set;
	long freq;
	long f_if;
	long f_official;
	long f_actual;
}
eeprom;
//eeprom = {false, 900000, 12000, 900000, 900000}; // RAI 900kHz

void setup(void)
{

	Serial.begin(115200);
	Serial.println("si5351vfo2_1_G8GCFSDR_jmP");

	EEPROM.get( 0, eeprom );
//	Serial.println(eeprom.set);
//	Serial.println(eeprom.freq);
//	Serial.println(eeprom.f_if);
//	Serial.println(eeprom.f_official);
//	Serial.println(eeprom.f_actual);
	if (eeprom.set != true) eeprom = {false, 900000, 12000, 900000, 900000}; // RAI 900kHz
	Serial.println(eeprom.set);
	Serial.println(eeprom.freq);
	Serial.println(eeprom.f_if);
	Serial.println(eeprom.f_official);
	Serial.println(eeprom.f_actual);

//	si5351.init( SI5351_CRYSTAL_LOAD_0PF, 0, 0 );
//	si5351.init( SI5351_CRYSTAL_LOAD_6PF, 0, 0 );
//	si5351.init( SI5351_CRYSTAL_LOAD_8PF, 0, 0 );
	si5351.init( SI5351_CRYSTAL_LOAD_10PF, 0, 0 );

	si5351.set_correction( 100LL*((eeprom.f_official+eeprom.f_if) - (eeprom.f_actual+eeprom.f_if))*10000000/(eeprom.f_official+eeprom.f_if), SI5351_PLL_INPUT_XO );

//	Serial.println( eeprom.freq );
	si5351.set_pll( SI5351_PLL_FIXED, SI5351_PLLA );

	si5351.set_freq( 100LL*4*(eeprom.freq+eeprom.f_if), SI5351_CLK1 );
	si5351.output_enable(SI5351_CLK1, 1);

	Serial.println();

	Serial.setTimeout(1);
}

class ParsingBuffer : public Stream {

public:
	uint8_t _buf[32];
	size_t _len;
	size_t _idx;

	ParsingBuffer() {_len=0; _idx=0; _timeout=0;}

	int available() {return _len;}
	int read() {if (_idx<_len) return _buf[_idx++]; else return -1;}
	int peek() {if (_idx<_len) return _buf[_idx]; else return -1;}

	size_t write(uint8_t c) {if (_len<sizeof(_buf)) {_buf[_len++] = c; return 1;} else return 0;}
	void clear() {_len=0; _idx=0;}

} cmd;

void cmd_parse()
{
	int c = cmd.read();

	switch(c) {

	case 'v':
		Serial.print("1.2");
		break;

	case 'f':
		eeprom.freq = cmd.parseInt();
		si5351.set_freq( 100LL*4*(eeprom.freq+eeprom.f_if), SI5351_CLK1 );
		break;

	case 'c':
		eeprom.f_official = cmd.parseInt();
		eeprom.f_actual = cmd.parseInt();
		si5351.set_correction( 100LL*10000000LL*((eeprom.f_official+eeprom.f_if) - (eeprom.f_actual+eeprom.f_if))/(eeprom.f_official+eeprom.f_if), SI5351_PLL_INPUT_XO );
		eeprom.set = true;
		EEPROM.put( 0, eeprom );
		break;
	}

	cmd.clear();
}

void loop(void)
{
	si5351.update_status();
	if (si5351.dev_status.SYS_INIT == 1) {
		Serial.println("xxx");
		setup();
		delay(500);
	}

	if (Serial.available()) {
		int c = Serial.read();
		if (c=='\n') cmd_parse(); else if (c!='\r') cmd.write(c);
	}
}
