/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.flink.streaming.examples.socket;

import org.apache.flink.api.common.functions.FlatMapFunction;
import org.apache.flink.api.common.functions.ReduceFunction;
import org.apache.flink.api.java.utils.ParameterTool;
import org.apache.flink.streaming.api.datastream.DataStream;
import org.apache.flink.streaming.api.environment.StreamExecutionEnvironment;
import org.apache.flink.streaming.api.windowing.time.Time;
import org.apache.flink.util.Collector;

/**
 * Implements a streaming windowed version of the "WordCount" program.
 *
 * This program connects to a server socket and reads strings from the socket.
 * The easiest way to try this out is to open a text sever (at port 12345)
 * using the <i>netcat</i> tool via
 * <pre>
 * nc -l 12345
 * </pre>
 * and run this example with the port as an argument.
 */
@SuppressWarnings("serial")
public class SocketWindowWordCount
{

	public static void main(String[] args) throws Exception
	{

		// the port to connect to
		final int port;

		try {
			final ParameterTool params = ParameterTool.fromArgs(args);
			port = params.getInt("port");

		} catch (Exception e) {
			System.err.println("No port specified. Please run 'BandwithMeter --port <port>', " +
							   "where port is the address of the text server");
			System.err.println("To start a simple text server, run 'netcat -l <port>' and type the input text " +
							   "into the command line");
			return;
		}

		// get the execution environment
		final StreamExecutionEnvironment env = StreamExecutionEnvironment.getExecutionEnvironment();

		// get input data by connecting to the socket
		DataStream<String> text = env.socketTextStream("nrg", port, '\n');


		long tmp2 = System.currentTimeMillis() * 1000000;

		// parse the data, group it, window it, and aggregate the counts
		DataStream<String> windowCounts = text

		.flatMap(new FlatMapFunction<String, String>() {
			private static final long numMsg = 100000;

			private long num = 0;
			private long startTime = tmp2;

			@Override
			public void flatMap(String value, Collector<String> out) {
				/*for (String word : value.split("\\s")) {
					out.collect(new WordWithCount(word, 1L));
				}*/
				num++;

				if (num >= numMsg) {
					long tmp = System.currentTimeMillis() * 1000000;
					out.collect("Tasa = " + ((double)(num) * value.length() * 8.) / (tmp - startTime) + " gbps");
					startTime = tmp;
					num = 0;
				}
			}
		})/*

				.keyBy("word")
				.timeWindow(Time.seconds(5), Time.seconds(1))

				.reduce(new ReduceFunction<WordWithCount>() {
					@Override
					public WordWithCount reduce(WordWithCount a, WordWithCount b) {
						return new WordWithCount(a.word, a.count + b.count);
					}
				})*/;

		// print the results with a single thread, rather than in parallel
		windowCounts.print().setParallelism(1);

		env.execute("Socket Window WordCount");
	}

}
