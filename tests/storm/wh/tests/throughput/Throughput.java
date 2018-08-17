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
package wh.tests.throughput;

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

/**
 * This is a basic example of a Storm topology.
 */
public class Throughput {
	private static final long numMsg = 1000000;
	private static final long msgSize = 60;

	public static class RecvBolt extends BaseRichBolt {
		private static final Logger LOG = LoggerFactory.getLogger(RecvBolt.class);

		OutputCollector _collector;

		private long num = 0;
		private PrintWriter writer;
		private long startTime;

		@Override
		public void prepare(Map conf, TopologyContext context, OutputCollector collector) {
			_collector = collector;

			try {
				startTime = System.nanoTime();

			} catch (Exception e) {
			}
		}

		@Override
		public void execute(Tuple tuple) {
			_collector.ack(tuple);
			num++;

			if (num >= numMsg) {
				synchronized (this) {
					if (num >= numMsg) {
						long tmp = System.nanoTime();
						LOG.info("Tasa = {} GCharsPS && {} gbps", ((double) (num) * msgSize * 8.) / (tmp - startTime),
								((double) (num) * msgSize * 16.) / (tmp - startTime));
						startTime = tmp;
						num = 0;
					}
				}
			}
		}

		@Override
		public void declareOutputFields(OutputFieldsDeclarer declarer) {
			declarer.declare(new Fields("data"));
		}
	}

	public static class SendSpout extends BaseRichSpout {
		private static final Logger LOG = LoggerFactory.getLogger(SendSpout.class);
		boolean _isDistributed;
		SpoutOutputCollector _collector;

		String msg;
		Values v;

		String randomString(long len) {
			final String AB = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
			SecureRandom rnd = new SecureRandom();

			StringBuilder sb = new StringBuilder((int) len);

			for (long i = 0; i < len; i++) {
				sb.append(AB.charAt(rnd.nextInt(AB.length())));
			}

			return sb.toString();
		}

		public SendSpout() {
			this(true);
		}

		public SendSpout(boolean isDistributed) {
			_isDistributed = isDistributed;
			msg = randomString(msgSize);
			v = new Values(msg);
		}

		public void open(Map conf, TopologyContext context, SpoutOutputCollector collector) {
			_collector = collector;
		}

		public void close() {
		}

		public void nextTuple() {
			_collector.emit(v);
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
		builder.setBolt("recv", new RecvBolt(), 1).shuffleGrouping("send");

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
