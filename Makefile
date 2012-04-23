AVR_CC = avr-gcc
AVR_AS = avr-as
AVR_OBJCOPY = avr-objcopy
AVR_SIZE = avr-size

AVRDUDE = avrdude

MKDIR = mkdir -p
RM = rm -f

AVRDUDE_FLAGS = -p m168 -c usbasp

AVR_CFLAGS += -Os -finline-limit=800
AVR_CFLAGS += -mmcu=atmega168
AVR_CFLAGS += -DF_CPU=16000000UL
AVR_CFLAGS += -DAVR_EEPROM_SIZE=512

CFLAGS += -std=c99 -Wall -W
CFLAGS += -Isrc/pt
CFLAGS += "-DVERSION=\"(unknown)\""

OBJDIR = objdir

MODULES += blink fifo string_format test thread timer uart

EXTRA_DEPENDENCIES =
OBJECTS =

-include config.mk

OBJECTS += $(addsuffix .o,$(MODULES))

EXTRA_DEPENDENCIES += Makefile
EXTRA_DEPENDENCIES += $(foreach m,$(MODULES),src/$(m).h)
EXTRA_DEPENDENCIES += src/pt/pt.h src/pt/lc.h src/pt/lc-switch.h

ifndef V
    QUIET_CC = @echo '   ' CC $@;
    QUIET_LINK = @echo '   ' LINK $@;
    QUIET_OBJCOPY = @echo '   ' OBJCOPY $@;
endif

default: $(OBJDIR)/hauptuhr.bin

all: default

clean:
	$(RM) -r $(OBJDIR) VERSION-FILE

install: $(OBJDIR)/hauptuhr.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U f:w:$<

size: $(OBJDIR)/hauptuhr.bin
	@stat -c "%n: %s bytes" $<

.PHONY: default all clean install size test

$(OBJDIR):
	$(MKDIR) $@

$(OBJDIR)/hauptuhr: $(addprefix $(OBJDIR)/,$(OBJECTS))
	$(QUIET_LINK)$(AVR_CC) $(CFLAGS) $(AVR_CFLAGS) -o $@ $^

$(addprefix $(OBJDIR)/,$(OBJECTS)): | $(OBJDIR)

$(OBJDIR)/%.o: src/%.c $(EXTRA_DEPENDENCIES)
	$(QUIET_CC)$(AVR_CC) $(CFLAGS) $(AVR_CFLAGS) -c -o $@ $<

$(OBJDIR)/%.bin: $(OBJDIR)/%
	$(QUIET_OBJCOPY)$(AVR_OBJCOPY) -O binary -R .eeprom $< $@

$(OBJDIR)/%.hex: $(OBJDIR)/%
	$(QUIET_OBJCOPY)$(AVR_OBJCOPY) -O ihex -R .eeprom $< $@
