from time import perf_counter
import asyncio

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
bus_clock = 426/4
adc_clock = bus_clock/4
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
	band = np.round(band, 1)
	band = np.unique(band)[::2]

	#lo = np.exp(1j * .6 * 1000 * 1000 * 2*np.pi*np.arange(4096)/(samplerate*1000*1000))
	#filt = [1.0]*250 + [0.0]*(2048-250)
	#filt = filt + list(reversed(filt))
	#filt = np.array(filt) > 0

	filt = (band > .01) + 0.0

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

def stft_debug(temporal):
	plt.imshow(np.log10(np.abs(np.fft.fftshift(np.fft.fft(np.split(temporal[:-(len(temporal)%8192)], 8192), 8192)))))

async def stream_audio():
	target = round(samplerate * 1000 * 1000)
	lo = np.exp(1j * .6 * 1000 * 1000 * 2*np.pi*np.arange(target)/(samplerate*1000*1000))

	ffplay = await asyncio.create_subprocess_exec("ffplay", "-f", "f32le", "-ar", f"{round(samplerate*1000*1000)}", "-", stdin=asyncio.subprocess.PIPE)

	def pull_data():
		byts = b""

		while len(byts) < target:
			byts += handle.bulkRead(EP, 32768, timeout=1000)

		return byts

	i = 0

	while True:
		if i < 3: # Build up a buffer in blocking mode, then coast in nonblocking
			byts = pull_data()
		else:
			byts = await asyncio.to_thread(pull_data)

		i += 1
		byts = byts[:target]

		ints = np.frombuffer(byts, dtype=np.uint8)

		temporal = ints * lo

		#plt.subplot(2, 1, 1)
		#stft_debug(temporal)

		# Handcrafted filter
		spectral = np.array([1] * 8 + [0j] * 112 + [1] * 8) * np.exp(1j * 2 * np.pi * np.arange(128)/2)
		temporal = np.convolve(temporal, np.fft.ifft(spectral))
		frequenc = temporal * np.roll(temporal, 1).conj()
		amplitud = np.angle(frequenc) / np.pi

		#plt.subplot(2, 1, 2)
		#stft_debug(temporal)
		#plt.show()

		ffplay.stdin.write( bytes(amplitud.astype("float32")) )
		#await ffplay.stdin.drain() # required if not using await to_thread and blocking instead

#wave_display()
#stft_display()

asyncio.run(stream_audio())
