package wh.tests.rtt;

import scala.Tuple2;

import org.apache.spark.SparkConf;
import org.apache.spark.api.java.StorageLevels;
import org.apache.spark.streaming.Durations;
import org.apache.spark.streaming.api.java.JavaDStream;
import org.apache.spark.streaming.api.java.JavaPairDStream;
import org.apache.spark.streaming.api.java.JavaReceiverInputDStream;
import org.apache.spark.streaming.api.java.JavaStreamingContext;
import org.apache.spark.api.java.function.*;

import java.io.IOException;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Properties;
import java.util.regex.Pattern;
import java.io.InputStream;
import java.util.Iterator;
import java.io.OutputStream;

import java.net.ServerSocket;
import java.net.Socket;
import java.io.BufferedInputStream;
import java.io.PrintStream;

import java.util.Random;
import java.security.SecureRandom;
import java.lang.Iterable;
import java.util.Iterator;
import java.util.NoSuchElementException;

public class RttClient
{
	static OutputStream pw;

	static int batch_size;
	static int batches;
	static Properties config;
	static long measurements[];
	static int thr_i;

	public static byte[] longToBytes(long l)
	{
		byte[] result = new byte[8];
		for (int i = 7; i >= 0; i--) {
			result[i] = (byte) (l & 0xFF);
			l >>= 8;
		}
		return result;
	}

	public static long bytesToLong(byte[] b)
	{
		long result = 0;
		for (int i = 0; i < 8; i++) {
			result <<= 8;
			result |= (b[i] & 0xFF);
		}
		return result;
	}

	public static void main(final String[] args) throws Exception
	{
		if (args.length != 7) {
			System.out.println("Params: <hostname> <port:c> <port:l> <message-size> <batch size> <batches> <sleep>");
		}
		final String hostname = args[0];
		final int portc       = Integer.parseInt(args[1]);
		final int portl       = Integer.parseInt(args[2]);

		final int message_size = Integer.parseInt(args[3]);
		batch_size             = Integer.parseInt(args[4]);
		batches                = Integer.parseInt(args[5]);
		final int sleep        = Integer.parseInt(args[6]);

		measurements = new long[batches];

		SparkConf sparkConf      = new SparkConf().setAppName("WhRttTestClient");
		JavaStreamingContext ssc = new JavaStreamingContext(sparkConf, Durations.milliseconds(100));

		Function byte2byte = new Function<InputStream, Iterable<byte[]>>() {
			class StreamIterable implements Iterable<byte[]>
			{
				InputStream is;

				// Constructor that takes a "raw" array and stores it
				public StreamIterable(InputStream is)
				{
					this.is = is;
				}

				// This is a private class that implements iteration over the elements
				// of the list. It is not accessed directly by the user, but is used in
				// the iterator() method of the Array class. It implements the hasNext()
				// and next() methods.
				class StreamIterator implements Iterator<byte[]>
				{
					// return whether or not there are more elements in the array that
					// have not been iterated over.
					public boolean hasNext()
					{
						return true;
					}

					// return the next element of the iteration and move the current
					// index to the element after that.
					public byte[] next()
					{
						if (!hasNext()) {
							throw new NoSuchElementException();
						}

						byte[] ret = new byte[message_size];
						int offset = 0;
						do {
							try {
								offset += is.read(ret, offset, message_size - offset);
							} catch (IOException e) {
							}
						} while (offset != message_size);
						return ret;
					}
				}

				// Return an iterator over the elements in the array. This is generally not
				// called directly, but is called by Java when used in a "simple" for loop.
				public Iterator<byte[]> iterator()
				{
					return new StreamIterator();
				}
			}

			public Iterable<byte[]> call(InputStream bif)
			{
				return new StreamIterable(bif);
			}
		};

		JavaReceiverInputDStream<byte[]> messages = ssc.socketStream(hostname, portc, byte2byte, StorageLevels.MEMORY_ONLY);

		thr_i = 0;
		messages.foreachRDD(rdd -> {
			rdd.foreach(msg -> {
				thr_i++;
				if (thr_i % batch_size == 0) {
					measurements[thr_i] = System.nanoTime() - bytesToLong(msg);
				}
			});
		});
		ssc.start();

		ServerSocket ss = new ServerSocket(portl);
		Socket s        = ss.accept();
		OutputStream pw = s.getOutputStream();

		byte[] msg = new byte[message_size];

		for (int i = 0; i != batches; i++) {
			for (int j = 0; j != batch_size; j++) {
				if (j == 0) {
					long ctime    = System.nanoTime();
					byte[] ctimeb = longToBytes(ctime);
					for (int k = 0; k < 8; ++k) {
						msg[k] = ctimeb[k];
					}
				}
				try {
					pw.write(msg);
				} catch (IOException e) {
				}
			}
			pw.flush();
			if (sleep > 0) {
				Thread.sleep(sleep);
			}
		}
		pw.flush();

		while (thr_i < batches * batch_size) {
			Thread.sleep(100);
		}

		if (thr_i >= batches * batch_size) {
			for (int i = 0; i < batches; i++) {
				System.out.println((measurements[i]) + " ns");
			}

			ssc.stop();
			System.exit(0);
		}
	}
}