include ../common.mak

# Restrict the source replacement to the experimental GT-BE98_PRO profile.
ifeq ($(ASUS_OPEN_COMPONENTS):$(BUILD_NAME),y:GT-BE98_PRO)
OPEN_ASUSLOG := y
endif

OBJS = libasuslog.o
PROGS = libasuslog.so

CFLAGS += $(if $(RTCONFIG_DEBUG),-g) -fPIC $(EXTRACFLAGS)
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(KERNEL_HEADER_DIR) -Wall
#CFLAGS += $(PRIVATE_EXTRACFLAGS)
LDFLAGS_SO = -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS_SO += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif
ifeq ($(HND_ROUTER),y)
LDFLAGS_SO += -lpthread -lgcc_s
endif
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
CFLAGS += -I$(SRCBASE)/common/include
LDFLAGS += -lpthread -lgcc_s
endif

ifeq ($(RTCONFIG_QCA),y)
LDFLAGS += -ldl
endif


#SHARE_STATIC=y


ifeq ($(OPEN_ASUSLOG),y)
# libnvram imports ASUS helpers from libshared without declaring DT_NEEDED.
LDFLAGS_SO = -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared -lpthread
all: $(PROGS)

# Rebuild when returning from a profile that copied a prebuilt .so over ours.
.PHONY: open-asuslog-force
libasuslog.o: source/libasuslog.c open-asuslog-force
	$(CC) $(CFLAGS) -c $< -o $@
else ifeq ($(wildcard $(TOP)/libasuslog/*.c),)
all:
	-cp -f $(TOP)/libasuslog/prebuild/$(BUILD_NAME)/libasuslog.so libasuslog.so
else
all: $(PROGS)
endif




libasuslog.so: $(OBJS)
ifeq ($(HND_ROUTER),y)
	$(CC) $(LDFLAGS_SO) -shared -o $@ $^
else
	$(LD) $(LDFLAGS_SO) -shared -o $@ $^
endif


install: all
	$(if $(RTCONFIG_DEBUG),,$(STRIP) $(PROGS))

	install -D libasuslog.so $(INSTALLDIR)/lib/libasuslog.so

clean:
	rm -f *.o $(PROGS)
