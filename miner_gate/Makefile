# Makefile to build monte_pi_sprng program
# --- macros
CC=arm-none-linux-gnueabi-g++
#MUST OPTIMISE -O3 FOR 1.4!
CFLAGS= -O3 -fpermissive -rdynamic -lpthread -static-libstdc++ -static-libgcc -DDC2DC_CHECK_ON_ERROR -DMINERGATE
OBJECTS= 
LIBS = -L./ -lpthread
SRCS=zabbix_log.c pll.c asic_thermal.c board.c scaling_dc2dc.c scaling_ac2dc.c leds.c pwm_manager.cpp  spond_debug.cpp corner_discovery.c i2c.c dc2dc.c ac2dc.c  json.c miner_gate.c mg_proto_parser.c scaling_manager.c hammer_lib.c nvm.c squid.cpp sha256.c real_time_queue.c 
INCLUDES = -I./


# --- targets
all:   miner_gate_arm 
miner_gate_arm:   $(SRCS) Makefile
	$(CC)  -o miner_gate_arm  $(SRCS) $(CFLAGS) $(INCLUDES)
        

# --- remove binary and executable files
clean:
	rm -f miner_gate_arm

depend: $(SRCS)
	makedepend $(INCLUDES) --$(CFLAGS) --$(SRCS) $^

# DO NOT DELETE THIS LINE -- make depend needs it

pll.o: /usr/include/stdio.h /usr/include/features.h
pll.o: /usr/include/stdc-predef.h /usr/include/libio.h
pll.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
pll.o: /usr/include/alloca.h pll.h /usr/include/stdint.h
pll.o: /usr/include/unistd.h /usr/include/getopt.h nvm.h dc2dc.h hammer.h
pll.o: pwm_manager.h squid.h spond_debug.h /usr/include/assert.h
pll.o: /usr/include/syslog.h
asic_testboard.o: defines.h i2c.h /usr/include/errno.h
asic_testboard.o: /usr/include/features.h /usr/include/stdc-predef.h
asic_testboard.o: /usr/include/string.h /usr/include/xlocale.h
asic_testboard.o: /usr/include/stdio.h /usr/include/libio.h
asic_testboard.o: /usr/include/_G_config.h /usr/include/wchar.h
asic_testboard.o: /usr/include/stdlib.h /usr/include/alloca.h
asic_testboard.o: /usr/include/unistd.h /usr/include/getopt.h
asic_testboard.o: /usr/include/fcntl.h /usr/include/time.h
asic_testboard.o: /usr/include/stdint.h spond_debug.h /usr/include/assert.h
asic_testboard.o: /usr/include/syslog.h asic_testboard.h
pwm_manager.o: pwm_manager.h /usr/include/stdint.h /usr/include/features.h
pwm_manager.o: /usr/include/stdc-predef.h /usr/include/unistd.h
pwm_manager.o: /usr/include/getopt.h /usr/include/stdio.h
pwm_manager.o: /usr/include/libio.h /usr/include/_G_config.h
pwm_manager.o: /usr/include/wchar.h /usr/include/string.h
pwm_manager.o: /usr/include/xlocale.h spond_debug.h /usr/include/assert.h
pwm_manager.o: /usr/include/syslog.h hammer.h nvm.h dc2dc.h hammer_lib.h
spond_debug.o: /usr/include/stdio.h /usr/include/features.h
spond_debug.o: /usr/include/stdc-predef.h /usr/include/libio.h
spond_debug.o: /usr/include/_G_config.h /usr/include/wchar.h
spond_debug.o: /usr/include/stdlib.h /usr/include/alloca.h
spond_debug.o: /usr/include/fcntl.h /usr/include/time.h
spond_debug.o: /usr/include/execinfo.h /usr/include/linux/types.h
spond_debug.o: /usr/include/linux/posix_types.h /usr/include/linux/stddef.h
spond_debug.o: /usr/include/syslog.h spond_debug.h /usr/include/assert.h
corner_discovery.o: /usr/include/stdio.h /usr/include/features.h
corner_discovery.o: /usr/include/stdc-predef.h /usr/include/libio.h
corner_discovery.o: /usr/include/_G_config.h /usr/include/wchar.h
corner_discovery.o: /usr/include/stdlib.h /usr/include/alloca.h
corner_discovery.o: corner_discovery.h nvm.h /usr/include/stdint.h
corner_discovery.o: /usr/include/unistd.h /usr/include/getopt.h dc2dc.h
corner_discovery.o: hammer.h pwm_manager.h defines.h scaling_manager.h
corner_discovery.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h
corner_discovery.o: hammer_lib.h squid.h
i2c.o: i2c.h /usr/include/errno.h /usr/include/features.h
i2c.o: /usr/include/stdc-predef.h /usr/include/string.h
i2c.o: /usr/include/xlocale.h /usr/include/stdio.h /usr/include/libio.h
i2c.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
i2c.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/getopt.h
i2c.o: /usr/include/fcntl.h /usr/include/time.h /usr/include/stdint.h
i2c.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h i2c-mydev.h
i2c.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
i2c.o: /usr/include/linux/stddef.h /usr/include/pthread.h
i2c.o: /usr/include/endian.h /usr/include/sched.h
dc2dc.o: dc2dc.h i2c.h /usr/include/errno.h /usr/include/features.h
dc2dc.o: /usr/include/stdc-predef.h /usr/include/string.h
dc2dc.o: /usr/include/xlocale.h /usr/include/stdio.h /usr/include/libio.h
dc2dc.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
dc2dc.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/getopt.h
dc2dc.o: /usr/include/fcntl.h /usr/include/time.h /usr/include/stdint.h
dc2dc.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h nvm.h
dc2dc.o: hammer.h pwm_manager.h defines.h
ac2dc.o: ac2dc_const.h ac2dc.h dc2dc.h i2c.h /usr/include/errno.h
ac2dc.o: /usr/include/features.h /usr/include/stdc-predef.h
ac2dc.o: /usr/include/string.h /usr/include/xlocale.h /usr/include/stdio.h
ac2dc.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
ac2dc.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/unistd.h
ac2dc.o: /usr/include/getopt.h /usr/include/fcntl.h /usr/include/time.h
ac2dc.o: /usr/include/stdint.h spond_debug.h /usr/include/assert.h
ac2dc.o: /usr/include/syslog.h hammer.h nvm.h pwm_manager.h
json.o: json.h spond_debug.h /usr/include/assert.h /usr/include/features.h
json.o: /usr/include/stdc-predef.h /usr/include/syslog.h
json.o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/libio.h
json.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
json.o: /usr/include/alloca.h /usr/include/string.h /usr/include/xlocale.h
miner_gate.o: defines.h mg_proto_parser.h /usr/include/stdio.h
miner_gate.o: /usr/include/features.h /usr/include/stdc-predef.h
miner_gate.o: /usr/include/libio.h /usr/include/_G_config.h
miner_gate.o: /usr/include/wchar.h /usr/include/stdlib.h
miner_gate.o: /usr/include/alloca.h /usr/include/getopt.h
miner_gate.o: /usr/include/fcntl.h /usr/include/time.h
miner_gate.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
miner_gate.o: /usr/include/linux/stddef.h /usr/include/linux/spi/spidev.h
miner_gate.o: /usr/include/netinet/in.h /usr/include/stdint.h
miner_gate.o: /usr/include/endian.h /usr/include/unistd.h
miner_gate.o: /usr/include/string.h /usr/include/xlocale.h
miner_gate.o: /usr/include/pthread.h /usr/include/sched.h hammer.h nvm.h
miner_gate.o: dc2dc.h pwm_manager.h spond_debug.h /usr/include/assert.h
miner_gate.o: /usr/include/syslog.h squid.h i2c.h /usr/include/errno.h
miner_gate.o: ac2dc.h hammer_lib.h miner_gate.h scaling_manager.h
miner_gate.o: corner_discovery.h asic_testboard.h
mg_proto_parser.o: mg_proto_parser.h /usr/include/stdio.h
mg_proto_parser.o: /usr/include/features.h /usr/include/stdc-predef.h
mg_proto_parser.o: /usr/include/libio.h /usr/include/_G_config.h
mg_proto_parser.o: /usr/include/wchar.h /usr/include/stdlib.h
mg_proto_parser.o: /usr/include/alloca.h /usr/include/getopt.h
mg_proto_parser.o: /usr/include/fcntl.h /usr/include/time.h
mg_proto_parser.o: /usr/include/linux/types.h
mg_proto_parser.o: /usr/include/linux/posix_types.h
mg_proto_parser.o: /usr/include/linux/stddef.h
mg_proto_parser.o: /usr/include/linux/spi/spidev.h /usr/include/netinet/in.h
mg_proto_parser.o: /usr/include/stdint.h /usr/include/endian.h
mg_proto_parser.o: /usr/include/assert.h
scaling_manager.o: squid.h /usr/include/stdint.h /usr/include/features.h
scaling_manager.o: /usr/include/stdc-predef.h /usr/include/unistd.h
scaling_manager.o: /usr/include/getopt.h /usr/include/stdio.h
scaling_manager.o: /usr/include/libio.h /usr/include/_G_config.h
scaling_manager.o: /usr/include/wchar.h /usr/include/string.h
scaling_manager.o: /usr/include/xlocale.h /usr/include/stdlib.h
scaling_manager.o: /usr/include/alloca.h /usr/include/time.h
scaling_manager.o: /usr/include/fcntl.h /usr/include/linux/types.h
scaling_manager.o: /usr/include/linux/posix_types.h
scaling_manager.o: /usr/include/linux/stddef.h
scaling_manager.o: /usr/include/linux/spi/spidev.h /usr/include/netinet/in.h
scaling_manager.o: /usr/include/endian.h queue.h pll.h nvm.h dc2dc.h
scaling_manager.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h
scaling_manager.o: hammer.h pwm_manager.h ac2dc.h hammer_lib.h
scaling_manager.o: scaling_manager.h
hammer_lib.o: squid.h /usr/include/stdint.h /usr/include/features.h
hammer_lib.o: /usr/include/stdc-predef.h /usr/include/unistd.h
hammer_lib.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
hammer_lib.o: /usr/include/_G_config.h /usr/include/wchar.h
hammer_lib.o: /usr/include/stdlib.h /usr/include/alloca.h
hammer_lib.o: /usr/include/fcntl.h /usr/include/time.h
hammer_lib.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
hammer_lib.o: /usr/include/linux/stddef.h /usr/include/linux/spi/spidev.h
hammer_lib.o: /usr/include/netinet/in.h /usr/include/endian.h
hammer_lib.o: mg_proto_parser.h hammer.h nvm.h dc2dc.h pwm_manager.h queue.h
hammer_lib.o: /usr/include/string.h /usr/include/xlocale.h ac2dc.h
hammer_lib.o: hammer_lib.h /usr/include/pthread.h /usr/include/sched.h
hammer_lib.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h
hammer_lib.o: real_time_queue.h miner_gate.h defines.h scaling_manager.h
nvm.o: squid.h /usr/include/stdint.h /usr/include/features.h
nvm.o: /usr/include/stdc-predef.h /usr/include/unistd.h /usr/include/getopt.h
nvm.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
nvm.o: /usr/include/wchar.h /usr/include/stdlib.h /usr/include/alloca.h
nvm.o: /usr/include/fcntl.h /usr/include/time.h /usr/include/linux/types.h
nvm.o: /usr/include/linux/posix_types.h /usr/include/linux/stddef.h
nvm.o: /usr/include/linux/spi/spidev.h /usr/include/netinet/in.h
nvm.o: /usr/include/endian.h nvm.h dc2dc.h hammer.h pwm_manager.h queue.h
nvm.o: /usr/include/string.h /usr/include/xlocale.h spond_debug.h
nvm.o: /usr/include/assert.h /usr/include/syslog.h hammer_lib.h
squid.o: squid.h /usr/include/stdint.h /usr/include/features.h
squid.o: /usr/include/stdc-predef.h /usr/include/unistd.h
squid.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
squid.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
squid.o: /usr/include/alloca.h /usr/include/fcntl.h /usr/include/time.h
squid.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
squid.o: /usr/include/linux/stddef.h /usr/include/linux/spi/spidev.h
squid.o: /usr/include/netinet/in.h /usr/include/endian.h
squid.o: /usr/include/string.h /usr/include/xlocale.h hammer.h nvm.h dc2dc.h
squid.o: pwm_manager.h spond_debug.h /usr/include/assert.h
squid.o: /usr/include/syslog.h
sha256.o: /usr/include/stdint.h /usr/include/features.h
sha256.o: /usr/include/stdc-predef.h /usr/include/stdio.h
sha256.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
sha256.o: /usr/include/unistd.h /usr/include/getopt.h /usr/include/string.h
sha256.o: /usr/include/xlocale.h hammer.h nvm.h dc2dc.h pwm_manager.h
sha256.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h
real_time_queue.o: squid.h /usr/include/stdint.h /usr/include/features.h
real_time_queue.o: /usr/include/stdc-predef.h /usr/include/unistd.h
real_time_queue.o: /usr/include/getopt.h /usr/include/stdio.h
real_time_queue.o: /usr/include/libio.h /usr/include/_G_config.h
real_time_queue.o: /usr/include/wchar.h /usr/include/stdlib.h
real_time_queue.o: /usr/include/alloca.h /usr/include/fcntl.h
real_time_queue.o: /usr/include/time.h /usr/include/linux/types.h
real_time_queue.o: /usr/include/linux/posix_types.h
real_time_queue.o: /usr/include/linux/stddef.h
real_time_queue.o: /usr/include/linux/spi/spidev.h /usr/include/netinet/in.h
real_time_queue.o: /usr/include/endian.h hammer.h nvm.h dc2dc.h pwm_manager.h
real_time_queue.o: queue.h /usr/include/string.h /usr/include/xlocale.h
real_time_queue.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h
real_time_queue.o: hammer_lib.h real_time_queue.h
pll.o: /usr/include/stdio.h /usr/include/features.h
pll.o: /usr/include/stdc-predef.h /usr/include/libio.h
pll.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
pll.o: /usr/include/alloca.h pll.h /usr/include/stdint.h
pll.o: /usr/include/unistd.h /usr/include/getopt.h nvm.h dc2dc.h hammer.h
pll.o: pwm_manager.h squid.h spond_debug.h /usr/include/assert.h
pll.o: /usr/include/syslog.h
asic_testboard.o: defines.h i2c.h /usr/include/errno.h
asic_testboard.o: /usr/include/features.h /usr/include/stdc-predef.h
asic_testboard.o: /usr/include/string.h /usr/include/xlocale.h
asic_testboard.o: /usr/include/stdio.h /usr/include/libio.h
asic_testboard.o: /usr/include/_G_config.h /usr/include/wchar.h
asic_testboard.o: /usr/include/stdlib.h /usr/include/alloca.h
asic_testboard.o: /usr/include/unistd.h /usr/include/getopt.h
asic_testboard.o: /usr/include/fcntl.h /usr/include/time.h
asic_testboard.o: /usr/include/stdint.h spond_debug.h /usr/include/assert.h
asic_testboard.o: /usr/include/syslog.h asic_testboard.h
pwm_manager.o: pwm_manager.h /usr/include/stdint.h /usr/include/features.h
pwm_manager.o: /usr/include/stdc-predef.h /usr/include/unistd.h
pwm_manager.o: /usr/include/getopt.h /usr/include/stdio.h
pwm_manager.o: /usr/include/libio.h /usr/include/_G_config.h
pwm_manager.o: /usr/include/wchar.h /usr/include/string.h
pwm_manager.o: /usr/include/xlocale.h spond_debug.h /usr/include/assert.h
pwm_manager.o: /usr/include/syslog.h hammer.h nvm.h dc2dc.h hammer_lib.h
spond_debug.o: /usr/include/stdio.h /usr/include/features.h
spond_debug.o: /usr/include/stdc-predef.h /usr/include/libio.h
spond_debug.o: /usr/include/_G_config.h /usr/include/wchar.h
spond_debug.o: /usr/include/stdlib.h /usr/include/alloca.h
spond_debug.o: /usr/include/fcntl.h /usr/include/time.h
spond_debug.o: /usr/include/execinfo.h /usr/include/linux/types.h
spond_debug.o: /usr/include/linux/posix_types.h /usr/include/linux/stddef.h
spond_debug.o: /usr/include/syslog.h spond_debug.h /usr/include/assert.h
corner_discovery.o: /usr/include/stdio.h /usr/include/features.h
corner_discovery.o: /usr/include/stdc-predef.h /usr/include/libio.h
corner_discovery.o: /usr/include/_G_config.h /usr/include/wchar.h
corner_discovery.o: /usr/include/stdlib.h /usr/include/alloca.h
corner_discovery.o: corner_discovery.h nvm.h /usr/include/stdint.h
corner_discovery.o: /usr/include/unistd.h /usr/include/getopt.h dc2dc.h
corner_discovery.o: hammer.h pwm_manager.h defines.h scaling_manager.h
corner_discovery.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h
corner_discovery.o: hammer_lib.h squid.h
i2c.o: i2c.h /usr/include/errno.h /usr/include/features.h
i2c.o: /usr/include/stdc-predef.h /usr/include/string.h
i2c.o: /usr/include/xlocale.h /usr/include/stdio.h /usr/include/libio.h
i2c.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
i2c.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/getopt.h
i2c.o: /usr/include/fcntl.h /usr/include/time.h /usr/include/stdint.h
i2c.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h i2c-mydev.h
i2c.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
i2c.o: /usr/include/linux/stddef.h /usr/include/pthread.h
i2c.o: /usr/include/endian.h /usr/include/sched.h
dc2dc.o: dc2dc.h i2c.h /usr/include/errno.h /usr/include/features.h
dc2dc.o: /usr/include/stdc-predef.h /usr/include/string.h
dc2dc.o: /usr/include/xlocale.h /usr/include/stdio.h /usr/include/libio.h
dc2dc.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
dc2dc.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/getopt.h
dc2dc.o: /usr/include/fcntl.h /usr/include/time.h /usr/include/stdint.h
dc2dc.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h nvm.h
dc2dc.o: hammer.h pwm_manager.h defines.h
ac2dc.o: ac2dc_const.h ac2dc.h dc2dc.h i2c.h /usr/include/errno.h
ac2dc.o: /usr/include/features.h /usr/include/stdc-predef.h
ac2dc.o: /usr/include/string.h /usr/include/xlocale.h /usr/include/stdio.h
ac2dc.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
ac2dc.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/unistd.h
ac2dc.o: /usr/include/getopt.h /usr/include/fcntl.h /usr/include/time.h
ac2dc.o: /usr/include/stdint.h spond_debug.h /usr/include/assert.h
ac2dc.o: /usr/include/syslog.h hammer.h nvm.h pwm_manager.h
json.o: json.h spond_debug.h /usr/include/assert.h /usr/include/features.h
json.o: /usr/include/stdc-predef.h /usr/include/syslog.h
json.o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/libio.h
json.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
json.o: /usr/include/alloca.h /usr/include/string.h /usr/include/xlocale.h
miner_gate.o: defines.h mg_proto_parser.h /usr/include/stdio.h
miner_gate.o: /usr/include/features.h /usr/include/stdc-predef.h
miner_gate.o: /usr/include/libio.h /usr/include/_G_config.h
miner_gate.o: /usr/include/wchar.h /usr/include/stdlib.h
miner_gate.o: /usr/include/alloca.h /usr/include/getopt.h
miner_gate.o: /usr/include/fcntl.h /usr/include/time.h
miner_gate.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
miner_gate.o: /usr/include/linux/stddef.h /usr/include/linux/spi/spidev.h
miner_gate.o: /usr/include/netinet/in.h /usr/include/stdint.h
miner_gate.o: /usr/include/endian.h /usr/include/unistd.h
miner_gate.o: /usr/include/string.h /usr/include/xlocale.h
miner_gate.o: /usr/include/pthread.h /usr/include/sched.h hammer.h nvm.h
miner_gate.o: dc2dc.h pwm_manager.h spond_debug.h /usr/include/assert.h
miner_gate.o: /usr/include/syslog.h squid.h i2c.h /usr/include/errno.h
miner_gate.o: ac2dc.h hammer_lib.h miner_gate.h scaling_manager.h
miner_gate.o: corner_discovery.h asic_testboard.h
mg_proto_parser.o: mg_proto_parser.h /usr/include/stdio.h
mg_proto_parser.o: /usr/include/features.h /usr/include/stdc-predef.h
mg_proto_parser.o: /usr/include/libio.h /usr/include/_G_config.h
mg_proto_parser.o: /usr/include/wchar.h /usr/include/stdlib.h
mg_proto_parser.o: /usr/include/alloca.h /usr/include/getopt.h
mg_proto_parser.o: /usr/include/fcntl.h /usr/include/time.h
mg_proto_parser.o: /usr/include/linux/types.h
mg_proto_parser.o: /usr/include/linux/posix_types.h
mg_proto_parser.o: /usr/include/linux/stddef.h
mg_proto_parser.o: /usr/include/linux/spi/spidev.h /usr/include/netinet/in.h
mg_proto_parser.o: /usr/include/stdint.h /usr/include/endian.h
mg_proto_parser.o: /usr/include/assert.h
scaling_manager.o: squid.h /usr/include/stdint.h /usr/include/features.h
scaling_manager.o: /usr/include/stdc-predef.h /usr/include/unistd.h
scaling_manager.o: /usr/include/getopt.h /usr/include/stdio.h
scaling_manager.o: /usr/include/libio.h /usr/include/_G_config.h
scaling_manager.o: /usr/include/wchar.h /usr/include/string.h
scaling_manager.o: /usr/include/xlocale.h /usr/include/stdlib.h
scaling_manager.o: /usr/include/alloca.h /usr/include/time.h
scaling_manager.o: /usr/include/fcntl.h /usr/include/linux/types.h
scaling_manager.o: /usr/include/linux/posix_types.h
scaling_manager.o: /usr/include/linux/stddef.h
scaling_manager.o: /usr/include/linux/spi/spidev.h /usr/include/netinet/in.h
scaling_manager.o: /usr/include/endian.h queue.h pll.h nvm.h dc2dc.h
scaling_manager.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h
scaling_manager.o: hammer.h pwm_manager.h ac2dc.h hammer_lib.h
scaling_manager.o: scaling_manager.h
hammer_lib.o: squid.h /usr/include/stdint.h /usr/include/features.h
hammer_lib.o: /usr/include/stdc-predef.h /usr/include/unistd.h
hammer_lib.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
hammer_lib.o: /usr/include/_G_config.h /usr/include/wchar.h
hammer_lib.o: /usr/include/stdlib.h /usr/include/alloca.h
hammer_lib.o: /usr/include/fcntl.h /usr/include/time.h
hammer_lib.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
hammer_lib.o: /usr/include/linux/stddef.h /usr/include/linux/spi/spidev.h
hammer_lib.o: /usr/include/netinet/in.h /usr/include/endian.h
hammer_lib.o: mg_proto_parser.h hammer.h nvm.h dc2dc.h pwm_manager.h queue.h
hammer_lib.o: /usr/include/string.h /usr/include/xlocale.h ac2dc.h
hammer_lib.o: hammer_lib.h /usr/include/pthread.h /usr/include/sched.h
hammer_lib.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h
hammer_lib.o: real_time_queue.h miner_gate.h defines.h scaling_manager.h
nvm.o: squid.h /usr/include/stdint.h /usr/include/features.h
nvm.o: /usr/include/stdc-predef.h /usr/include/unistd.h /usr/include/getopt.h
nvm.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
nvm.o: /usr/include/wchar.h /usr/include/stdlib.h /usr/include/alloca.h
nvm.o: /usr/include/fcntl.h /usr/include/time.h /usr/include/linux/types.h
nvm.o: /usr/include/linux/posix_types.h /usr/include/linux/stddef.h
nvm.o: /usr/include/linux/spi/spidev.h /usr/include/netinet/in.h
nvm.o: /usr/include/endian.h nvm.h dc2dc.h hammer.h pwm_manager.h queue.h
nvm.o: /usr/include/string.h /usr/include/xlocale.h spond_debug.h
nvm.o: /usr/include/assert.h /usr/include/syslog.h hammer_lib.h
squid.o: squid.h /usr/include/stdint.h /usr/include/features.h
squid.o: /usr/include/stdc-predef.h /usr/include/unistd.h
squid.o: /usr/include/getopt.h /usr/include/stdio.h /usr/include/libio.h
squid.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
squid.o: /usr/include/alloca.h /usr/include/fcntl.h /usr/include/time.h
squid.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
squid.o: /usr/include/linux/stddef.h /usr/include/linux/spi/spidev.h
squid.o: /usr/include/netinet/in.h /usr/include/endian.h
squid.o: /usr/include/string.h /usr/include/xlocale.h hammer.h nvm.h dc2dc.h
squid.o: pwm_manager.h spond_debug.h /usr/include/assert.h
squid.o: /usr/include/syslog.h
sha256.o: /usr/include/stdint.h /usr/include/features.h
sha256.o: /usr/include/stdc-predef.h /usr/include/stdio.h
sha256.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
sha256.o: /usr/include/unistd.h /usr/include/getopt.h /usr/include/string.h
sha256.o: /usr/include/xlocale.h hammer.h nvm.h dc2dc.h pwm_manager.h
sha256.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h
real_time_queue.o: squid.h /usr/include/stdint.h /usr/include/features.h
real_time_queue.o: /usr/include/stdc-predef.h /usr/include/unistd.h
real_time_queue.o: /usr/include/getopt.h /usr/include/stdio.h
real_time_queue.o: /usr/include/libio.h /usr/include/_G_config.h
real_time_queue.o: /usr/include/wchar.h /usr/include/stdlib.h
real_time_queue.o: /usr/include/alloca.h /usr/include/fcntl.h
real_time_queue.o: /usr/include/time.h /usr/include/linux/types.h
real_time_queue.o: /usr/include/linux/posix_types.h
real_time_queue.o: /usr/include/linux/stddef.h
real_time_queue.o: /usr/include/linux/spi/spidev.h /usr/include/netinet/in.h
real_time_queue.o: /usr/include/endian.h hammer.h nvm.h dc2dc.h pwm_manager.h
real_time_queue.o: queue.h /usr/include/string.h /usr/include/xlocale.h
real_time_queue.o: spond_debug.h /usr/include/assert.h /usr/include/syslog.h
real_time_queue.o: hammer_lib.h real_time_queue.h
