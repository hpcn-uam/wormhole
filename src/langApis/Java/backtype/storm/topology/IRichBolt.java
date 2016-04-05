package backtype.storm.topology;

import backtype.storm.task.TopologyContext;
import backtype.storm.task.OutputCollector;
import backtype.storm.tuple.Tuple;
import backtype.storm.topology.OutputFieldsDeclarer;

import java.util.Map;

public interface IRichBolt
{
	public void prepare(Map conf, TopologyContext context, OutputCollector collector);
	public void execute(Tuple tuple);
	public void declareOutputFields(OutputFieldsDeclarer declarer);
}