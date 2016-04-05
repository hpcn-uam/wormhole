package backtype.storm.topology.base;

import backtype.storm.topology.IRichSpout;
import backtype.storm.task.TopologyContext;
import backtype.storm.spout.SpoutOutputCollector;

import java.util.Map;

public class BaseRichSpout implements IRichSpout
{
	public BaseRichSpout()
	{

	}

	public void open(Map conf, TopologyContext context, SpoutOutputCollector collector)
	{

	}

	public void close()
	{

	}

	public void nextTuple()
	{

	}

	public void ack(Object msgId)
	{

	}

	public void fail(Object msgId)
	{

	}

	public Map<String, Object> getComponentConfiguration()
	{
		return null;
	}
}