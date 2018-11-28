all: asm-build vm-build
clean: asm-clean vm-clean
asm-build:
	$(.MAKE) $(.MAKEFLAGS) -C asm all
vm-build:
	$(.MAKE) $(.MAKEFLAGS) -C vm all
asm-clean:
	$(.MAKE) $(.MAKEFLAGS) -C asm clean
vm-clean:
	$(.MAKE) $(.MAKEFLAGS) -C vm clean
