
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

# Bus clock = 204 MHz
# ADC clock = 51 MHz
#
# 32 clocks per sample
#
# Samplerate = 1.59375 MHz
def wave_display():
	x = 627.4509 * np.arange(4096) / 1000

	while True:
		byts = b""

		while len(byts) < 8192:
			byts += handle.bulkRead(EP, 1024, timeout=1000)

		ints = np.frombuffer(byts, dtype=np.uint16)

		plt.clf()
		plt.plot(x, ints)
		plt.ylim(0, 65535)
		plt.xlim(0, 128)
		plt.xlabel("us")
		plt.pause(.1)

def stft_display():
	stft_log = []

	spcy = np.linspace(0, 0.8, 9)

	while True:
		for i in range(512):
			byts = b""

			while len(byts) < 8192:
				byts += handle.bulkRead(EP, 1024, timeout=1000)

			ints = np.frombuffer(byts, dtype=np.uint16)

			# Bus clock = 204 MHz
			# ADC clock = 51 MHz
			#
			# 32 clocks per sample
			#
			# Samplerate = 1.59375 MHz
			#
			# Usable band = ~0.8 MHz
			#
			spectral = np.fft.fft(ints - np.mean(ints))[:2048]
			stft_log.append(np.log10(np.abs(spectral)))

		plt.clf()
		plt.imshow(stft_log)
		plt.xticks(spcy/0.8 * 2048, [f"{x:.1f} MHz" for x in spcy])
		plt.pause(.1)

wave_display()
