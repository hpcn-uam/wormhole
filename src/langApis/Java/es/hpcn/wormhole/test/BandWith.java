package es.hpcn.wormhole.test;

import es.hpcn.wormhole.Worm;

import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;

public class BandWith
{
	private static final int numMsg = 50000;
	private static final int msgSize = 1048576;

	private static final int PEAK_FRACTION_DOWN = numMsg / 3;
	private static final int PEAK_FRACTION_UP = PEAK_FRACTION_DOWN * 2;

	private static String randomString(long len)
	{
		final String AB = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		SecureRandom rnd = new SecureRandom();

		StringBuilder sb = new StringBuilder((int)len);

		for (long i = 0; i < len; i++) {
			sb.append(AB.charAt(rnd.nextInt(AB.length())));
		}

		return sb.toString();
	}

	public static void main(String[] args) throws Exception
	{
		Worm worm = new Worm();

		long startTime;
		long endTime;

		long peakStartTime = 0;
		long peakEndTime = 0;

		if (worm.getId() == 1) {
			//sender
			String tmp = randomString(msgSize);
			byte[] data = tmp.getBytes(StandardCharsets.US_ASCII);

			System.out.println("Sending msg of " + data.length + " Bytes");

			startTime = System.currentTimeMillis() * 1000000;

			for (int i = 0; i < numMsg; i++) {
				//System.out.println("Sent: '"+tmp+"'");
				if (worm.send(data) != 0) {
					System.out.println("Send Error (msgnum=" + i + ")");
				}

				if (i == PEAK_FRACTION_DOWN) {
					peakStartTime = System.currentTimeMillis() * 1000000;
				}

				if (i == PEAK_FRACTION_UP) {
					peakEndTime = System.currentTimeMillis() * 1000000;
				}
			}

			worm.flushIO();

			endTime = System.currentTimeMillis() * 1000000;
			System.out.println("Tasa SEND = " + ((double)(numMsg) * msgSize * 8.) / (endTime - startTime) + " gbps");

		} else {
			//recv
			byte[] data = new byte[msgSize];

			startTime = System.currentTimeMillis() * 1000000;

			for (int i = 0; i < numMsg; i++) {
				if (worm.recv(data) != msgSize) {
					System.out.println("Recv Error (msgnum=" + i + ")");
				}

				if (i == PEAK_FRACTION_DOWN) {
					peakStartTime = System.currentTimeMillis() * 1000000;
				}

				if (i == PEAK_FRACTION_UP) {
					peakEndTime = System.currentTimeMillis() * 1000000;
				}

				//System.out.println("Recv: '"+new String(data)+"'");
			}

			endTime = System.currentTimeMillis() * 1000000;
			System.out.println("Tasa RECV = " + ((double)(numMsg) * msgSize * 8.) / (endTime - startTime) + " gbps");

		}

		System.out.println("Tasa Pico = " + ((double)(PEAK_FRACTION_UP - PEAK_FRACTION_DOWN) * msgSize * 8.) / (peakEndTime - peakStartTime) + " gbps");


		worm.halt();
	}
}