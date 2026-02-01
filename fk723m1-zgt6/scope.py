
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

# Rates in MHz
bus_clock = 137.5
adc_clock = bus_clock/1
persample = 8.5 + 16.5 # timings depending on resolution + sampling time
samplerate = adc_clock/persample

def wave_display():
	x = 1/samplerate * np.arange(4096) # us per sample

	while True:
		byts = b""

		while len(byts) < 8192:
			byts += handle.bulkRead(EP, 1024, timeout=1000)

		ints = np.frombuffer(byts, dtype=np.uint16)

		plt.clf()
		plt.plot(x, ints)
		plt.ylim(0, 65535)
		plt.xlim(0, 16)
		plt.xlabel("us")
		plt.pause(.1)

def stft_display():
	stft_log = []

	spcy = np.linspace(0, samplerate/2, 9)

	while True:
		for i in range(128):
			byts = b""

			while len(byts) < 8192:
				byts += handle.bulkRead(EP, 1024, timeout=1000)

			ints = np.frombuffer(byts, dtype=np.uint16)

			spectral = np.fft.fft(ints - np.mean(ints))[:2048]
			stft_log.append(np.log10(np.abs(spectral)))

		plt.clf()
		plt.imshow(stft_log)
		plt.xticks(spcy/(samplerate/2) * 2048, [f"{x:.1f} MHz" for x in spcy])
		plt.pause(.1)

#wave_display()
stft_display()
