include $(TOPDIR)/rules.mk

PKG_NAME:=signal_extract
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/signal_extract
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=this is signal_test
	DEPENDS:=+libpthread
	DEPENDS:=+libc
	DEPENDS:=+libpcap 
endef

define Package/signal_extractg/description
	this is a test signal_extract!
endef


define Build/Prepare
	touch $(PKG_BUILD_DIR)/signal_extract
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/signal_extract/install
	$(INSTALL_DIR) $(1)/bin
	$(CP) $(TOPDIR)/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/lib/libpthread.so.0 $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/signal_extract $(1)/bin
endef

$(eval $(call BuildPackage,signal_extract))
