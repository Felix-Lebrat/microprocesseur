test: test_speed.pss
	../pssembly test_speed.pss && cp test_speed.pss.rom fe_ROM0

generate : clock.pss
	../pssembly clock.pss && cp clock.pss.rom fe_ROM0

gen_speed: clock_speed.pss
	../pssembly clock_speed.pss && cp clock_speed.pss.rom fe_ROM0
.PHONY : generate
