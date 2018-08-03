package backtype.storm.topology;

import backtype.storm.task.TopologyContext;
import backtype.storm.spout.SpoutOutputCollector;

import java.util.Map;

public interface IRichSpout
{

	public void open(Map conf, TopologyContext context, SpoutOutputCollector collector);
	public void close();
	public void nextTuple();
	public void ack(Object msgId);
	public void fail(Object msgId);
	public Map<String, Object> getComponentConfiguration();

}