
import matplotlib.pyplot as plt
import numpy as np
import usb1

ctx = usb1.USBContext()
handle = None

print ("VID  PID")

for dev in ctx.getDeviceList():
	vid = dev.getVendorID()
	pid = dev.getProductID()
	print(f"{vid:04x} {pid:04x}")
	if vid == 0xCAFE and pid == 0x4011:
		print("Device found")
		handle = dev.open()

handle.claimInterface(2)

EP = 0x03

while True:
	byts = b""

	while len(byts) < 8192:
		byts += handle.bulkRead(EP, 1024, timeout=1000)

	ints = np.frombuffer(byts, dtype=np.uint16)
	plt.clf()
	plt.plot(ints)
	plt.ylim(0, 65535)
	plt.pause(.1)
