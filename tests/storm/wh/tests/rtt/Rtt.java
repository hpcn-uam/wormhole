/**
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
package wh.tests.rtt;

import org.apache.storm.Config;
import org.apache.storm.StormSubmitter;
import org.apache.storm.task.OutputCollector;
import org.apache.storm.task.TopologyContext;
import org.apache.storm.topology.OutputFieldsDeclarer;
import org.apache.storm.topology.TopologyBuilder;
import org.apache.storm.topology.base.BaseRichBolt;
import org.apache.storm.tuple.Fields;
import org.apache.storm.tuple.Tuple;
import org.apache.storm.tuple.Values;

import org.apache.storm.spout.SpoutOutputCollector;
import org.apache.storm.topology.base.BaseRichSpout;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.HashMap;
import java.util.Random;

import java.util.Map;
import java.io.PrintWriter;

import java.security.SecureRandom;

import java.lang.Thread;

/**
 * This is a basic example of a Storm topology.
 */
public class Rtt {
	private static final long numMsg = 1000000;
	private static final int message_size = 60;
	private static final int batch_size = 1;
	private static final int batches = 100000;
	private static final int sleep = 0;

	public static class recvBolt extends BaseRichBolt {
		private static final Logger LOG = LoggerFactory.getLogger(recvBolt.class);

		OutputCollector _collector;

		private long i = 0;
		private long measurements[];

		public static long bytesToLong(byte[] b) {
			long result = 0;
			for (int k = 0; k < 8; k++) {
				result <<= 8;
				result |= (b[k] & 0xFF);
			}
			return result;
		}

		@Override
		public void prepare(Map conf, TopologyContext context, OutputCollector collector) {
			_collector = collector;
			measurements = new long[batches];

		}

		@Override
		public void execute(Tuple tuple) {
			_collector.ack(tuple);

			if (i % batch_size == 0) {
				byte[] msg = tuple.getBinary(0);
				measurements[(int) (i / batch_size)] = System.nanoTime() - bytesToLong(msg);
			}

			i++;

			if (i >= batches * batch_size) {
				for (int k = 0; k < batches; k++) {
					LOG.info("Measured RTT: {} ns", measurements[k]);
				}
				i = -i;
			}
		}

		@Override
		public void declareOutputFields(OutputFieldsDeclarer declarer) {
			declarer.declare(new Fields("data"));
		}
	}

	public static class ProxyBolt extends BaseRichBolt {
		private static final Logger LOG = LoggerFactory.getLogger(ProxyBolt.class);

		OutputCollector _collector;

		@Override
		public void prepare(Map conf, TopologyContext context, OutputCollector collector) {
			_collector = collector;
		}

		@Override
		public void execute(Tuple tuple) {
			_collector.ack(tuple);
			byte[] msg = (byte[]) tuple.getValue(0);
			_collector.emit(new Values(msg));
		}

		@Override
		public void declareOutputFields(OutputFieldsDeclarer declarer) {
			declarer.declare(new Fields("data"));
		}
	}

	public static class SendSpout extends BaseRichSpout {
		private static final Logger LOG = LoggerFactory.getLogger(SendSpout.class);
		private SpoutOutputCollector _collector;

		private byte[] msg;
		private long i = 0;

		public static byte[] longToBytes(long l) {
			byte[] result = new byte[8];
			for (int k = 7; k >= 0; k--) {
				result[k] = (byte) (l & 0xFF);
				l >>= 8;
			}
			return result;
		}

		public SendSpout() {
			this(true);
		}

		public SendSpout(boolean isDistributed) {
			msg = new byte[message_size];
		}

		public void open(Map conf, TopologyContext context, SpoutOutputCollector collector) {
			_collector = collector;
		}

		public void close() {
		}

		public void nextTuple() {
			if (i % batch_size == 0) {
				if (sleep > 0) {
					try {
						Thread.sleep(sleep);
					} catch (Exception e) {
					}
				}
				while (i >= batches * batch_size) {
				}

				long ctime = System.nanoTime();
				byte[] ctimeb = longToBytes(ctime);
				for (int k = 0; k < 8; ++k) {
					msg[k] = ctimeb[k];
				}
			}
			i++;
			_collector.emit(new Values(msg));
		}

		public void ack(Object msgId) {
		}

		public void fail(Object msgId) {
		}

		public void declareOutputFields(OutputFieldsDeclarer declarer) {
			declarer.declare(new Fields("data"));
		}

		@Override
		public Map<String, Object> getComponentConfiguration() {
			return null;
		}
	}

	public static void main(String[] args) throws Exception {
		TopologyBuilder builder = new TopologyBuilder();

		builder.setSpout("send", new SendSpout(), 1);
		builder.setBolt("proxy", new ProxyBolt(), 1).shuffleGrouping("send");
		builder.setBolt("recv", new recvBolt(), 1).shuffleGrouping("proxy");

		Config conf = new Config();
		conf.setNumWorkers(3);
		conf.setNumAckers(0);
		// conf.setDebug(true);

		if (args != null && args.length > 0) {
			StormSubmitter.submitTopologyWithProgressBar(args[0], conf, builder.createTopology());

		} else {
			System.out.println("arguments necesary");
		}
	}
}
