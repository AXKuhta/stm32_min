
import matplotlib.pyplot as plt
import numpy as np
import usb1

from time import perf_counter

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
bus_clock = 426/4
adc_clock = bus_clock/2
persample = 4.5 + 64.5 # timings depending on resolution + sampling time, see Table 228. TSAR timings depending on resolution
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

	band = np.arange(2048)/2048 * samplerate/2
	filt = (band > .01) + 0.0
	band = np.round(band, 1)
	band = np.unique(band)[::2]

	while True:
		for i in range(128):
			byts = b""

			while len(byts) < 4096:
				byts += handle.bulkRead(EP, 1024, timeout=1000)

			ints = np.frombuffer(byts, dtype=np.uint8)

			spectral = np.fft.fft(ints)[:2048] * filt
			stft_log.append(np.log10(np.abs(spectral)))

		plt.clf()
		plt.imshow(stft_log[-1000:])
		plt.xticks(band/samplerate*2 * 2048, [f"{bus_clock+x:.1f} MHz" for x in band])
		plt.pause(.1)


def stream_audio():
	stft_log = []
	freq_log = []

	filt = [1.0]*250 + [0.0]*(2048-250)
	filt = filt + list(reversed(filt))
	filt = np.array(filt) > 0

	lo = np.exp(1j * .6 * 1000 * 1000 * 2*np.pi*np.arange(4096)/(samplerate*1000*1000))

	# ffplay -f f32le -ar 85000 decode.f32
	f = open("decode.f32", "wb")

	while True:
		for i in range(128):
			byts = b""

			while len(byts) < 4096:
				byts += handle.bulkRead(EP, 1024, timeout=1000)

			ints = np.frombuffer(byts, dtype=np.uint8) * lo

			spectral = np.fft.fftshift(np.fft.fft(ints)[filt])
			temporal = np.fft.ifft(np.fft.fftshift(spectral))
			frequenc = temporal * np.roll(temporal, 1).conj()
			#f.write(bytes(temporal))
			stft_log.append(np.log10(np.abs(spectral)))
			freq_log.append(np.angle(frequenc) / np.pi)
			f.write(bytes(freq_log[-1].astype("float32")))

		#plt.clf()
		#plt.plot(np.hstack(freq_log)[-8000:])
		#plt.imshow(stft_log[-1000:])
		#plt.pause(.1)

#wave_display()
#stft_display()
stream_audio()
